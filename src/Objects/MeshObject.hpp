#pragma once
#include <memory>
#include <raylib.h>

#include "Object.hpp"

class MeshObject : public Object {
public:
    Model meshmodel;
    Color meshcolor;
    MeshObject(Vector3 rotation, Vector3 position, std::shared_ptr<Model> model, Color color) : Object(rotation, position) {meshmodel = *model; meshcolor = color;};

    void render() override;
};