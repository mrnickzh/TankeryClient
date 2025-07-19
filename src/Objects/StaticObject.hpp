#pragma once
#include <memory>
#include <raylib.h>

#include "Object.hpp"

class StaticObject : public Object {
public:
    Model staticmodel;
    Color staticcolor;
    StaticObject(Vector3 rotation, Vector3 position, std::shared_ptr<Model> model, Color color) : Object(rotation, position) {staticmodel = *model; staticcolor = color;};

    void render() override;
    void create_physics() override;
};