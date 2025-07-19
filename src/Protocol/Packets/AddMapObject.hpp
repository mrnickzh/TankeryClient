#pragma once

#include "../Packet.hpp"
#include "../../main.hpp"
#include "../../Objects/StaticObject.hpp"
#include "../../Objects/FloorObject.hpp"

class AddMapObject : public Packet {
    void receive(ByteBuf &buffer) override {
        int id = buffer.readInt();
        Vector3 rotation;
        Vector3 position;
        position.x = buffer.readFloat();
        position.y = buffer.readFloat();
        position.z = buffer.readFloat();
        rotation.x = buffer.readFloat();
        rotation.y = buffer.readFloat();
        rotation.z = buffer.readFloat();

        switch (id) {
            case 0: {
                Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
                Model cubeModel = LoadModelFromMesh(cubeMesh);
                std::shared_ptr<StaticObject> staticObject = std::make_shared<StaticObject>(
                    rotation, position, std::make_shared<Model>(cubeModel), RED);
                staticObject->create_physics();
                Main::map.push_back(staticObject);
            }
                break;
            case 1: {
                Mesh floorMesh = GenMeshCube(100.0f,1.0f,100.0f);
                Model floorModel = LoadModelFromMesh(floorMesh);
                std::shared_ptr<FloorObject> floorObject = std::make_shared<FloorObject>(Vector3(0.0f, 0.0f, 0.0f),  Vector3(0.0f, -1.0f, 0.0f), std::make_shared<Model>(floorModel), GREEN);
                floorObject->create_physics();
                Main::map.push_back(floorObject);
                break;
            }
        }
    }
    void send(ByteBuf &buffer) override {}
};