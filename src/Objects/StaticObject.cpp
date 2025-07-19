#include "StaticObject.hpp"

#include <iostream>
#include <raymath.h>

#include "../main.hpp"
#include "../Utils/PositionConverter.h"

void StaticObject::render() {
    staticmodel.transform = MatrixRotateXYZ(rotation * DEG2RAD);
    DrawModel(staticmodel, position, 1.0f, staticcolor);
}

void StaticObject::create_physics() {
    bodyid = Main::physicsEngine.create_static(PositionConverter::getHalfExtent(Vector3(1.0f, 1.0f, 1.0f)), position, rotation);
}
