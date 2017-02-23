/// @file VandleTimingFunction.hpp
/// @brief A class to handle the processing of traces
/// @author S. V. Paulauskas
/// @date October 3, 2014
#include <cmath>

#include "VandleTimingFunction.hpp"

///This defines the stock VANDLE timing function. Here is a breakdown of the
/// parameters:
/// * p[0] = phase
/// * p[1] = amplitude
/// * p[2] = beta
/// * p[3] = gamma
/// * p[4] = baseline
double VandleTimingFunction::operator()(double *x, double *p) {
    if (x[0] < p[0])
        return p[4];

    return p[1] * std::exp(-p[2] * (x[0] - p[0])) *
           (1 - std::exp(-std::pow(p[3] * (x[0] - p[0]), 4.))) + p[4];
}
