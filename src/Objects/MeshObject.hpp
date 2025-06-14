#pragma once
#include <memory>
#include <raylib.h>

#include "Object.hpp"

class MeshObject : public Object {
public:
    Model meshmodel;
    MeshObject(Vector3 rotation, Vector3 position, std::shared_ptr<Model> model) : Object(rotation, position) {meshmodel = *model;};

    void render() override;
};