#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

class Object {
public:
    Vector3 rotation;
    Vector3 position;
    JPH::BodyID bodyid;
    Object(Vector3 rotation, Vector3 position) : rotation(rotation), position(position) {}

    virtual void render() {}
    virtual void create_physics() {}
};
