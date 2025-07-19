#pragma once
#include <memory>
#include <raylib.h>

#include "Object.hpp"

class FloorObject : public Object {
public:
    Model floormodel;
    Color floorcolor;
    FloorObject(Vector3 rotation, Vector3 position, std::shared_ptr<Model> model, Color color) : Object(rotation, position) {floormodel = *model; floorcolor = color;};

    void render() override;
    void create_physics() override;
};