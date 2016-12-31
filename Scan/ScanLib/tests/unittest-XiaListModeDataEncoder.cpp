///@file unittest-XiaListModeDataEncoder.cpp
///@brief Unit tests for the XiaListModeDataDecoder class
///@author S. V. Paulauskas
///@author December 25, 2016
#include <stdexcept>

#include <UnitTest++.h>

#include "XiaListModeDataEncoder.hpp"

using namespace std;

//Here is all of the expected data for the XiaData.
static const unsigned int chan = 13;
static const unsigned int slot = 2;
static const unsigned int crate = 0;
static const unsigned int ts_high = 26001;
static const unsigned int ts_low = 123456789;
static const unsigned int ex_ts_high = 26001;
static const unsigned int ex_ts_low = 123456789;
static const unsigned int energy = 2345;
static const vector<unsigned int> trace = {
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
static const vector<unsigned int> qdc = {123, 456, 789, 987, 654,
                                         321, 135, 791};

TEST_FIXTURE(XiaListModeDataEncoder, TestEncodingThrows) {
    //Check that we throw a range error when we pass an empty XiaData class
    CHECK_THROW(EncodeXiaData(XiaData(), R30474, 250), invalid_argument);

    //Check that we catch an invalid_argument when we pass a bogus firmware
    CHECK_THROW(EncodeXiaData(XiaData(), UNKNOWN, 250), invalid_argument);

    //Check that we catch an invalid_argument when we pass a bogus frequency
    CHECK_THROW(EncodeXiaData(XiaData(), R30474, 2500), invalid_argument);
}

///Test if we can encode some headers with different information.
///@TODO Add headers for Esums and external TS.
TEST_FIXTURE(XiaListModeDataEncoder, TestSimpleHeaderEncoding) {
    vector<unsigned int> expected_header =
            {540717, 123456789, 26001, 2345};
    vector<unsigned int> expected_w_trc = {
            8667181, 123456789, 26001, 8128809,
            28574133, 28443058, 28639669, 28508598, 28705202, 28639671,
            28443062, 28770739, 28443062, 28508594, 28836277, 28508599,
            28770741, 28508598, 28574132, 28770741, 28377523, 28574130,
            28901815, 28639668, 28705207, 28508598, 28443058, 28705206,
            28443058, 28836277, 28705207, 28574130, 28770743, 28574133,
            28574130, 28639670, 28639668, 28836280, 28574135, 28639667,
            73531893, 229968182, 227217128, 155716457, 100796282, 68355300,
            49152877, 40567451, 36897359, 30016014, 26411403, 31326660,
            32637420, 31261166, 30081484, 30212558, 29884876, 29622724,
            29688263, 28901822, 29098424, 30081480, 29491651, 29163967,
            29884865, 29819336
    };
    vector<unsigned int> expected_w_qdc = {
            1622061, 123456789, 26001, 2345,
            123, 456, 789, 987, 654, 321, 135, 791
    };
    vector<unsigned int> expected_w_qdc_n_trc = {
            9748525, 123456789, 26001, 8128809,
            123, 456, 789, 987, 654, 321, 135, 791,
            28574133, 28443058, 28639669, 28508598, 28705202, 28639671,
            28443062, 28770739, 28443062, 28508594, 28836277, 28508599,
            28770741, 28508598, 28574132, 28770741, 28377523, 28574130,
            28901815, 28639668, 28705207, 28508598, 28443058, 28705206,
            28443058, 28836277, 28705207, 28574130, 28770743, 28574133,
            28574130, 28639670, 28639668, 28836280, 28574135, 28639667,
            73531893, 229968182, 227217128, 155716457, 100796282, 68355300,
            49152877, 40567451, 36897359, 30016014, 26411403, 31326660,
            32637420, 31261166, 30081484, 30212558, 29884876, 29622724,
            29688263, 28901822, 29098424, 30081480, 29491651, 29163967,
            29884865, 29819336
    };

    XiaData data;
    data.SetEnergy(energy);
    data.SetSlotNumber(slot);
    data.SetChannelNumber(chan);
    data.SetCrateNumber(crate);
    data.SetEventTimeLow(ts_low);
    data.SetEventTimeHigh(ts_high);

    //Check that we can handle just a simple 4 word header
    CHECK_ARRAY_EQUAL(expected_header, EncodeXiaData(data, R30474, 250),
                      expected_header.size());

    //Check that we can handle a header with a trace
    data.SetTrace(trace);
    CHECK_ARRAY_EQUAL(expected_w_trc, EncodeXiaData(data, R30474, 250),
                      expected_w_trc.size());

    //Check that we can handle a QDC
    data.SetQdc(qdc);
    data.SetTrace(vector<unsigned int>());
    CHECK_ARRAY_EQUAL(expected_w_qdc, EncodeXiaData(data, R30474, 250),
                      expected_w_qdc.size());

    //Check that we can handle a QDC and a Trace
    data.SetQdc(qdc);
    data.SetTrace(trace);
    CHECK_ARRAY_EQUAL(expected_w_qdc_n_trc, EncodeXiaData(data, R30474, 250),
                      expected_w_qdc_n_trc.size());
}

int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}