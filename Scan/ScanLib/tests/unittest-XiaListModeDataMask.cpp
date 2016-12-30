///@file unittest-XiaListModeDataMask.cpp
///@brief Unit testing of the XiaListModeDataMask class
///@author S. V. Paulauskas
///@date December 29, 2016
#include <stdexcept>
#include <vector>

#include <UnitTest++.h>

#include "XiaListModeDataMask.hpp"

using namespace std;

TEST_FIXTURE(XiaListModeDataMask, TestXiaListModeDataMask) {
    //Check that we throw an invalid argument when we put in an unrecognized
    // option.
    CHECK_THROW(SetFirmware("test"), invalid_argument);

    //Checking that the if statement works as expected when converting to the
    // different firmware revisions.
    SetFirmware("R29432");
    CHECK_EQUAL(R29432, GetFirmware());

    SetFirmware("R30474");
    CHECK_EQUAL(R30474, GetFirmware());

    SetFirmware("R30980");
    CHECK_EQUAL(R30980, GetFirmware());

    SetFirmware("R30981");
    CHECK_EQUAL(R30981, GetFirmware());

    SetFirmware("R34688");
    CHECK_EQUAL(R34688, GetFirmware());

    //We do not need to test more than on version of these since they are
    // identical across all firmware versions.
    CHECK_EQUAL((unsigned int) 0x0000000F, GetChannelNumberMask().first);
    CHECK_EQUAL((unsigned int) 0, GetChannelNumberMask().second);

    CHECK_EQUAL((unsigned int) 0x000000F0, GetSlotIdMask().first);
    CHECK_EQUAL((unsigned int) 4, GetSlotIdMask().second);

    CHECK_EQUAL((unsigned int) 0x00000F00, GetCrateIdMask().first);
    CHECK_EQUAL((unsigned int) 8, GetCrateIdMask().second);

    CHECK_EQUAL((unsigned int) 0x0001F000, GetHeaderLengthMask().first);
    CHECK_EQUAL((unsigned int) 12, GetHeaderLengthMask().second);

    CHECK_EQUAL((unsigned int) 0x1FFE0000, GetEventLengthMask().first);
    CHECK_EQUAL((unsigned int) 17, GetEventLengthMask().second);

    CHECK_EQUAL((unsigned int) 0x80000000, GetFinishCodeMask().first);
    CHECK_EQUAL((unsigned int) 31, GetFinishCodeMask().second);

    CHECK_EQUAL((unsigned int) 0x0000FFFF, GetEventTimeHighMask().first);
    CHECK_EQUAL((unsigned int) 0, GetEventTimeHighMask().second);
}

///Tests for the 100 MHz versions
TEST_FIXTURE(XiaListModeDataMask, Test_100MSps_Word2) {
    SetFrequency(100);

    SetFirmware("R29432");
    CHECK_EQUAL((unsigned int) 0xFFFF0000, GetCfdFractionalTimeMask().first);
    CHECK_EQUAL((unsigned int) 16, GetCfdFractionalTimeMask().second);

    CHECK_EQUAL((unsigned int) 0, GetCfdForcedTriggerBitMask().first);
    CHECK_EQUAL((unsigned int) 0, GetCfdForcedTriggerBitMask().second);

    CHECK_EQUAL((unsigned int) 0, GetCfdTriggerSourceMask().first);
    CHECK_EQUAL((unsigned int) 0, GetCfdTriggerSourceMask().second);

    vector<string> firm = {"R30474", "R30980", "R30981", "R34688"};
    for (vector<string>::iterator it = firm.begin(); it != firm.end(); it++) {
        SetFirmware(*it);
        CHECK_EQUAL((unsigned int) 0x7FFF0000,
                    GetCfdFractionalTimeMask().first);
        CHECK_EQUAL((unsigned int) 16, GetCfdFractionalTimeMask().second);

        CHECK_EQUAL((unsigned int) 0x80000000,
                    GetCfdForcedTriggerBitMask().first);
        CHECK_EQUAL((unsigned int) 31, GetCfdForcedTriggerBitMask().second);

        CHECK_EQUAL((unsigned int) 0, GetCfdTriggerSourceMask().first);
        CHECK_EQUAL((unsigned int) 0, GetCfdTriggerSourceMask().second);
    }
}

///Tests for the 250 MHz versions
TEST_FIXTURE(XiaListModeDataMask, Test_250Msps_Word2) {
    SetFrequency(250);

    SetFirmware("R29432");
    CHECK_EQUAL((unsigned int) 0x7FFF0000, GetCfdFractionalTimeMask().first);
    CHECK_EQUAL((unsigned int) 16, GetCfdFractionalTimeMask().second);

    CHECK_EQUAL((unsigned int) 0, GetCfdForcedTriggerBitMask().first);
    CHECK_EQUAL((unsigned int) 0, GetCfdForcedTriggerBitMask().second);

    CHECK_EQUAL((unsigned int) 0x80000000, GetCfdTriggerSourceMask().first);
    CHECK_EQUAL((unsigned int) 31, GetCfdTriggerSourceMask().second);

    vector<string> firm = {"R30474", "R30980", "R30981", "R34688"};
    for (vector<string>::iterator it = firm.begin(); it != firm.end(); it++) {
        SetFirmware(*it);
        CHECK_EQUAL((unsigned int) 0x3FFF0000,
                    GetCfdFractionalTimeMask().first);
        CHECK_EQUAL((unsigned int) 16, GetCfdFractionalTimeMask().second);

        CHECK_EQUAL((unsigned int) 0x80000000,
                    GetCfdForcedTriggerBitMask().first);
        CHECK_EQUAL((unsigned int) 31, GetCfdForcedTriggerBitMask().second);

        CHECK_EQUAL((unsigned int) 0x40000000, GetCfdTriggerSourceMask().first);
        CHECK_EQUAL((unsigned int) 30, GetCfdTriggerSourceMask().second);
    }
}

///Tests for the 500 MHz versions
TEST_FIXTURE(XiaListModeDataMask, Test_500MSps_Word2) {
    SetFrequency(500);

    vector<string> firm = {"R29432", "R30474", "R30980", "R30981", "R34688"};

    for (vector<string>::iterator it = firm.begin(); it != firm.end(); it++) {
        SetFirmware(*it);
        CHECK_EQUAL((unsigned int) 0x1FFF0000,
                    GetCfdFractionalTimeMask().first);
        CHECK_EQUAL((unsigned int) 16, GetCfdFractionalTimeMask().second);

        CHECK_EQUAL((unsigned int) 0, GetCfdForcedTriggerBitMask().first);
        CHECK_EQUAL((unsigned int) 0, GetCfdForcedTriggerBitMask().second);

        CHECK_EQUAL((unsigned int) 0xE0000000, GetCfdTriggerSourceMask().first);
        CHECK_EQUAL((unsigned int) 29, GetCfdTriggerSourceMask().second);
    }
}

TEST_FIXTURE(XiaListModeDataMask, Test_R29432_To_R30981_Word3) {
    vector<unsigned int> freq = {100, 250, 500};
    vector<string> firm = {"R29432", "R30474", "R30980", "R30981"};
    for (vector<string>::iterator it = firm.begin(); it != firm.end(); it++) {
        SetFirmware(*it);
        for (vector<unsigned>::iterator it1 = freq.begin();
             it1 != freq.end(); it1++) {
            SetFrequency(*it1);
            CHECK_EQUAL((unsigned int) 0x00007FFF, GetEventEnergyMask().first);
            CHECK_EQUAL((unsigned int) 0, GetEventEnergyMask().second);

            CHECK_EQUAL((unsigned int) 0x00008000,
                        GetTraceOutOfRangeFlagMask().first);
            CHECK_EQUAL((unsigned int) 15, GetTraceOutOfRangeFlagMask().second);

            CHECK_EQUAL((unsigned int) 0xFFFF0000, GetTraceLengthMask().first);
            CHECK_EQUAL((unsigned int) 16, GetTraceLengthMask().second);
        }
    }
}

TEST_FIXTURE(XiaListModeDataMask, Test_R34688_Word3) {
    SetFirmware("R34688");
    vector<unsigned int> freq = {100, 250, 500};
    for (vector<unsigned>::iterator it = freq.begin();
         it != freq.end(); it++) {
        SetFrequency(*it);
        CHECK_EQUAL((unsigned int) 0x0000FFFF, GetEventEnergyMask().first);
        CHECK_EQUAL((unsigned int) 0, GetEventEnergyMask().second);

        CHECK_EQUAL((unsigned int) 0x80000000,
                    GetTraceOutOfRangeFlagMask().first);
        CHECK_EQUAL((unsigned int) 31, GetTraceOutOfRangeFlagMask().second);

        CHECK_EQUAL((unsigned int) 0x7FFF0000, GetTraceLengthMask().first);
        CHECK_EQUAL((unsigned int) 16, GetTraceLengthMask().second);
    }
}

int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}