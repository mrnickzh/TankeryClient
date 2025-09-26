#include "ArmorObject.hpp"

#include <raymath.h>

#include "../main.hpp"
#include "../Utils/PositionConverter.h"

void ArmorObject::render() {
    float yawRad = parentobject->rotation.y * DEG2RAD;
    float pitchRad = parentobject->rotation.x * DEG2RAD;

    Vector3 rotatedposition = Vector3(0, 0, 0);
    rotatedposition.x = offsetposition.x * std::sin(yawRad) * std::cos(pitchRad);
    rotatedposition.y = offsetposition.x * std::sin(pitchRad);
    rotatedposition.z = offsetposition.x * std::cos(yawRad) * std::cos(pitchRad);

    Vector3 realrotation = parentobject->rotation + offsetrotation;
    armormodel.transform = MatrixRotateXYZ(realrotation * DEG2RAD);
    Vector3 realposition = parentobject->position + Vector3(offsetposition.z + rotatedposition.z, offsetposition.y + rotatedposition.y, rotatedposition.x);
    DrawModel(armormodel, realposition, 1.0f, armorcolor);
    position = realposition;
    rotation = realrotation;
}

void ArmorObject::create_physics() {
    Vector3 realposition = parentobject->position + offsetposition;
    Vector3 realrotation = rotation + offsetrotation;
    bodyid = Main::physicsEngine.create_sensor(PositionConverter::getHalfExtent(Vector3(1.0f, armorstats.thickness / 1000, 1.0f)), realposition, realrotation);
}
