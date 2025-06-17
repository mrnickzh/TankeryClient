#include "main.hpp"

#include <algorithm>
#include <emscripten.h>
#include <iostream>
#include <emscripten/html5.h>
#include <thread>
#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

#include "Utils/tppcamera.h"
#include "Event/EventBus.hpp"
#include "Event/Events/TickEvent.hpp"
#include "Objects/MeshObject.hpp"
#include "Objects/Object.hpp"
#include "Protocol/PacketHelper.hpp"
#include "Protocol/Socket.hpp"
#include "Protocol/Packets/AddMapObject.hpp"
#include "Protocol/Packets/EntityAction.hpp"
#include "Protocol/Packets/HandShakePacket.hpp"
#include "Protocol/Packets/PlayerAuthInput.hpp"

int windowWidth = 800;
int windowHeight = 600;

Camera3D camera = { 0 };
Model cubeModel;

void calculatevelocity() {
    for (auto e : Main::entities) {
        auto cv = e->velocity;
        auto cp = e->position;
        cv->prev_velocity = cv->velocity;

        float yawRad = cp->rotation.y * DEG2RAD;
        float dx = std::sin(yawRad);
        float dz = std::cos(yawRad);

        cp->prevPosition = cp->position;

        float tmpx = cp->position.x + cv->velocity.x * dz;
        float tmpz = cp->position.z + cv->velocity.z * -dx;
        float tmpy = cp->position.y + cv->velocity.y;

        cp->position.x = tmpx;
        cp->position.z = tmpz;
        cp->position.y = tmpy;

        if (cv->velocity.x < 0) { cv->velocity.x += 0.001; }
        if (cv->velocity.x > 0) { cv->velocity.x -= 0.001; }
        if (cv->velocity.x < 0.001 && cv->velocity.x > -0.001) { cv->velocity.x = 0; }

        if (cv->velocity.z < 0) { cv->velocity.z += 0.001; }
        if (cv->velocity.z > 0) { cv->velocity.z -= 0.001; }
        if (cv->velocity.z < 0.001 && cv->velocity.z > -0.001) { cv->velocity.z = 0; }

        if (cv->velocity.y < 0) { cv->velocity.y += 0.001; }
        if (cv->velocity.y > 0) { cv->velocity.y -= 0.001; }
        if (cv->velocity.y < 0.001 && cv->velocity.y > -0.001) { cv->velocity.y = 0; }
    }
}

EM_BOOL onResize(int, const EmscriptenUiEvent* e, void*) {
    windowWidth = e->windowInnerWidth;
    windowHeight = e->windowInnerHeight;
    SetWindowSize(windowWidth, windowHeight);
    return EM_TRUE;
}

void setupAndRender() {
    Main::objects.insert(Main::objects.end(), Main::map.begin(), Main::map.end());
    auto p = Main::localTank->position;
    Main::localTankObject = std::make_shared<MeshObject>(p->rotation, p->position, std::make_shared<Model>(cubeModel), BLUE);
    Main::objects.push_back(Main::localTankObject);

    for (auto& e : Main::entities) {
        auto pos = e->position;
        Main::objects.push_back(std::make_shared<MeshObject>(pos->rotation, pos->position, std::make_shared<Model>(cubeModel), BLUE));
    }

    float camX = camera.position.x, camY = camera.position.y, camZ = camera.position.z;
    std::vector<std::shared_ptr<Object>> sortedObjects = Main::objects;
    std::sort(sortedObjects.begin(), sortedObjects.end(),
              [camX, camY, camZ](const std::shared_ptr<Object>& a, const std::shared_ptr<Object>& b) {
                  float distA = (a->position.x - camX)*(a->position.x - camX) +
                                (a->position.y - camY)*(a->position.y - camY) +
                                (a->position.z - camZ)*(a->position.z - camZ);
                  float distB = (b->position.x - camX)*(b->position.x - camX) +
                                (b->position.y - camY)*(b->position.y - camY) +
                                (b->position.z - camZ)*(b->position.z - camZ);
                  return distA > distB;
              });
    for (const auto& object : sortedObjects) {
        object->render();
    }
    Main::objects.clear();

    DrawGrid(10, 1.0f);
}

void mainLoop() {
    UpdateTPPCamera(&camera, CAMERA_THIRD_PERSON);

    auto cv = Main::localTank->velocity;
    auto cp = Main::localTank->position;
    bool moved = false;
    if (IsKeyDown(KEY_W)) {
        if (cv->velocity.z > -0.1 && cv->velocity.x > -0.1) { cv->velocity.z += 0.002; cv->velocity.x += 0.002; moved = true; }
    }
    if (IsKeyDown(KEY_S)) {
        if (cv->velocity.z < 0.1 && cv->velocity.x < 0.01) { cv->velocity.z -= 0.002; cv->velocity.x -= 0.002; moved = true; }
    }
    if (IsKeyDown(KEY_A)) { cp->rotation.y += 0.05; if (cp->rotation.y > 360.0f) { cp->rotation.y -= 360.0f; } moved = true; }
    if (IsKeyDown(KEY_D)) { cp->rotation.y -= 0.05; if (cp->rotation.y < 0.0f) { cp->rotation.y += 360.0f; } moved = true; }
    if (IsKeyDown(KEY_SPACE)) { cv->velocity.y += 0.002; moved = true; }
    if (IsKeyDown(KEY_LEFT_SHIFT)) { cv->velocity.y -= 0.002; moved = true; }
    if (moved) { PlayerAuthInput pkt; SocketClient::sendPacket(&pkt); std::cout << "PAI sent" << std::endl; }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    std::string velocityText = "Velocity x: " + std::to_string(cv->velocity.x) +
                           " y: " + std::to_string(cv->velocity.y) +
                           " z: " + std::to_string(cv->velocity.z);

    std::string positionText = "Position x: " + std::to_string(cp->position.x) +
                       " y: " + std::to_string(cp->position.y) +
                       " z: " + std::to_string(cp->position.z);
    std::string rotationText = "Rotation x: " + std::to_string(cp->rotation.x) +
                       " y: " + std::to_string(cp->rotation.y) +
                       " z: " + std::to_string(cp->rotation.z);
    DrawText(velocityText.c_str(), 10, 10, 10, BLACK);
    DrawText(positionText.c_str(), 10, 20, 10, BLACK);
    DrawText(rotationText.c_str(), 10, 30, 10, BLACK);

    camera.target = Main::localTank->position->position;

    BeginMode3D(camera);

    setupAndRender();

    EndMode3D();
    EndDrawing();
}

int main() {
    bool ticklooprunning = true;

    emscripten_get_canvas_element_size("#canvas", &windowWidth, &windowHeight);
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, onResize);
    emscripten_request_pointerlock("canvas", EM_TRUE);

    L_SUBSCRIBE(TickEvent, [](TickEvent* event) {
        bool collided = false;

        auto cv = Main::localTank->velocity;
        auto cp = Main::localTank->position;
        cv->prev_velocity = cv->velocity;

        float yawRad = cp->rotation.y * DEG2RAD;
        float dx = std::sin(yawRad);
        float dz = std::cos(yawRad);

        cp->prevPosition = cp->position;

        float tmpx = cp->position.x + cv->velocity.x * dz;
        float tmpz = cp->position.z + cv->velocity.z * -dx;
        float tmpy = cp->position.y + cv->velocity.y;

        for (auto& obj : Main::map) {
            if (CheckCollisionBoxes(
            (BoundingBox){(Vector3){ tmpx - 1.0f/2,
                                     tmpy - 1.0f/2,
                                     tmpz - 1.0f/2 },
                          (Vector3){ tmpx + 1.0f/2,
                                     tmpy + 1.0f/2,
                                     tmpz + 1.0f/2 }},
            (BoundingBox){(Vector3){ obj->position.x - 1.0f/2,
                                     obj->position.y - 1.0f/2,
                                     obj->position.z - 1.0f/2 },
                          (Vector3){ obj->position.x + 1.0f/2,
                                     obj->position.y + 1.0f/2,
                                     obj->position.z + 1.0f/2 }})) { collided = true; cv->velocity = Vector3(0, 0, 0); }
        }

        if (!collided) {
            cp->position.x = tmpx;
            cp->position.z = tmpz;
            cp->position.y = tmpy;
            camera.position = Vector3Add(camera.position, (Vector3){ cv->velocity.x * dz, cv->velocity.y,  cv->velocity.z * -dx });
        }

        if (cv->velocity.x < 0) { cv->velocity.x += 0.001; }
        if (cv->velocity.x > 0) { cv->velocity.x -= 0.001; }
        if (cv->velocity.x < 0.001 && cv->velocity.x > -0.001) { cv->velocity.x = 0; }

        if (cv->velocity.z < 0) { cv->velocity.z += 0.001; }
        if (cv->velocity.z > 0) { cv->velocity.z -= 0.001; }
        if (cv->velocity.z < 0.001 && cv->velocity.z > -0.001) { cv->velocity.z = 0; }

        if (cv->velocity.y < 0) { cv->velocity.y += 0.001; }
        if (cv->velocity.y > 0) { cv->velocity.y -= 0.001; }
        if (cv->velocity.y < 0.001 && cv->velocity.y > -0.001) { cv->velocity.y = 0; }

        calculatevelocity();
    });

    PacketHelper::registerPacket(0, []() { return new HandShakePacket(); });
    PacketHelper::registerPacket(1, []() { return new AddMapObject(); });
    PacketHelper::registerPacket(2, []() { return new EntityAction(); });
    PacketHelper::registerPacket(3, []() { return new PlayerAuthInput(); });
    SocketClient::connect();

    InitWindow(windowWidth, windowHeight, "TankeryOnlineClient");

    camera.position = (Vector3){ -10.0f, 5.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Mesh cubeMesh = GenMeshCube(1.0f,1.0f,1.0f);
    cubeModel = LoadModelFromMesh(cubeMesh);

    DisableCursor();

    std::thread tick([&]() {
        int tps = 40;
        int mps = 1000 / tps;
        long long ltt = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        while (ticklooprunning) {
            long long now = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            long long elapsed = now - ltt;

            if (elapsed >= mps) {
                int ticksToRun = (int) (elapsed / mps);
                ltt += ticksToRun * mps;

                for (int i = 0; i < ticksToRun; i++) {
                    TickEvent event;
                    EventBus::getInstance().publish(&event);
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
         }
    });

    Main::localTank = new LocalTank();
    Main::localTank->position->position.y = 1.0f;

    emscripten_set_main_loop(mainLoop, 0, 1);
    ticklooprunning = false;
    tick.join();
    CloseWindow();

    return 0;
}