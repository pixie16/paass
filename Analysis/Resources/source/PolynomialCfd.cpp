/// @file PolynomialCfd.cpp
/// @brief Timing method that calculates the timing using a Polynomial based
/// CFD.
/// @author C. R. Thornsberry and S. V. Paulauskas
/// @date December 6, 2016
#include <cmath>

#include "HelperFunctions.hpp"
#include "PolynomialCfd.hpp"

using namespace std;

/// Perform CFD analysis on the waveform.
double PolynomialCfd::CalculatePhase(const std::vector<double> &data, const std::pair<double, double> &pars,
                                     const std::pair<unsigned int, double> &max,
                                     const std::pair<double, double> baseline) {
    if (data.size() == 0)
        throw range_error("PolynomialCfd::CalculatePhase - The data vector was empty!");
    if (data.size() < max.first)
        throw range_error("PolynomialCfd::CalculatePhase - The maximum "
                                  "position is larger than the size of the "
                                  "data vector.");

    //double threshold = pars.first * max.second;  
    // For now the threshold fraction will remain hardcoded. The default parameter reader is not working correctly.
    double threshold = 0.45 * max.second;
    double phase = -9999;
    float multiplier = 1.;

    vector<double> result;
    for (unsigned int cfdIndex = max.first; cfdIndex > 0; cfdIndex--) {
        if (data[cfdIndex - 1] < threshold && data[cfdIndex] >= threshold) {
            // Fit the rise of the trace to a 2nd order polynomial.
            result = Polynomial::CalculatePoly1(data, cfdIndex - 1).second;

            /* //Second Order Phase calculation currenly commented out
            // Calculate the phase of the trace.
            if (result[2] > 1)
                multiplier = -1.;

            phase = (-result[1] + multiplier * sqrt(result[1] * result[1] - 4 * result[2] * (result[0] - threshold)))
                    / (2 * result[2]);
            */
            phase = ( threshold - result[0] ) / result[1];

            break;
        }
    }
    return phase;
}