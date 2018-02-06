/// @file SiPMTimingFunction.hpp
/// @brief A class to handle the processing of traces
/// @author D. P. Loureiro
/// @date July 26, 2017
#include <cmath>

#include "SiPMTimingFunction.hpp"

double SiPMTimingFunction::operator()(double *x, double *p) {
    double phase = p[0];
    double amplitude = p[1];
    double beta = p[2];
    double gamma = p[3];
    double baseline = p[4];

    double diff = x[0] - phase;

    if (x[0] < phase)
        return baseline;

    return amplitude * std::exp(-beta * diff) *
           (1 - std::exp(-std::pow(gamma * diff, 3.5))) + baseline;
}
