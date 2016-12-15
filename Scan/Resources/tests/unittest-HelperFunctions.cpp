///@file unittest-HelperFunctions.cpp
///@author S. V. Paulauskas
///@date December 12, 2016
#include <stdexcept>
#include <utility>
#include <vector>

#include <UnitTest++.h>

#include "HelperFunctions.hpp"

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
//A data vector that contains constant data.
static const vector<unsigned int> const_data = {1000, 4};
//A data vector to test the integration
static const vector<unsigned int> integration_data = {0, 1, 2, 3, 4, 5};
//The expected value from the CalculateIntegral function
static const double expected_integral = 12.5;

//The expected maximum from the poly3 fitting
static const double expected_poly3_val = 3818.0718412264;
//The expected maximum from the pol2 fitting
static const double expected_poly2_val = 10737.0720588236;

//This is the expected value of the maximum
static const double expected_maximum_value = 3816;
//This is the expected position of the maximum
static const unsigned int expected_max_position = 76;
//This is the pair made from the expected maximum information
static const pair<unsigned int, double> expected_max_info(expected_max_position,
                                                          expected_maximum_value);

///This tests that the TraceFunctions::CalculateBaseline function works as
/// expected. This also verifies the Statistics functions CalculateAverage
/// and CalculateStandardDeviation
TEST(TestCalculateBaseline) {
    //These two values were obtained using the first 70 values of the above trace.
    //The expected baseline value was obtained using the AVERAGE function in
    // Google Sheets.
    static const double expected_baseline = 436.7428571;
    //The expected standard deviation was obtained using the STDEVP function in
    // Google Sheets.
    static const double expected_standard_deviation = 1.976184739;

    //Checking that we throw a range_error when the range is too small
    CHECK_THROW(TraceFunctions::CalculateBaseline(data, make_pair(0, 1)),
                range_error);

    //Checking that we throw a range_error when the data vector is sized 0
    CHECK_THROW(TraceFunctions::CalculateBaseline(empty_data, make_pair(0, 16)),
                range_error);

    //Checking that we throw a range_error when the range is inverted
    CHECK_THROW(TraceFunctions::CalculateBaseline(data, make_pair(17, 1)),
                range_error);

    //Checking that we throw a range_error when the range is larger than the
    // data vector
    CHECK_THROW(TraceFunctions::CalculateBaseline(data, make_pair(0, 1000)),
                range_error);

    //Check that we are actually calculating the parameters properly
    pair<double, double> result =
            TraceFunctions::CalculateBaseline(data, make_pair(0, 70));
    CHECK_CLOSE(expected_baseline, result.first, 1e-7);
    CHECK_CLOSE(expected_standard_deviation, result.second, 1e-9);
}

TEST(TestFindMaxiumum) {
    static const unsigned int trace_delay = 80;

    //Checking that we throw a range_error when the data vector is sized 0
    CHECK_THROW(TraceFunctions::FindMaximum(empty_data, trace_delay),
                range_error);

    //Checking that we are throwing a range_error when delay is larger than
    // the size of the data vector
    CHECK_THROW(TraceFunctions::FindMaximum(data, 1000), range_error);

    //Checking that when the trace delay is smaller than the minimum number of
    // samples for the baseline we throw an error
    CHECK_THROW(TraceFunctions::FindMaximum(data, 10), range_error);

    //Checking that we throw a range error if we could not find a maximum in
    // the specified range.
    CHECK_THROW(TraceFunctions::FindMaximum(const_data, trace_delay),
                range_error);

    pair<unsigned int, double> result =
            TraceFunctions::FindMaximum(data, trace_delay);
    CHECK_EQUAL(expected_max_position, result.first);
    CHECK_EQUAL(expected_maximum_value, result.second);
}

TEST(TestFindLeadingEdge) {
    //This is the expected position of the leading edge of signal.
    static const unsigned int expected_leading_edge_position = 72;

    //Check that if we define a threshold that is 0 or less we throw a range
    // error
    CHECK_THROW(TraceFunctions::FindLeadingEdge(data, -2, expected_max_info),
                range_error);
    //Check that we throw an error if if we have a vector that isn't big
    // enough to do proper analysis.
    CHECK_THROW(TraceFunctions::FindLeadingEdge(empty_data, 0.05,
                                                expected_max_info),
                range_error);
    //Check that if we have a maximum position that is larger than the size
    // of the data vector we throw a range error.
    CHECK_THROW(TraceFunctions::FindLeadingEdge(const_data, 0.05,
                                                make_pair(2000, 3)),
                range_error);

    //Check that we are getting what we expect for the leading edge
    ///@TODO We still need to fix this function so that it works properly
    CHECK_EQUAL(expected_leading_edge_position,
                TraceFunctions::FindLeadingEdge(data, 0.131,
                                                expected_max_info));
}

TEST(TestCalculatePoly3) {
    //A data vector that contains only the four points for the Poly3 Fitting.
    static const vector<unsigned int> poly3_data(data.begin() + 74,
                                                 data.begin() + 78);
    //A vector containing the coefficients obtained from gnuplot using the data
    // from pol3_data with an x value starting at 0
    static const vector<double> expected_poly3_coeffs =
            {2358.0, 1635.66666666667, -516.0, 31.3333333333333};

    //Check that we throw an error when the passed data vector is too small.
    CHECK_THROW(Polynomial::CalculatePoly3(empty_data, 0), range_error);

    pair<double, vector<double> > result =
            Polynomial::CalculatePoly3(poly3_data, 0);


    //Check that we are returning the correct coefficients for the data being
    // passed.
    CHECK_ARRAY_CLOSE(expected_poly3_coeffs, result.second, 4, 1e-6);

    //Check that the calculated maximum value is accurate
    CHECK_CLOSE(expected_poly3_val, result.first, 1e-6);
}

//For determination of the maximum value of the trace this traces favors the
// left side since max+1 is less than max - 1
TEST(TestExtrapolateMaximum) {
    static const vector<double> expected_coeffs =
            {-15641316.0007084, 592747.666694852, -7472.00000037373,
             31.3333333349849};

    //Check that we throw an error when the passed data vector is too small.
    CHECK_THROW(TraceFunctions::ExtrapolateMaximum(empty_data,
                                                   expected_max_info),
                range_error);

    pair<double, vector<double> > result =
            TraceFunctions::ExtrapolateMaximum(data, expected_max_info);

    //Check that the calculated maximum value is accurate
    CHECK_CLOSE(expected_poly3_val, result.first, 1e-6);

    //Check that we are returning the correct coefficients for the data being
    // passed.
    CHECK_ARRAY_CLOSE(expected_coeffs, result.second, 4, 1e-3);
}

TEST(TestCalculatePoly2) {
    //A data vector containing only the three points for the Poly2 fitting
    static const vector<unsigned int> poly2_data(data.begin() + 73,
                                                 data.begin() + 75);
    //Vector containing the expected coefficients from the poly 2 fit
    static const vector<double> expected_poly2_coeffs =
            {1122.0, 1278.5, -42.4999999999999};

    pair<double, vector<double> > result =
            Polynomial::CalculatePoly2(poly2_data, 0);

    CHECK_CLOSE(expected_poly2_val, result.first, 1e-4);
}

TEST(TestCalculateIntegral) {
    //Check that we throw an error if the data vector is too small.
    CHECK_THROW(Statistics::CalculateIntegral(empty_data), range_error);
    CHECK_EQUAL(expected_integral,
                Statistics::CalculateIntegral(integration_data));
}

TEST(TestCalculateQdc) {
    static const double expected = 6;
    //Check that we are throwing an error when the data is empty
    CHECK_THROW(TraceFunctions::CalculateQdc(empty_data, make_pair(0, 4)),
                range_error);
    //Check that we are throwing an error when the range is too large
    CHECK_THROW(TraceFunctions::CalculateQdc(data, make_pair(0, 1000)),
                range_error);
    CHECK_EQUAL(expected, TraceFunctions::CalculateQdc
            (integration_data, make_pair(2, 5)));
}

TEST(TestCalculateTailRatio) {
    static const double expected_ratio = 0.2960894762;
    //Check that we are throwing an error when the data is empty
    CHECK_THROW(TraceFunctions::CalculateTailRatio(empty_data, make_pair(0, 4),
                                                   100.0), range_error);
    //Check that the upper bound of the range is not too big
    CHECK_THROW(TraceFunctions::CalculateTailRatio(empty_data,
                                                   make_pair(0, 400), 100.0),
                range_error);
    //Check that the QDC we passed actually makes sense
    CHECK_THROW(TraceFunctions::CalculateTailRatio(data, make_pair(0, 4),
                                                   0.0), range_error);

    double qdc = TraceFunctions::CalculateQdc(data, make_pair(70, 91));
    pair<unsigned int, unsigned int> range(80, 91);
    double result = TraceFunctions::CalculateTailRatio(data, range, qdc);
    CHECK_CLOSE(expected_ratio, result, 1e-6);
}

int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}