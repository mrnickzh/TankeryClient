#include "MeshObject.hpp"

#include <iostream>
#include <raymath.h>

#include "../main.hpp"

void MeshObject::render() {
    meshmodel.transform = MatrixMultiply(MatrixRotateXYZ(rotation * DEG2RAD), meshmodel.transform);
    DrawModel(meshmodel, position, 1.0f, meshcolor);
}