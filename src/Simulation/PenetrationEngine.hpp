#pragma once

class PenetrationEngine {
public:
    void simulate_hit();
    float calculate_maxpen(float velocity, float mass, float caliber, float angle, float armor);
    float calculate_postvel(float velocity, float mass, float caliber, float angle, float armor, float thickness);
};