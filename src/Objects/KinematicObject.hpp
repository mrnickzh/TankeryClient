#pragma once
#include <memory>
#include <raylib.h>

#include "Object.hpp"

class KinematicObject : public Object {
public:
    Model kinematicmodel;
    Color kinematiccolor;
    KinematicObject(Vector3 rotation, Vector3 position, std::shared_ptr<Model> model, Color color) : Object(rotation, position) {kinematicmodel = *model; kinematiccolor = color;};

    void render() override;
    void create_physics() override;
};