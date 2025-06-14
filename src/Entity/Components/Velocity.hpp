#pragma once

#include <raylib.h>

struct VelocityComponent{
    Vector3 velocity;
    Vector3 prev_velocity;
};