///@file unittest-HelperFunctions.cpp
///@author S. V. Paulauskas
///@date December 12, 2016
#include <UnitTest++.h>

#include "HelperFunctions.hpp"
#include "UnitTestSampleData.hpp"

using namespace std;
using namespace unittest_trace_variables;
using namespace unittest_helper_functions;

///This tests that the TraceFunctions::CalculateBaseline function works as
/// expected. This also verifies the Statistics functions CalculateAverage
/// and CalculateStandardDeviation
TEST(TestCalculateBaseline) {
        //Checking that we throw a range_error when the range is too small
        CHECK_THROW(TraceFunctions::CalculateBaseline(trace, make_pair(0, 1)),
                    range_error);

        //Checking that we throw a range_error when the data vector is sized 0
        CHECK_THROW(TraceFunctions::CalculateBaseline(empty_vector_uint,
        make_pair(0, 16)),
        range_error);

        //Checking that we throw a range_error when the range is inverted
        CHECK_THROW(TraceFunctions::CalculateBaseline(trace, make_pair(17, 1)),
        range_error);

        //Checking that we throw a range_error when the range is larger than the
        // data vector
        CHECK_THROW(TraceFunctions::CalculateBaseline(
        trace, make_pair(0, trace.size() + 100)), range_error);

        //Check that we are actually calculating the parameters properly
        pair<double, double> result =
        TraceFunctions::CalculateBaseline(trace, make_pair(0, 70));
        CHECK_CLOSE(baseline, result.first, 1e-7);
        CHECK_CLOSE(standard_deviation, result.second, 1e-9);
}

TEST(TestFindMaxiumum) {
        //Checking that we throw a range_error when the data vector is sized 0
        CHECK_THROW(TraceFunctions::FindMaximum(empty_vector_uint, trace_delay),
                    range_error);

        //Checking that we are throwing a range_error when delay is larger than
        // the size of the data vector
        CHECK_THROW(TraceFunctions::FindMaximum(trace, trace.size() + 100),
        range_error);

        //Checking that when the trace delay is smaller than the minimum number of
        // samples for the baseline we throw an error
        CHECK_THROW(TraceFunctions::FindMaximum(trace, 5), range_error);

        //Checking that we throw a range error if we could not find a maximum in
        // the specified range.
        CHECK_THROW(TraceFunctions::FindMaximum(const_vector_uint, trace_delay),
        range_error);

        pair<unsigned int, double> result =
        TraceFunctions::FindMaximum(trace, trace_delay);
        CHECK_EQUAL(max_position, result.first);
        CHECK_EQUAL(maximum_value, result.second);
}


TEST(TestFindLeadingEdge) {
        //Check that if we define a threshold that is 0 or less we throw a range
        // error
        CHECK_THROW(
                TraceFunctions::FindLeadingEdge(trace, bad_fraction, max_pair),
                range_error);
        //Check that we throw an error if if we have a vector that isn't big
        // enough to do proper analysis.
        CHECK_THROW(TraceFunctions::FindLeadingEdge(empty_vector_uint,
        leading_edge_fraction,
        max_pair), range_error);
        //Check that if we have a maximum position that is larger than the size
        // of the data vector we throw a range error.
        CHECK_THROW(TraceFunctions::FindLeadingEdge(
        trace, leading_edge_fraction, make_pair(trace.size() + 10, 3.)),
        range_error);

        //Check that we are getting what we expect for the leading edge
        ///@TODO We still need to fix this function so that it works properly
        CHECK_EQUAL(leading_edge_position,
        TraceFunctions::FindLeadingEdge(trace, leading_edge_fraction,
        max_pair));
}

TEST(TestCalculatePoly3) {
        //Check that we throw an error when the passed data vector is too small.
        CHECK_THROW(Polynomial::CalculatePoly3(empty_vector_uint, 0),
                    range_error);

        //A pair containing the data and the starting position of the fit
        pair<double, vector<double> > result =
        Polynomial::CalculatePoly3(poly3_data, 0);

        //Check that we are returning the correct coefficients for the data being
        // passed.
        CHECK_ARRAY_CLOSE(poly3_coeffs, result.second, poly3_coeffs.size(), 1e-6);

        //Check that the calculated maximum value is accurate
        CHECK_CLOSE(extrapolated_maximum, result.first, 1e-6);
}

//For determination of the extrapolated maximum value of the trace. This trace
// favors the left side since f(max+1) is less than f(max - 1).
TEST(TestExtrapolateMaximum) {
        //Check that we throw an error when the passed data vector is too small.
        CHECK_THROW(TraceFunctions::ExtrapolateMaximum(empty_vector_uint,
                                                       max_pair), range_error);

        pair<double, vector<double> > result =
        TraceFunctions::ExtrapolateMaximum(trace, max_pair);

        //Check that the calculated maximum value is accurate
        CHECK_CLOSE(extrapolated_maximum, result.first, 1e-6);

        //Check that we are returning the correct coefficients for the data being
        // passed.
        CHECK_ARRAY_CLOSE(extrapolated_max_coeffs, result.second,
        extrapolated_max_coeffs.size(), 1e-3);
}

TEST(TestCalculatePoly2) {
        //Check that we throw an error when the passed data vector is too small.
        CHECK_THROW(Polynomial::CalculatePoly2(empty_vector_uint, 0),
                    range_error);

        pair<double, vector<double> > result =
        Polynomial::CalculatePoly2(poly2_data, 0);

        //Check that we are returning the correct coefficients for the data being
        // passed.
        CHECK_ARRAY_CLOSE(poly2_coeffs, result.second,
        poly2_coeffs.size(), 1e-3);

        CHECK_CLOSE(poly2_val, result.first, 1e-4);
}

TEST(TestCalculateIntegral) {
        //Check that we throw an error if the data vector is too small.
        CHECK_THROW(Statistics::CalculateIntegral(empty_vector_uint),
                    range_error);
        CHECK_EQUAL(integral, Statistics::CalculateIntegral(integration_data));
}

TEST(TestCalculateQdc) {
        //Check that we are throwing an error when the data is empty
        CHECK_THROW(TraceFunctions::CalculateQdc(empty_vector_uint,
                                                 make_pair(0, 4)),
                    range_error);
        //Check that we are throwing an error when the range is too large
        CHECK_THROW(TraceFunctions::CalculateQdc(trace,
        make_pair(0, trace.size() + 10)),
        range_error);
        //Check for range error when the range is inverted
        CHECK_THROW(TraceFunctions::CalculateQdc(trace, make_pair(1000, 0)),
        range_error);
        //Check that we get the expected result
        CHECK_EQUAL(integration_qdc,
        TraceFunctions::CalculateQdc(integration_data, qdc_pair));
}

TEST(TestCalculateTailRatio) {
        //Check that we are throwing an error when the data is empty
        CHECK_THROW(TraceFunctions::CalculateTailRatio(empty_vector_uint,
                                                       make_pair(0, 4), 100.0),
                    range_error);
        //Check that the upper bound of the range is not bigger than the data size
        CHECK_THROW(TraceFunctions::CalculateTailRatio(trace,
        make_pair(0, trace.size() + 10), 100.0),
        range_error);
        //Check that the QDC is not zero
        CHECK_THROW(TraceFunctions::CalculateTailRatio(trace, make_pair(0, 4),
        0.0), range_error);
        double qdc = TraceFunctions::CalculateQdc(trace, make_pair(70, 91));
        pair<unsigned int, unsigned int> range(80, 91);
        double result = TraceFunctions::CalculateTailRatio(trace, range, qdc);
        CHECK_CLOSE(tail_ratio, result, 1e-6);
}

TEST(TestIeeeFloatingToDecimal) {
        unsigned int input = 1164725159;
        double expected = 3780.7283;
        CHECK_CLOSE(expected, IeeeStandards::IeeeFloatingToDecimal(input), 1e-4);
}

int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}
