///@file unittest-XiaListModeDataDecoder.cpp
///@brief Unit tests for the XiaListModeDataDecoder class
///@author S. V. Paulauskas
///@author December 25, 2016
#include <stdexcept>

#include <UnitTest++.h>

#include "HelperEnumerations.hpp"
#include "UnitTestSampleData.hpp"
#include "XiaListModeDataDecoder.hpp"

using namespace std;
using namespace DataProcessing;
using namespace unittest_encoded_data;
using namespace unittest_decoded_data;

///@TODO These need to be expanded so that we cover all of the nine different
/// firmware and frequency combinations.
static const XiaListModeDataMask mask(R30474, 250);
using namespace unittest_encoded_data::R30474_250;

//Test the error handling in the class
TEST_FIXTURE(XiaListModeDataDecoder, TestBufferLengthChecks) {
    //Check that we throw a length error when the buffer length is zero
    CHECK_THROW(DecodeBuffer(&empty_buffer[0], mask), length_error);
    //Check that we return an empty vector when we have an empty module
    CHECK_EQUAL(empty_buffer[1],
                DecodeBuffer(&empty_module_buffer[0], mask).size());
}
///Test if we can decode a simple 4 word header that includes the Pixie
/// Module Data Header.
TEST_FIXTURE(XiaListModeDataDecoder, TestHeaderDecoding) {
    //Check for length_error when the header has an impossible size.
    CHECK_THROW(DecodeBuffer(&header_w_bad_headerlen[0], mask), length_error);

    //Check that we can decode a simple 4-word header.
    XiaData result_data = *(DecodeBuffer(&header[0], mask).front());

    CHECK_EQUAL(slotId, result_data.GetSlotNumber());
    CHECK_EQUAL(channelNumber, result_data.GetChannelNumber());
    CHECK_EQUAL(energy, result_data.GetEnergy());
    CHECK_EQUAL(ts_high, result_data.GetEventTimeHigh());
    CHECK_EQUAL(ts_low, result_data.GetEventTimeLow());
    CHECK_EQUAL(ts, result_data.GetTime());
}

//Test if we can decode a trace properly
TEST_FIXTURE(XiaListModeDataDecoder, TestTraceDecoding) {
    //Check that we throw length_error when the event length doesn't match.
    CHECK_THROW(DecodeBuffer(&header_w_bad_eventlen[0], mask), length_error);

    XiaData result = *(DecodeBuffer(&header_N_trace[0], mask).front());
    CHECK_ARRAY_EQUAL(unittest_trace_variables::trace, result.GetTrace(),
                      unittest_trace_variables::trace.size());
}

//Test if we can decode the qdc properly
TEST_FIXTURE(XiaListModeDataDecoder, TestQdcDecoding) {
    XiaData result = *(DecodeBuffer(&header_N_qdc[0], mask).front());
    CHECK_ARRAY_EQUAL(qdc, result.GetQdc(), qdc.size());
}

//Test that we can get the right timestamp if we involve the CFD.
TEST_FIXTURE(XiaListModeDataDecoder, TestCfdTimeCalculation) {
    XiaData result = *(DecodeBuffer(&header_N_Cfd[0], mask).front());
    CHECK_EQUAL(cfd_fractional_time, result.GetCfdFractionalTime());
    CHECK_CLOSE(ts_w_cfd, result.GetTime(), 1e-5);
}


int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}