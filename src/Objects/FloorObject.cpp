#include "FloorObject.hpp"

#include <raymath.h>

#include "../main.hpp"
#include "../Utils/PositionConverter.h"

void FloorObject::render() {
    floormodel.transform = MatrixRotateXYZ(rotation * DEG2RAD);
    DrawModel(floormodel, position, 1.0f, floorcolor);
}

void FloorObject::create_physics() {
    bodyid = Main::physicsEngine.create_floor(PositionConverter::getHalfExtent(Vector3(100.0f, 1.0f, 100.0f)), position);
}
