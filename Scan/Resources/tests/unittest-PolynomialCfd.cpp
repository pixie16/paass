///@file unittest-HelperFunctions.cpp
///@author S. V. Paulauskas
///@date December 12, 2016
#include <iostream>
#include <utility>
#include <vector>

#include <UnitTest++.h>

#include "PolynomialCfd.hpp"
#include "UnitTestExampleTrace.hpp"

using namespace std;
using namespace unittest_trace_variables;

//This pair provides us with the expected extrapolated maximum and the
// position of the maximum.
static const pair<unsigned int, double> max_info(expected_max_position,
                                                 expected_poly3_val);

TEST_FIXTURE(PolynomialCfd, TestPolynomialCfd) {
    static const pair<double,double> pars(0.5, 2.);

    //Checking that we throw a range_error when the data vector is zero
    CHECK_THROW(CalculatePhase(empty_data, pars, max_info),
                range_error);

    //Checking that we throw a range_error when the max index is too large
    // for the data

    static const pair<unsigned int, double> tmp(1000, expected_poly3_val);
    CHECK_THROW(CalculatePhase(data, pars, tmp), range_error);

    CHECK(-9999 != CalculatePhase(data, pars, max_info));
    cout << CalculatePhase(data, pars, max_info) << endl;
}

int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}