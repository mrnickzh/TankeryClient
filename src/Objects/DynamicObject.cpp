#include "DynamicObject.hpp"

#include <iostream>
#include <raymath.h>

#include "../main.hpp"
#include "../Utils/PositionConverter.h"

void DynamicObject::render() {
    dynamicmodel.transform = MatrixRotateXYZ(rotation * DEG2RAD);
    DrawModel(dynamicmodel, position, 1.0f, dynamiccolor);
}

void DynamicObject::create_physics() {
    bodyid = Main::physicsEngine.create_dynamic(PositionConverter::getHalfExtent(Vector3(1.0f, 1.0f, 1.0f)), position, rotation);
}
