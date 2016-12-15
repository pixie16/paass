//
// Created by vincent on 12/6/16.
//
#include <cmath>

#include "HelperFunctions.hpp"

using namespace std;

/// Perform CFD analysis on the waveform.
double PolynomialCfdCalculator::AnalyzeCFD(const double &f/*=0.5*/,
                                           vector<unsigned int> &trc, const
                                           double &maximum,
                                           const double &baseline) {
    if (size == 0 || baseline < 0) { return -9999; }

    double threshold = f * maximum + baseline;
    double phase = -9999;
    vector<double> coeffs;
    for (unsigned int cfdIndex = max_index; cfdIndex > 0; cfdIndex--) {
        if (event->adcTrace[cfdIndex - 1] < threshold &&
            event->adcTrace[cfdIndex] >= threshold) {
            // Fit the rise of the trace to a 2nd order polynomial.
            coeffs = Polynomial::CalculatePoly2(cfdIndex - 1,
                                       &event->adcTrace.data()[cfdIndex - 1],
                                                coeffs);

            // Calculate the phase of the trace.
            phase = (-cfdPar[5] + std::sqrt(cfdPar[5] * cfdPar[5] -
                                            4 * cfdPar[6] *
                                            (cfdPar[4] - threshold))) /
                    (2 * cfdPar[6]);

            break;
        }
    }
    return phase;
}