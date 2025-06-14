#include "main.hpp"

#include <algorithm>
#include <emscripten.h>
#include <iostream>
#include <emscripten/html5.h>
#include <thread>
#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

#include "Event/EventBus.hpp"
#include "Event/Events/TickEvent.hpp"
#include "Objects/MeshObject.hpp"
#include "Objects/Object.hpp"
#include "Protocol/PacketHelper.hpp"
#include "Protocol/Socket.hpp"
#include "Protocol/Packets/AddMapObject.hpp"
#include "Protocol/Packets/EntityAction.hpp"
#include "Protocol/Packets/HandShakePacket.hpp"

int windowWidth = 800;
int windowHeight = 600;

Camera3D camera = { 0 };
Model cubeModel;

EM_BOOL onResize(int, const EmscriptenUiEvent* e, void*) {
    windowWidth = e->windowInnerWidth;
    windowHeight = e->windowInnerHeight;
    SetWindowSize(windowWidth, windowHeight);
    return EM_TRUE;
}

void setupAndRender() {
    Main::objects.insert(Main::objects.end(), Main::map.begin(), Main::map.end());
    auto p = Main::localTank->position;
    Main::localTankObject = std::make_shared<MeshObject>(p->rotation, p->position, std::make_shared<Model>(cubeModel));
    Main::objects.push_back(Main::localTankObject);

    for (auto& e : Main::entities) {
        auto pos = e->position;
        Main::objects.push_back(std::make_shared<MeshObject>(pos->rotation, pos->position, std::make_shared<Model>(cubeModel)));
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
    UpdateCamera(&camera, CAMERA_THIRD_PERSON);
    BeginDrawing();
    ClearBackground(RAYWHITE);

    auto cv = Main::localTank->velocity;
    auto cp = Main::localTank->position;
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

    BeginMode3D(camera);

    auto poscomp = Main::localTank->position;
    camera.target = poscomp->position;

    setupAndRender();

    EndMode3D();
    EndDrawing();
}

int main() {
    bool ticklooprunning = true;

    emscripten_get_canvas_element_size("#canvas", &windowWidth, &windowHeight);
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, onResize);
    emscripten_request_pointerlock("canvas", EM_TRUE);

    PacketHelper::registerPacket(0, []() { return new HandShakePacket(); });
    PacketHelper::registerPacket(1, []() { return new AddMapObject(); });
    PacketHelper::registerPacket(2, []() { return new EntityAction(); });
    SocketClient::connect();

    InitWindow(windowWidth, windowHeight, "TankeryOnlineClient");

    camera.position = (Vector3){ -1.0f, 1.0f, 0.0f };
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

    emscripten_set_main_loop(mainLoop, 0, 1);
    ticklooprunning = false;
    tick.join();
    CloseWindow();

    return 0;
}