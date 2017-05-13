///@file unittest-TraditionalCfd.cpp
///@author S. V. Paulauskas
///@date December 12, 2016
#include <iostream>
#include <stdexcept>

#include <UnitTest++.h>

#include "TraditionalCfd.hpp"
#include "UnitTestSampleData.hpp"

using namespace std;
using namespace unittest_trace_variables;
using namespace unittest_cfd_variables;

TEST_FIXTURE(TraditionalCfd, TestTraditionalCfd
) {
//Checking that we throw a range_error when the data vector is zero
CHECK_THROW(CalculatePhase(empty_vector_double, cfd_test_pars, max_pair,
                           baseline_pair), range_error
);

//Check that we throw a range error when the max position is larger than
// the data we provided.
CHECK_THROW(CalculatePhase(trace_sans_baseline, cfd_test_pars,
                           make_pair(trace_sans_baseline.size() + 3, 100),
                           baseline_pair), range_error
);

//The expected value in this case is the value that I obtained after
// debugging the algorithm using other means. This check is here simply
// to tell us whether or not the algorithm has changed drastically from
// the "acceptable" value.
CHECK_CLOSE(75.1408,
CalculatePhase(trace_sans_baseline, cfd_test_pars,
        extrapolated_maximum_pair, baseline_pair
),
5);
}

int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}