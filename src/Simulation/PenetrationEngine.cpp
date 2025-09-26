#include "PenetrationEngine.hpp"

#include <cmath>
#include <raylib.h>

void PenetrationEngine::simulate_hit() {

}

float PenetrationEngine::calculate_maxpen(float velocity, float mass, float caliber, float angle, float armor) {
    float maxpen = std::pow((velocity/armor), 1.43) * (std::pow(mass, 0.71) / std::pow(caliber, 1.07)) * std::pow(std::cos(angle * DEG2RAD), 1.4);
    return maxpen;
}

float PenetrationEngine::calculate_postvel(float velocity, float mass, float caliber, float angle, float armor, float thickness) {
    float spentvelocity = armor * (std::pow(caliber, 0.75) * std::pow(thickness, 0.7)) / (std::pow(mass, 0.5) * std::cos(angle * DEG2RAD));
    return velocity - spentvelocity;
}