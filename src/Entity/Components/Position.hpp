#pragma once
#include <raylib.h>

struct PositionComponent{
    Vector3 position;
    Vector3 rotation;
    Vector3 prevPosition;
    Vector3 prevRotation;
};
