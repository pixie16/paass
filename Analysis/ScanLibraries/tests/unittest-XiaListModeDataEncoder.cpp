///@file unittest-XiaListModeDataEncoder.cpp
///@brief Unit tests for the XiaListModeDataDecoder class
///@author S. V. Paulauskas
///@author December 25, 2016
#include <stdexcept>

#include <UnitTest++.h>

#include "HelperEnumerations.hpp"
#include "UnitTestSampleData.hpp"
#include "XiaListModeDataEncoder.hpp"

using namespace std;
using namespace DataProcessing;
using namespace unittest_encoded_data;
using namespace unittest_decoded_data;
using namespace unittest_encoded_data::R30474_250;

TEST_FIXTURE(XiaListModeDataEncoder, TestEncodingThrows
) {
//Check that we throw a range error when we pass an empty XiaData class
CHECK_THROW(EncodeXiaData(XiaData(), R30474, 250), invalid_argument
);

//Check that we catch an invalid_argument when we pass a bogus firmware
CHECK_THROW(EncodeXiaData(XiaData(), UNKNOWN, 250), invalid_argument
);

//Check that we catch an invalid_argument when we pass a bogus frequency
CHECK_THROW(EncodeXiaData(XiaData(), R30474, 2500), invalid_argument
);
}

///Test if we can encode some headers with different information.
///@TODO Add headers for Esums and external TS.
TEST_FIXTURE(XiaListModeDataEncoder, TestSimpleHeaderEncoding
) {
XiaData data;
data.
SetEnergy(energy);
data.
SetSlotNumber(slotId);
data.
SetChannelNumber(channelNumber);
data.
SetCrateNumber(crateId);
data.
SetEventTimeLow(ts_low);
data.
SetEventTimeHigh(ts_high);

//Check that we can handle just a simple 4 word header
CHECK_ARRAY_EQUAL(header_vec, EncodeXiaData(data, R30474, 250),
        header_vec
.

size()

);

//Check that we can handle a header with a trace
data.
SetTrace(unittest_trace_variables::trace);
CHECK_ARRAY_EQUAL(header_vec_w_trc, EncodeXiaData(data, R30474, 250),
        header_vec_w_trc
.

size()

);

//Check that we can handle a QDC
data.
SetQdc(qdc);
data.

SetTrace (vector<unsigned int>());

CHECK_ARRAY_EQUAL(header_vec_w_qdc, EncodeXiaData(data, R30474, 250),
        header_vec_w_qdc
.

size()

);

//Check that we can handle a QDC and a Trace
data.
SetQdc(qdc);
data.
SetTrace(unittest_trace_variables::trace);
CHECK_ARRAY_EQUAL(header_vec_w_qdc_n_trc, EncodeXiaData(data, R30474, 250),
        header_vec_w_qdc_n_trc
.

size()

);
}

int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}