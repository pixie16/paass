/// @file unittest-RootFitter.cpp
/// @brief Unit tests for the RootFitter class
/// @author S. V. Paulauskas
/// @date December 18, 2016
#include <iostream>
#include <stdexcept>

#include <UnitTest++.h>

#include "RootFitter.hpp"
#include "UnitTestSampleData.hpp"

using namespace std;
using namespace unittest_trace_variables;
using namespace unittest_fit_variables;

TEST_FIXTURE(RootFitter, TestRootFitter
) {
CHECK_THROW(CalculatePhase(empty_vector_double, fitting_parameters,
                           max_pair, baseline_pair), range_error
);

SetQdc(waveform_qdc);
CHECK_CLOSE(-0.581124,
CalculatePhase(waveform, fitting_parameters,
        max_pair, baseline_pair
), 1.);
}

int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}