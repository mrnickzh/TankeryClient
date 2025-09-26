#pragma once
#include <memory>
#include <raylib.h>

#include "Object.hpp"
#include "DynamicObject.hpp"
#include "../Simulation/Armor.hpp"

class ArmorObject : public Object {
public:
    Model armormodel;
    Color armorcolor;
    Armor armorstats;
    std::shared_ptr<DynamicObject> parentobject;
    Vector3 offsetposition;
    Vector3 offsetrotation;
    ArmorObject(Vector3 rotation, Vector3 position, std::shared_ptr<Model> model, Color color, Armor armor, std::shared_ptr<DynamicObject> parent, Vector3 offsetpos, Vector3 offsetrot) : Object(rotation, position) {armormodel = *model; armorcolor = color; armorstats = armor; parentobject = parent; offsetposition = offsetpos; offsetrotation = offsetrot; };

    void render() override;
    void create_physics() override;
};