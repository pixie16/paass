///@file unittest-HelperFunctions.cpp
///@author S. V. Paulauskas
///@date December 12, 2016
#include <UnitTest++.h>

#include "HelperFunctions.hpp"
#include "UnitTestExampleTrace.hpp"

using namespace std;
using namespace unittest_trace_variables;

///This tests that the TraceFunctions::CalculateBaseline function works as
/// expected. This also verifies the Statistics functions CalculateAverage
/// and CalculateStandardDeviation
TEST(TestCalculateBaseline) {
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
    CHECK_THROW(TraceFunctions::FindLeadingEdge(empty_data, 0.5,
                                                expected_max_info),
                range_error);
    //Check that if we have a maximum position that is larger than the size
    // of the data vector we throw a range error.
    CHECK_THROW(TraceFunctions::FindLeadingEdge(const_data, 0.5,
                                                make_pair(2000, 3)),
                range_error);

    //Check that we are getting what we expect for the leading edge
    ///@TODO We still need to fix this function so that it works properly
    CHECK_EQUAL(expected_leading_edge_position,
                TraceFunctions::FindLeadingEdge(data, 0.131,
                                                expected_max_info));
}

TEST(TestCalculatePoly3) {
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
    //Check that we throw an error when the passed data vector is too small.
    CHECK_THROW(Polynomial::CalculatePoly2(empty_data, 0), range_error);

    pair<double, vector<double> > result =
            Polynomial::CalculatePoly2(poly2_data, 0);

    //Check that we are returning the correct coefficients for the data being
    // passed.
    CHECK_ARRAY_CLOSE(expected_poly2_coeffs, result.second, 3, 1e-3);

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