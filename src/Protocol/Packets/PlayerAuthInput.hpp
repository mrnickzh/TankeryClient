#pragma once

#include "../Packet.hpp"
#include "../../main.hpp"

class PlayerAuthInput : public Packet {
public:
    void receive(ByteBuf &buffer) override {
       std::string uuid = buffer.readString();
       for (auto& e : Main::entities) {
           if (e->uuid == uuid) {
               e->position->position = Vector3(buffer.readFloat(), buffer.readFloat(), buffer.readFloat());
               e->position->rotation = Vector3(buffer.readFloat(), buffer.readFloat(), buffer.readFloat());
               e->velocity->velocity = Vector3(buffer.readFloat(), buffer.readFloat(), buffer.readFloat());
               std::cout << e->velocity->velocity.x << " " << e->velocity->velocity.y << " " << e->velocity->velocity.z << std::endl;
           }
       }
    }

    void send(ByteBuf &buffer) override {
        buffer.writeFloat(Main::localTank->position->position.x);
        buffer.writeFloat(Main::localTank->position->position.y);
        buffer.writeFloat(Main::localTank->position->position.z);

        buffer.writeFloat(Main::localTank->position->rotation.x);
        buffer.writeFloat(Main::localTank->position->rotation.y);
        buffer.writeFloat(Main::localTank->position->rotation.z);

        std::cout << Main::localTank->velocity->velocity.x << " " << Main::localTank->velocity->velocity.y << " " << Main::localTank->velocity->velocity.z << " " << std::endl;

        buffer.writeFloat(Main::localTank->velocity->velocity.x);
        buffer.writeFloat(Main::localTank->velocity->velocity.y);
        buffer.writeFloat(Main::localTank->velocity->velocity.z);
    }
};