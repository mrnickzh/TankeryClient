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
        buffer.writeFloat(Main::localTank->object->position.x);
        buffer.writeFloat(Main::localTank->object->position.y);
        buffer.writeFloat(Main::localTank->object->position.z);

        buffer.writeFloat(Main::localTank->object->rotation.x);
        buffer.writeFloat(Main::localTank->object->rotation.y);
        buffer.writeFloat(Main::localTank->object->rotation.z);

        auto vel = Main::physicsEngine.get_dynamic_linear_velocity(Main::localTank->object->bodyid);
        buffer.writeFloat(vel.x);
        buffer.writeFloat(vel.y);
        buffer.writeFloat(vel.z);
    }
};