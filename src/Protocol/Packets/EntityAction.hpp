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
            Main::entities.push_back(e);
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