///@file TraditionalCfd.cpp
///@brief Same CFD algorithm implemented by Xia LLC but offline.
///@author S. V. Paulauskas
///@date July 22, 2011

#include "XiaCfd.hpp"

/// Perform CFD analysis on the waveform
double XiaCfd::CalculatePhase(const double &F_/*=0.5*/,
                              const size_t &D_/*=1*/,
                              const size_t &L_/*=1*/) {
    if (size == 0 || baseline < 0) { return -9999; }
    if (!cfdvals)
        cfdvals = new double[size];

    double cfdMinimum = 9999;
    size_t cfdMinIndex = 0;

    phase = -9999;

    // Compute the cfd waveform.
    for (size_t cfdIndex = 0; cfdIndex < size; ++cfdIndex) {
        cfdvals[cfdIndex] = 0.0;
        if (cfdIndex >= L_ + D_ - 1) {
            for (size_t i = 0; i < L_; i++)
                cfdvals[cfdIndex] +=
                        F_ * (event->adcTrace[cfdIndex - i] - baseline) -
                        (event->adcTrace[cfdIndex - i - D_] - baseline);
        }
        if (cfdvals[cfdIndex] < cfdMinimum) {
            cfdMinimum = cfdvals[cfdIndex];
            cfdMinIndex = cfdIndex;
        }
    }

    // Find the zero-crossing.
    if (cfdMinIndex > 0) {
        // Find the zero-crossing.
        for (size_t cfdIndex = cfdMinIndex - 1; cfdIndex >= 0; cfdIndex--) {
            if (cfdvals[cfdIndex] >= 0.0 && cfdvals[cfdIndex + 1] < 0.0) {
                phase = cfdIndex - cfdvals[cfdIndex] /
                                   (cfdvals[cfdIndex + 1] - cfdvals[cfdIndex]);
                break;
            }
        }
    }

    return phase;
}