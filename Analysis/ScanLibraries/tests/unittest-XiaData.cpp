///@file unittest-XiaData.cpp
///@brief A program that will execute unit tests on XiaData
///@author S. V. Paulauskas
///@date December 5, 2016
#include <vector>

#include <cmath>

#include <UnitTest++.h>

#include "UnitTestSampleData.hpp"
#include "XiaData.hpp"

using namespace std;
using namespace unittest_trace_variables;
using namespace unittest_decoded_data;

XiaData lhs, rhs;

TEST_FIXTURE(XiaData, Test_GetBaseline
) {
SetBaseline(baseline);
CHECK_EQUAL(baseline, GetBaseline()
);
}

TEST_FIXTURE(XiaData, Test_GetId
) {
SetSlotNumber(slotId);
SetChannelNumber(channelNumber);
SetCrateNumber(crateId);
CHECK_EQUAL(crateId
* 208 +

GetModuleNumber()

* 16 +
channelNumber,

GetId()

);
}

TEST_FIXTURE(XiaData, Test_GetSetCfdForcedTrig
) {
SetCfdForcedTriggerBit(cfd_forced_trigger);

CHECK (GetCfdForcedTriggerBit());

}

TEST_FIXTURE(XiaData, Test_GetSetCfdFractionalTime
) {
SetCfdFractionalTime(cfd_fractional_time);
CHECK_EQUAL(cfd_fractional_time, GetCfdFractionalTime()
);
}

TEST_FIXTURE(XiaData, Test_GetSetCfdTriggerSourceBit
) {
SetCfdTriggerSourceBit(cfd_source_trigger_bit);

CHECK (GetCfdTriggerSourceBit());

}

TEST_FIXTURE(XiaData, Test_GetSetChannelNumber
) {
SetChannelNumber(channelNumber);
CHECK_EQUAL(channelNumber, GetChannelNumber()
);
}

TEST_FIXTURE(XiaData, Test_GetSetCrateNumber
) {
SetCrateNumber(crateId);
CHECK_EQUAL(crateId, GetCrateNumber()
);
}

TEST_FIXTURE(XiaData, Test_GetSetEnergy
) {
SetEnergy(energy);
CHECK_EQUAL(energy, GetEnergy()
);
}

TEST_FIXTURE(XiaData, Test_GetSetEnergySums
) {
SetEnergySums(energy_sums);
CHECK_ARRAY_EQUAL(energy_sums, GetEnergySums(),
        energy_sums
.

size()

);
}

TEST_FIXTURE(XiaData, Test_GetSetEventTimeHigh
) {
SetEventTimeHigh(ts_high);
CHECK_EQUAL(ts_high, GetEventTimeHigh()
);
}

TEST_FIXTURE(XiaData, Test_GetSetEventTimeLow
) {
SetEventTimeLow(ts_low);
CHECK_EQUAL(ts_low, GetEventTimeLow()
);
}

TEST_FIXTURE(XiaData, Test_GetSetExternalTimeHigh
) {
SetExternalTimeHigh(ex_ts_high);
CHECK_EQUAL(ex_ts_high, GetExternalTimeHigh()
);
}

TEST_FIXTURE(XiaData, Test_GetSetExternalTimeLow
) {
SetExternalTimeLow(ex_ts_low);
CHECK_EQUAL(ex_ts_low, GetExternalTimeLow()
);
}

TEST_FIXTURE(XiaData, Test_GetSetPileup
) {
SetPileup(pileup_bit);

CHECK (IsPileup());

}

TEST_FIXTURE(XiaData, Test_GetSetQdc
) {
SetQdc(qdc);
CHECK_ARRAY_EQUAL(qdc, GetQdc(), qdc
.

size()

);
}

TEST_FIXTURE(XiaData, Test_GetSetSaturation
) {
SetSaturation(trace_out_of_range);

CHECK (IsSaturated());

}

TEST_FIXTURE(XiaData, Test_GetSetSlotNumber
) {
SetSlotNumber(slotId);
CHECK_EQUAL(slotId, GetSlotNumber()
);
}

TEST_FIXTURE(XiaData, Test_GetSetTrace
) {
SetTrace(trace);
CHECK_ARRAY_EQUAL(trace, GetTrace(), trace
.

size()

);
}

TEST_FIXTURE(XiaData, Test_GetSetVirtualChannel
) {
SetVirtualChannel(virtual_channel);

CHECK (IsVirtualChannel());

}

TEST_FIXTURE(XiaData, Test_GetTime
) {
SetTime(ts);
CHECK_EQUAL(ts, GetTime()
);
}

///This will test that the Time for the rhs is greater than the lhs
TEST(Test_CompareTime){
        lhs.Clear(); rhs.Clear();

        lhs.SetTime(ts);
        rhs.SetTime(ts+10);

        CHECK(lhs.CompareTime(&lhs, &rhs));
}

//This will test that the ID for the rhs is greater than the lhs
TEST(Test_CompareId) {
        lhs.Clear(); rhs.Clear();
        lhs.SetChannelNumber(channelNumber);
        lhs.SetSlotNumber(slotId);
        lhs.SetCrateNumber(crateId);

        rhs.SetChannelNumber(channelNumber);
        rhs.SetSlotNumber(slotId+2);
        rhs.SetCrateNumber(crateId);

        CHECK(lhs.CompareId(&lhs, &rhs));
}

TEST(Test_Equality) {
        lhs.Clear(); rhs.Clear();
        lhs.SetChannelNumber(channelNumber);
        lhs.SetSlotNumber(slotId);
        lhs.SetCrateNumber(crateId);
        rhs = lhs;
        CHECK(lhs == rhs);
}

TEST(Test_LessThanOperator) {
        lhs.Clear(); rhs.Clear();
        lhs.SetTime(ts);
        rhs = lhs;
        rhs.SetTime(ts+10);
        CHECK(lhs < rhs);
}

int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}

