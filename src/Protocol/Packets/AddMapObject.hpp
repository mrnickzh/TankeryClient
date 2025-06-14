#pragma once

#include "../Packet.hpp"
#include "../../main.hpp"
#include "../../Objects/MeshObject.hpp"

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
            case 0:
                Mesh cubeMesh = GenMeshCube(1.0f,1.0f,1.0f);
                Model cubeModel = LoadModelFromMesh(cubeMesh);
                Main::map.push_back(std::make_shared<MeshObject>(rotation,  position, std::make_shared<Model>(cubeModel)));
                break;
        }
    }
    void send(ByteBuf &buffer) override {}
};