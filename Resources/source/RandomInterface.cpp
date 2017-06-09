/**@file RandomInterface.cpp
 * @brief Provides a singleton interface for random numbers
 * @author D. Miller and S. V. Paulauskas
 * @date May 2017
 */
#include <chrono>

#include "RandomInterface.hpp"

RandomInterface *RandomInterface::instance = NULL;

RandomInterface *RandomInterface::get() {
    if (!instance)
        instance = new RandomInterface();
    return instance;
}

RandomInterface::RandomInterface() {
    engine_ = std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count());
    distribution_ = std::uniform_real_distribution<double>(0.0, 1.0);
}

double RandomInterface::Generate(const double &range/*=1*/) {
    return distribution_(engine_) * range;
}
