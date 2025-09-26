#include "main.hpp"

#include <algorithm>
#include <emscripten.h>
#include <iostream>
#include <emscripten/html5.h>
#include <thread>
#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <queue>
#include <string>

#include "Utils/tppcamera.h"
#include "Event/EventBus.hpp"
#include "Event/Events/TickEvent.hpp"
#include "Objects/DynamicObject.hpp"
#include "Objects/StaticObject.hpp"
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

double tickTimer = 0.0f;

EM_BOOL onResize(int, const EmscriptenUiEvent* e, void*) {
    windowWidth = e->windowInnerWidth;
    windowHeight = e->windowInnerHeight;
    SetWindowSize(windowWidth, windowHeight);
    return EM_TRUE;
}

void setupAndRender() {
    Main::objects.insert(Main::objects.end(), Main::map.begin(), Main::map.end());
    Main::objects.push_back(Main::localTank->object);

    // for (auto& e : Main::localTank->armorplates) {
    //     Main::objects.push_back(e);
    // }

    for (auto& e : Main::entities) {
        Main::objects.push_back(e->object);
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
        Main::physicsEngine.update(GetFrameTime(), object->position, object->rotation, object->bodyid);
        object->render();
    }
    Main::objects.clear();

    DrawGrid(10, 1.0f);
}

void mainLoop() {
    UpdateTPPCamera(&camera, CAMERA_THIRD_PERSON);

    auto bodyid = Main::localTank->object->bodyid;
    auto cv = Main::physicsEngine.get_dynamic_linear_velocity(bodyid);
    auto cp = Main::physicsEngine.get_body_position(bodyid);
    auto cr = Main::physicsEngine.get_body_rotation(bodyid);
    auto ca = Main::physicsEngine.get_dynamic_angular_velocity(bodyid);
    bool moved = false;

    if (IsKeyDown(KEY_W)) {
        if (cv.z < 5.0f && cv.x < 5.0f) { Main::physicsEngine.add_dynamic_linear_velocity_with_rotation(bodyid, Vector3(0.1f, 0.1f, 0.1f), cr); moved = true; }
    }
    if (IsKeyDown(KEY_S)) {
        if (cv.z > -5.0f && cv.x > -5.0f) { Main::physicsEngine.add_dynamic_linear_velocity_with_rotation(bodyid, Vector3(-0.1f, -0.1f, -0.1f), cr); moved = true; }
    }
    if (IsKeyDown(KEY_A)) { Main::physicsEngine.set_dynamic_angular_velocity(bodyid, Vector3(0.0f, 20.0f, 0.0f)); moved = true; }
    if (IsKeyDown(KEY_D)) { Main::physicsEngine.set_dynamic_angular_velocity(bodyid, Vector3(0.0f, -20.0f, 0.0f)); moved = true; }
    if (moved) { PlayerAuthInput pkt; SocketClient::sendPacket(&pkt); std::cout << "PAI sent" << std::endl; }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    Vector3 cofm = Main::physicsEngine.get_body_cofm(bodyid);

    std::string velocityText = "Velocity x: " + std::to_string(cv.x) +
                           " y: " + std::to_string(cv.y) +
                           " z: " + std::to_string(cv.z);
    std::string positionText = "Position x: " + std::to_string(cp.x) +
                       " y: " + std::to_string(cp.y) +
                       " z: " + std::to_string(cp.z);
    std::string rotationText = "Rotation x: " + std::to_string(cr.x) +
                       " y: " + std::to_string(cr.y) +
                       " z: " + std::to_string(cr.z);
    std::string angularText = "Angular x: " + std::to_string(ca.x) +
                      " y: " + std::to_string(ca.y) +
                      " z: " + std::to_string(ca.z);
    std::string cofmText = "CofM x: " + std::to_string(cofm.x) +
                      " y: " + std::to_string(cofm.y) +
                      " z: " + std::to_string(cofm.z);
    DrawText(velocityText.c_str(), 10, 10, 10, BLACK);
    DrawText(positionText.c_str(), 10, 20, 10, BLACK);
    DrawText(rotationText.c_str(), 10, 30, 10, BLACK);
    DrawText(angularText.c_str(), 10, 40, 10, BLACK);
    DrawText(cofmText.c_str(), 10, 50, 10, BLACK);

    BeginMode3D(camera);

    setupAndRender();

    EndMode3D();
    EndDrawing();

    auto cpd = Main::physicsEngine.get_body_position(bodyid) - cp;
    camera.target = cp;
    camera.position = Vector3(camera.position.x + cpd.x, camera.position.y + cpd.y, camera.position.z + cpd.z);
}

int main() {
    bool ticklooprunning = true;

    Main::physicsEngine.initialise();
    Main::physicsEngine.start_simulation();

    emscripten_get_canvas_element_size("#canvas", &windowWidth, &windowHeight);
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, onResize);
    emscripten_request_pointerlock("canvas", EM_TRUE);

    L_SUBSCRIBE(TickEvent, [](TickEvent* event) {
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
    Main::localTank->object = std::make_shared<DynamicObject>(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 5.0f, 0.0f), std::make_shared<Model>(cubeModel), BLUE);
    Armor armor = Armor(1.0f, 100.0f);
    Mesh armorMesh = GenMeshCube(armor.thickness / 1000, 1.0f, 1.0f);
    Model armorModel = LoadModelFromMesh(armorMesh);
    Main::localTank->add_armor(std::make_shared<ArmorObject>(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), std::make_shared<Model>(armorModel), YELLOW, armor, Main::localTank->object, Vector3(2.0f, 0, 0), Vector3(0.0f, 0.0f, 0.0f)));
    Main::localTank->object->create_physics();

    // for (auto& e : Main::localTank->armorplates) {
    //     e->create_physics();
    // }

    emscripten_set_main_loop(mainLoop, 0, 1);
    ticklooprunning = false;
    tick.join();
    CloseWindow();

    return 0;
}

