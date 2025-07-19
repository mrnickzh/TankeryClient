#pragma once

#include <string>
#include "../Objects/DynamicObject.hpp"

class Entity {
public:
    std::string uuid;
    std::shared_ptr<DynamicObject> object;
    Entity() {}

    bool operator==(const Entity& other) const {
        return this->uuid == other.uuid;
    }
};