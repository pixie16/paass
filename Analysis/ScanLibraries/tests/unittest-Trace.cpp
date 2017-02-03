///@file unittest-Trace.cpp
///@brief A program that will execute unit tests on Trace
///@author S. V. Paulauskas
///@date February 3, 2017
#include <vector>

#include <cmath>

#include <UnitTest++.h>

#include "UnitTestSampleData.hpp"
#include "Trace.hpp"

using namespace std;
using namespace unittest_trace_variables;
using namespace unittest_decoded_data;

TEST_FIXTURE(Trace, TestingGettersAndSetters){
    SetBaseline(baseline_pair);
    CHECK_EQUAL(baseline_pair.first, GetBaselineInfo().first);
    CHECK_EQUAL(baseline_pair.second, GetBaselineInfo().second);

    SetMax(max_pair);
    CHECK_EQUAL(max_pair.first, GetMaxInfo().first);
    CHECK_EQUAL(max_pair.second, GetMaxInfo().second);

    SetWaveformRange(waveform_range);
    CHECK_EQUAL(waveform_range.first, GetWaveformRange().first);
    CHECK_EQUAL(waveform_range.second, GetWaveformRange().second);

    SetTraceSansBaseline(trace_sans_baseline);
    CHECK_ARRAY_EQUAL(trace_sans_baseline, GetTraceSansBaseline(),
                      trace_sans_baseline.size());

    CHECK_ARRAY_EQUAL(waveform, GetWaveform(), waveform.size());

    SetTriggerFilter(trace_sans_baseline);
    CHECK_ARRAY_EQUAL(trace_sans_baseline, GetTriggerFilter(),
                      trace_sans_baseline.size());

    SetEnergySums(waveform);
    CHECK_ARRAY_EQUAL(waveform, GetEnergySums(), waveform.size());

    SetQdc(100.);
    CHECK_EQUAL(100., GetQdc());

    SetExtrapolatedMax(extrapolated_maximum_pair);
    CHECK_EQUAL(extrapolated_maximum_pair.first,
                GetExtrapolatedMaxInfo().first);
    CHECK_EQUAL(extrapolated_maximum_pair.second,
                GetExtrapolatedMaxInfo().second);

    SetIsSaturated(true);
    CHECK(IsSaturated());
}

int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}