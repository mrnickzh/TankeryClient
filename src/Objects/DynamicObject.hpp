#pragma once
#include <memory>
#include <raylib.h>

#include "Object.hpp"

class DynamicObject : public Object {
public:
    Model dynamicmodel;
    Color dynamiccolor;
    DynamicObject(Vector3 rotation, Vector3 position, std::shared_ptr<Model> model, Color color) : Object(rotation, position) {dynamicmodel = *model; dynamiccolor = color;};

    void render() override;
    void create_physics() override;
};