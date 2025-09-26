#pragma once

#include "../Packet.hpp"
#include "../../main.hpp"

class PlayerAuthInput : public Packet {
public:
    void receive(ByteBuf &buffer) override {
       std::string uuid = buffer.readString();
       for (auto& e : Main::entities) {
           if (e->uuid == uuid) {
               auto epos = Vector3(buffer.readFloat(), buffer.readFloat(), buffer.readFloat());
               auto erot = Vector3(buffer.readFloat(), buffer.readFloat(), buffer.readFloat());
               auto evel = Vector3(buffer.readFloat(), buffer.readFloat(), buffer.readFloat());
               Main::physicsEngine.set_body_position(e->object->bodyid, epos);
               Main::physicsEngine.set_body_rotation(e->object->bodyid, erot);
               Main::physicsEngine.set_dynamic_linear_velocity(e->object->bodyid, evel);
           }
       }
    }

    void send(ByteBuf &buffer) override {
        auto localid = Main::localTank->object->bodyid;
        Vector3 position = Main::physicsEngine.get_body_position(localid);
        Vector3 rotation = Main::physicsEngine.get_body_rotation(localid);
        Vector3 velocity = Main::physicsEngine.get_dynamic_linear_velocity(localid);

        buffer.writeFloat(position.x);
        buffer.writeFloat(position.y);
        buffer.writeFloat(position.z);

        buffer.writeFloat(rotation.x);
        buffer.writeFloat(rotation.y);
        buffer.writeFloat(rotation.z);

        buffer.writeFloat(velocity.x);
        buffer.writeFloat(velocity.y);
        buffer.writeFloat(velocity.z);
    }
};