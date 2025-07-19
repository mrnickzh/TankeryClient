#pragma once

#include <vector>

#include "Physics/physics.h"
#include "Entity/LocalTank/LocalTank.hpp"
#include "Objects/DynamicObject.hpp"
#include "Objects/Object.hpp"

namespace Main {
    inline std::vector<std::shared_ptr<Object>> objects;
    inline std::vector<std::shared_ptr<Object>> map;

    inline PhysicsEngine physicsEngine;
    inline LocalTank* localTank = nullptr;

    inline std::vector<std::shared_ptr<Entity>> entities;
}
