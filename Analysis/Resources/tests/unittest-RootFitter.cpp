/// @file unittest-RootFitter.cpp
/// @brief Unit tests for the RootFitter class
/// @author S. V. Paulauskas
/// @date December 18, 2016
//#include <UnitTest++.h>

#include "RootFitter.hpp"
#include "UnitTestSampleData.hpp"

using namespace std;
using namespace unittest_trace_variables;
using namespace unittest_fit_variables;


//TEST_FIXTURE(RootFitter, TestRootFitter) {
//    CHECK_THROW(CalculatePhase(empty_data, expected_trace_pars,
//                               expected_max_info, expected_baseline_pair),
//                range_error);
//
//    CalculatePhase(waveform, expected_trace_pars, expected_max_info,
//                   expected_baseline_pair);
//
//    //CHECK( != 0.0);
//}

int main(int argv, char *argc[]) {
    RootFitter fitter;
    fitter.CalculatePhase(waveform, fitting_parameters, max_pair,
                          baseline_pair);
    //return (UnitTest::RunAllTests());
}