///\file unittest-GslFitter.cpp
///\brief A small code to test the functionality of the FitDriver
///\author S. V. Paulauskas
///\date August 8, 2016
#include <iostream>

#include <UnitTest++.h>

#include "GslFitter.hpp"
#include "UnitTestSampleData.hpp"

using namespace std;
using namespace unittest_trace_variables;
using namespace unittest_fit_variables;

TEST_FIXTURE(GslFitter, TestGslFitter) {
    //We need to set the QDC before the fit
    SetQdc(21329.85714285);

    //Actually perform the fitting
    double phase = CalculatePhase(waveform, fitting_parameters,
                                  max_pair, baseline_pair);

    CHECK_CLOSE(0.8565802, GetAmplitude(), 0.1);
    CHECK_CLOSE(-0.0826487, phase, 1.);
}

int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}