#pragma once

#include <vector>

#include "Entity/LocalTank/LocalTank.hpp"
#include "Objects/MeshObject.hpp"
#include "Objects/Object.hpp"

namespace Main {
    inline std::vector<std::shared_ptr<Object>> objects;
    inline std::vector<std::shared_ptr<Object>> map;


    inline LocalTank* localTank = nullptr;
    inline std::shared_ptr<MeshObject> localTankObject = nullptr;

    inline std::vector<std::shared_ptr<Entity>> entities;
}
