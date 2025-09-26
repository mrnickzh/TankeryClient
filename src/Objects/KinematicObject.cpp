#include "KinematicObject.hpp"

#include <raymath.h>

#include "../main.hpp"
#include "../Utils/PositionConverter.h"

void KinematicObject::render() {
    kinematicmodel.transform = MatrixRotateXYZ(rotation * DEG2RAD);
    DrawModel(kinematicmodel, position, 1.0f, kinematiccolor);
}

void KinematicObject::create_physics() {
    bodyid = Main::physicsEngine.create_kinematic(PositionConverter::getHalfExtent(Vector3(1.0f, 1.0f, 1.0f)), position, rotation);
}
