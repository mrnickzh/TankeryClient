#pragma once
#include "../Entity.hpp"
#include "Components/PartsHealth.hpp"

class LocalTank : public Entity {
public:
    PartsHealth* health;

    LocalTank() : Entity() {
        health = new PartsHealth(100, 100, 100, 100, 100);
    }
};