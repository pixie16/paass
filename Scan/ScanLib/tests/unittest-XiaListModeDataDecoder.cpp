///@file unittest-XiaListModeDataDecoder.cpp
///@brief Unit tests for the XiaListModeDataDecoder class
///@author S. V. Paulauskas
///@author December 25, 2016
#include <exception>
#include <UnitTest++.h>

#include "XiaListModeDataDecoder.hpp"

using namespace std;

///@TODO These need to be expanded so that we cover all of the nine different
/// firmware and frequency combinations.
static const XiaListModeDataMask mask(R30474, 250);

///We have added in the first to elements for the pixie module data header.
/// It contains the information about how many words are in the buffer and
/// the module VSN (number). This header is for firmware R30474
///This 4 word header has the following characteristics:
/// 1. Word 0
///     * Channel Number [3:0] : 13
///     * Slot ID [7:4] : 2
///     * CrateID [11:8]  : 0
///     * Header Length[16:12] : 4
///     * Event Length [30:17] : 4
///     * Finish Code [31] : 0
/// 2. Word 1
///     * EVTTIME_LO [0:31] : 123456789
/// 3. Word 2
///     * EVTTIME_HI [0:15] : 26001
///     * CFD Fractional Time [30:16] : 0
///     * CFD Trigger source bit [31] : 0
/// 4. Word 3
///     * Event Energy [0:14] : 2345
///     * Trace Out-of-Range Flag [15] : 0
///     * Trace Length [31:16] : 0
unsigned int header[6] = {4, 0, 540717, 123456789, 26001, 2345};

//The header is the standard 4 words. The trace is 62 words, which gives a
// trace length of 124. This gives us an event length of 66.
// We have 2 words for the Pixie Module Data Header.
unsigned int header_N_trace[68] = {
        66, 0, 8667181, 123456789, 26001, 8128809,
        28574133, 28443058, 28639669, 28508598, 28705202, 28639671, 28443062,
        28770739, 28443062, 28508594, 28836277, 28508599, 28770741, 28508598,
        28574132, 28770741, 28377523, 28574130, 28901815, 28639668, 28705207,
        28508598, 28443058, 28705206, 28443058, 28836277, 28705207, 28574130,
        28770743, 28574133, 28574130, 28639670, 28639668, 28836280, 28574135,
        28639667, 73531893, 229968182, 227217128, 155716457, 100796282,
        68355300, 49152877, 40567451, 36897359, 30016014, 26411403, 31326660,
        32637420, 31261166, 30081484, 30212558, 29884876, 29622724, 29688263,
        28901822, 29098424, 30081480, 29491651, 29163967, 29884865, 29819336
};

unsigned int header_N_qdc[14] = {
        12, 0, 1622061, 123456789, 26001, 2345,
        123, 456, 789, 987, 654, 321, 135, 791
};

//This header has the CFD fractional time set to 1234.
unsigned int header_N_Cfd[6] {4, 0, 540717, 123456789, 80897425, 2345};

//Here is all of the expected data for the above header.
static const unsigned int expected_chan = 13;
static const unsigned int expected_size = 1;
static const unsigned int expected_ts_high = 26001;
static const unsigned int expected_ts_low = 123456789;
static const unsigned int expected_cfd_fractional_time = 1234;
static const unsigned int expected_slot = 2;
static const unsigned int expected_energy = 2345;
static const double expected_ts = 111673568120085;
static const double expected_ts_w_cfd = 223347136240170.075317;
static const vector<unsigned int> expected_trace = {
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
static const vector<unsigned int> expected_qdc = {123, 456, 789, 987, 654,
                                                  321, 135, 791};

//Test the error handling in the class
TEST_FIXTURE(XiaListModeDataDecoder, TestBufferLengthChecks) {
    //Check for a length_error when the buffer length is zero
    unsigned int buffer_len_zero[6] = {0, 0};
    CHECK_THROW(DecodeBuffer(&buffer_len_zero[0], mask), length_error);
    //Check for an empty vector when the buffer length is 2 (empty module)
    unsigned int buffer_len_two[6] = {2, 0};
    unsigned int expected_size = 0;
    CHECK_EQUAL(expected_size, DecodeBuffer(&buffer_len_two[0], mask).size());
}
///Test if we can decode a simple 4 word header that includes the Pixie
/// Module Data Header.
TEST_FIXTURE(XiaListModeDataDecoder, TestHeaderDecoding) {
    //Check for length_error when the header has an impossible size.
    ///A header with a header length 20 instead of the true header length 4
    unsigned int header_w_bad_len[6] = {4, 0, 3887149, 123456789, 26001, 2345};
    CHECK_THROW(DecodeBuffer(&header_w_bad_len[0], mask), length_error);

    //Check that we can decode a simple 4-word header.
    vector<XiaData*> result = DecodeBuffer(&header[0], mask);
    XiaData result_data = *result.front();

    CHECK_EQUAL(expected_size, result.size());
    CHECK_EQUAL(expected_slot, result_data.GetSlotNumber());
    CHECK_EQUAL(expected_chan, result_data.GetChannelNumber());
    CHECK_EQUAL(expected_energy, result_data.GetEnergy());
    CHECK_EQUAL(expected_ts_high, result_data.GetEventTimeHigh());
    CHECK_EQUAL(expected_ts_low, result_data.GetEventTimeLow());
    CHECK_EQUAL(expected_ts, result_data.GetTime());
}

//Test if we can decode a trace properly
TEST_FIXTURE(XiaListModeDataDecoder, TestTraceDecoding) {
    unsigned int badlen[6] = {
            59, 0, 7749677, 123456789, 26001, 8128809};
    //Check that we throw length_error when the event length doesn't match.
    CHECK_THROW(DecodeBuffer(&badlen[0], mask), length_error);

    XiaData result = *DecodeBuffer(&header_N_trace[0], mask).front();
    CHECK_ARRAY_EQUAL(expected_trace, result.GetTrace(), expected_trace.size());
}

//Test if we can decode the qdc properly
TEST_FIXTURE(XiaListModeDataDecoder, TestQdcDecoding) {
    XiaData result = *DecodeBuffer(&header_N_qdc[0], mask).front();
    CHECK_ARRAY_EQUAL(expected_qdc, result.GetQdc(), expected_qdc.size());
}

//Test that we can get the right timestamp if we involve the CFD.
TEST_FIXTURE(XiaListModeDataDecoder, TestCfdTimeCalculation) {
    XiaData result = *DecodeBuffer(&header_N_Cfd[0], mask).front();
    CHECK_EQUAL(expected_cfd_fractional_time, result.GetCfdFractionalTime());
    CHECK_CLOSE(expected_ts_w_cfd, result.GetTime(), 1e-5);
}


int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}