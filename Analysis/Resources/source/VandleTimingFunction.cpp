/// @file VandleTimingFunction.hpp
/// @brief A class to handle the processing of traces
/// @author S. V. Paulauskas
/// @date October 3, 2014
#include <cmath>

#include "VandleTimingFunction.hpp"

double VandleTimingFunction::operator()(double *x, double *p) {
    double phase = p[0];
    double amplitude = p[1];
    double beta = p[2];
    double gamma = p[3];
    double baseline = p[4];

    double diff = x[0] - phase;

    if (x[0] < phase)
        return baseline;

    return amplitude * std::exp(-beta * diff) *
           (1 - std::exp(-std::pow(gamma * diff, 4.))) + baseline;
}
