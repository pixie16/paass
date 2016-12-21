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
double PolynomialCfd::CalculatePhase(const std::vector<unsigned int> &data,
                                     const std::pair<double, double> &pars,
                                     const std::pair<unsigned int, double> &max,
                                     const std::pair<double, double> baseline) {
    if (data.size() == 0)
        throw range_error("PolynomialCfd::CalculatePhase - The data vector "
                                  "was empty!");
    if (data.size() < max.first)
        throw range_error("PolynomialCfd::CalculatePhase - The maximum "
                                  "position is larger than the size of the "
                                  "data vector.");

    vector<double> tmp;
    for(unsigned int i = 0; i < data.size(); i++)
        tmp.push_back(data[i] - baseline.first);

    double threshold = pars.first * (max.second - baseline.first);
    double phase = -9999;

    vector<double> result;
    for (unsigned int cfdIndex = max.first; cfdIndex > 0; cfdIndex--) {
        if (tmp[cfdIndex - 1] < threshold && tmp[cfdIndex] >= threshold) {
            // Fit the rise of the trace to a 2nd order polynomial.
            ///@TODO Fix this so that we do not need to baseline subtract the
            /// whole trace.
            result = Polynomial::CalculatePoly2(tmp, cfdIndex - 1).second;

            //We want to stop things here so that the user can do some
            // debugging of potential issues.
            if (result[2] > 0)
                throw range_error("PolynomialCfd::CalculatePhase : The "
                                          "calculated coefficients were for a"
                                          " concave-upward parabola. Try "
                                          "increasing your fraction to "
                                          "improve quality.");

            // Calculate the phase of the trace.
            phase = (-result[1] +
                     sqrt(result[1] * result[1] -
                                       4 * result[2] *
                                       (result[0] - threshold))) /
                    (2 * result[2]);

            break;
        }
    }
    return phase;
}