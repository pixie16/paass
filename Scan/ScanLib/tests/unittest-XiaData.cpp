///@file unittest-XiaData.cpp
///@brief A program that will execute unit tests on XiaData
///@author S. V. Paulauskas
///@date December 5, 2016
#include <vector>

#include <cmath>

#include <UnitTest++.h>

#include "XiaData.hpp"

using namespace std;

static const bool test_bool = true;
static const double test_baseline = 32.2;
static const double test_energy = 123.4;
static const unsigned int test_cfd_time = 124;
static const unsigned int test_channel_number = 13;
static const unsigned int test_crate_number = 0;
static const unsigned int test_event_time_high = 1234;
static const unsigned int test_event_time_low = 123456789;
static const unsigned int test_external_time_high = 4321;
static const unsigned int test_external_time_low = 987654321;
static const unsigned int test_slot_number = 3;
static const unsigned int test_module_number = 1;
static const vector<unsigned int> test_energy_sums = {12, 13, 14};
static const vector<unsigned int> test_qdc = {12, 13, 14, 78, 23, 34, 35, 6};
static const vector<unsigned int> test_trace = {12, 12, 12, 12, 12, 75, 90, 54,
                                                32, 12, 12};
XiaData lhs, rhs;

TEST_FIXTURE(XiaData, Test_GetBaseline) {
    SetBaseline(test_baseline);
    CHECK_EQUAL(test_baseline, GetBaseline());
}

TEST_FIXTURE(XiaData, Test_GetId) {
    SetSlotNumber(test_slot_number);
    SetChannelNumber(test_channel_number);
    SetCrateNumber(test_crate_number);
    CHECK_EQUAL(test_crate_number * 208 + test_module_number * 16 +
                test_channel_number, GetId());
}

TEST_FIXTURE(XiaData, Test_GetSetCfdForcedTrig) {
    SetCfdForcedTriggerBit(test_bool);
    CHECK(GetCfdForcedTriggerBit());
}

TEST_FIXTURE(XiaData, Test_GetSetCfdFractionalTime) {
    SetCfdFractionalTime(test_cfd_time);
    CHECK_EQUAL(test_cfd_time, GetCfdFractionalTime());
}

TEST_FIXTURE(XiaData, Test_GetSetCfdTriggerSourceBit) {
    SetCfdTriggerSourceBit(test_bool);
    CHECK(GetCfdTriggerSourceBit());
}

TEST_FIXTURE(XiaData, Test_GetSetChannelNumber) {
    SetChannelNumber(test_channel_number);
    CHECK_EQUAL(test_channel_number, GetChannelNumber());
}

TEST_FIXTURE(XiaData, Test_GetSetCrateNumber) {
    SetCrateNumber(test_crate_number);
    CHECK_EQUAL(test_crate_number, GetCrateNumber());
}

TEST_FIXTURE(XiaData, Test_GetSetEnergy) {
    SetEnergy(test_energy);
    CHECK_EQUAL(test_energy, GetEnergy());
}

TEST_FIXTURE(XiaData, Test_GetSetEnergySums) {
    SetEnergySums(test_energy_sums);
    CHECK_ARRAY_EQUAL(test_energy_sums, GetEnergySums(),
                      test_energy_sums.size());
}

TEST_FIXTURE(XiaData, Test_GetSetEventTimeHigh) {
    SetEventTimeHigh(test_event_time_high);
    CHECK_EQUAL(test_event_time_high, GetEventTimeHigh());
}

TEST_FIXTURE(XiaData, Test_GetSetEventTimeLow) {
    SetEventTimeLow(test_event_time_low);
    CHECK_EQUAL(test_event_time_low, GetEventTimeLow());
}

TEST_FIXTURE(XiaData, Test_GetSetExternalTimeHigh) {
    SetExternalTimeHigh(test_external_time_high);
    CHECK_EQUAL(test_external_time_high, GetExternalTimeHigh());
}

TEST_FIXTURE(XiaData, Test_GetSetExternalTimeLow) {
    SetExternalTimeLow(test_external_time_low);
    CHECK_EQUAL(test_external_time_low, GetExternalTimeLow());
}

TEST_FIXTURE(XiaData, Test_GetSetPileup) {
    SetPileup(test_bool);
    CHECK(IsPileup());
}

TEST_FIXTURE(XiaData, Test_GetSetQdc) {
    SetQdc(test_qdc);
    CHECK_ARRAY_EQUAL(test_qdc, GetQdc(), test_qdc.size());
}

TEST_FIXTURE(XiaData, Test_GetSetSaturation) {
    SetSaturation(test_bool);
    CHECK(IsSaturated());
}

TEST_FIXTURE(XiaData, Test_GetSetSlotNumber) {
    SetSlotNumber(test_slot_number);
    CHECK_EQUAL(test_slot_number, GetSlotNumber());
}

TEST_FIXTURE(XiaData, Test_GetSetTrace) {
    SetTrace(test_trace);
    CHECK_ARRAY_EQUAL(test_trace, GetTrace(), test_trace.size());
}

TEST_FIXTURE(XiaData, Test_GetSetVirtualChannel) {
    SetVirtualChannel(test_bool);
    CHECK(IsVirtualChannel());
}

TEST_FIXTURE(XiaData, Test_GetTime) {
    SetTime(123456789.);
    CHECK_EQUAL(123456789, GetTime());
}

///This will test that the Time for the rhs is greater than the lhs
TEST(Test_CompareTime){
    lhs.Clear(); rhs.Clear();

    lhs.SetTime(123456789.);
    rhs.SetTime(123456799.);

    CHECK(lhs.CompareTime(&lhs, &rhs));
}

//This will test that the ID for the rhs is greater than the lhs
TEST(Test_CompareId) {
    lhs.Clear(); rhs.Clear();
    lhs.SetChannelNumber(test_channel_number);
    lhs.SetSlotNumber(test_slot_number);
    lhs.SetCrateNumber(test_crate_number);

    rhs.SetChannelNumber(test_channel_number);
    rhs.SetSlotNumber(test_slot_number+2);
    rhs.SetCrateNumber(test_crate_number);

    CHECK(lhs.CompareId(&lhs, &rhs));
}

TEST(Test_Equality) {
    lhs.Clear(); rhs.Clear();
    lhs.SetChannelNumber(test_channel_number);
    lhs.SetSlotNumber(test_slot_number);
    lhs.SetCrateNumber(test_crate_number);
    rhs = lhs;
    CHECK(lhs == rhs);
}

TEST(Test_LessThanOperator) {
    lhs.Clear(); rhs.Clear();
    lhs.SetTime(123456789);
    rhs = lhs;
    rhs.SetTime(123456799);
    CHECK(lhs < rhs);
}

int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}

