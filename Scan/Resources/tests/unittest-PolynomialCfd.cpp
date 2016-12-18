///@file unittest-HelperFunctions.cpp
///@author S. V. Paulauskas
///@date December 12, 2016
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <UnitTest++.h>

#include "PolynomialCfd.hpp"

using namespace std;

//This is a trace taken from a medium VANDLE module.
static const vector<unsigned int> data = {
        437, 436, 434, 434, 437, 437, 438, 435, 434, 438, 439, 437, 438, 434,
        435, 439, 438, 434, 434, 435, 437, 440, 439, 435, 437, 439, 438, 435,
        436, 436, 437, 439, 435, 433, 434, 436, 439, 441, 436, 437, 439, 438,
        438, 435, 434, 434, 438, 438, 434, 434, 437, 440, 439, 438, 434, 436,
        439, 439, 437, 436, 434, 436, 438, 437, 436, 437, 440, 440, 439, 436,
        435, 437, 501, 1122, 2358, 3509, 3816, 3467, 2921, 2376, 1914, 1538,
        1252, 1043, 877, 750, 667, 619, 591, 563, 526, 458, 395, 403, 452, 478,
        492, 498, 494, 477, 460, 459, 462, 461, 460, 456, 452, 452, 455, 453,
        446, 441, 440, 444, 456, 459, 451, 450, 447, 445, 449, 456, 456, 455
};

//An empty data vector to test error checking.
static const vector<unsigned int> empty_data;

//The expected maximum from the poly3 fitting
static const double expected_poly3_val = 3818.0718412264;
//This is the expected position of the maximum
static const unsigned int expected_max_position = 76;
//This is the pair made from the expected maximum information
static const pair<unsigned int, double>
        expected_max_info(expected_max_position, expected_poly3_val);

//These two values were obtained using the first 70 values of the above trace.
//The expected baseline value was obtained using the AVERAGE function in
// Google Sheets.
static const double expected_baseline = 436.7428571;
//The expected standard deviation was obtained using the STDEVP function in
// Google Sheets.
static const double expected_standard_deviation = 1.976184739;

TEST_FIXTURE(PolynomialCfd, TestPolynomialCfd) {
    static const pair<double,double> pars(0.5, 2.);

    //Checking that we throw a range_error when the data vector is zero
    CHECK_THROW(CalculatePhase(empty_data, pars, expected_max_info),
                range_error);

    //Checking that we throw a range_error when the max index is too large
    // for the data

    static const pair<unsigned int, double> tmp(1000, expected_poly3_val);
    CHECK_THROW(CalculatePhase(data, pars, tmp), range_error);

    CHECK(-9999 != CalculatePhase(data, pars, expected_max_info));
    cout << CalculatePhase(data, pars, expected_max_info) << endl;
}

int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}