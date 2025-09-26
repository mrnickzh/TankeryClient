#pragma once
#include "../Entity.hpp"
#include "../../Objects/ArmorObject.hpp"

class LocalTank : public Entity {
public:
    std::vector<std::shared_ptr<ArmorObject>> armorplates;

    LocalTank() : Entity() {}

    void add_armor(std::shared_ptr<ArmorObject> armor) {
        armorplates.push_back(armor);
    }
};
