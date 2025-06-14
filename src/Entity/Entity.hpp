#pragma once

#include <string>

#include "Components/Position.hpp"
#include "Components/Velocity.hpp"

class Entity {
public:
    std::string uuid;
    VelocityComponent* velocity;
    PositionComponent* position;
    Entity() {
        velocity = new VelocityComponent(Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 0.f));
        position = new PositionComponent(Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 0.f));
    }

    bool operator==(const Entity& other) const {
        return this->uuid == other.uuid;
    }
};