#pragma once

class Object {
public:
    Vector3 rotation;
    Vector3 position;

    Object(Vector3 rotation, Vector3 position) : rotation(rotation), position(position) {}

    virtual void render() {}
};
