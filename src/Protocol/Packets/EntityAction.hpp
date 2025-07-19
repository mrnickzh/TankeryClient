#pragma once

#include "../Packet.hpp"
#include "../../main.hpp"
#include "../../Entity/Entity.hpp"

class EntityAction : public Packet {
public:
    void receive(ByteBuf &buffer) override {
        int action = buffer.readInt();
        std::string uuidStr = buffer.readString();

        if (action == 0) {
            std::shared_ptr<Entity> e = std::make_shared<Entity>();
            e->uuid = uuidStr;
            e->object = std::make_shared<DynamicObject>(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f), std::make_shared<Model>(LoadModelFromMesh(GenMeshCube(1.0f,1.0f,1.0f))), BLUE);
            e->object->create_physics();
            if (std::find_if(Main::entities.begin(), Main::entities.end(), [uuidStr](std::shared_ptr<Entity> f)->bool{ return (*f).uuid == uuidStr; }) == Main::entities.end()) {
                Main::entities.push_back(e);
                std::cout << e->uuid << " uuid" << std::endl;
                std::cout << Main::entities.size() << std::endl;
            }
        } else if (action == 1) {
            auto &entities = Main::entities;
            entities.erase(
                    std::remove_if(entities.begin(), entities.end(),
                                   [&uuidStr](const std::shared_ptr<Entity>& e) {
                                       return e->uuid == uuidStr;
                                   }),
                    entities.end()
            );
        }
    }

    void send(ByteBuf &buffer) override {}
};