/// @file XiaListModeDataDecoder.hpp
/// @brief Class that handles decoding list mode data from XIA Pixie-16
/// modules.
/// @author S. V. Paulauskas
/// @date December 23, 2016
#include <iostream>
#include <sstream>

#include <cmath>

#include "HelperEnumerations.hpp"
#include "XiaListModeDataDecoder.hpp"

using namespace std;
using namespace DataProcessing;

vector<XiaData *> XiaListModeDataDecoder::DecodeBuffer(
        unsigned int *buf, const XiaListModeDataMask &mask) {

    unsigned int *bufStart = buf;
    ///@NOTE : These two pieces here are the Pixie Module Data Header. They
    /// tell us the number of words read from the module (bufLen) and the VSN
    /// of the module (module number).
    unsigned int bufLen = *buf++;
    unsigned int modNum = *buf++;

    //A buffer length of zero is an issue, we'll throw a length error.
    if (bufLen == 0)
        throw length_error("Unpacker::ReadBuffer - The buffer length was "
                                   "sized 0. This is a huge issue.");

    //For empty buffers we just return an empty vector.
    static const unsigned int emptyBufferLength = 2;
    if (bufLen == emptyBufferLength)
        return vector<XiaData *>();

    stringstream msg;
    vector<XiaData *> events;
    static unsigned int numSkippedBuffers = 0;

    while (buf < bufStart + bufLen) {
        XiaData *data = new XiaData();
        bool hasExternalTimestamp = false;
        bool hasQdc = false;
        bool hasEnergySums = false;

        pair<unsigned int, unsigned int> lengths =
                DecodeWordZero(buf[0], *data, mask);
        unsigned int headerLength = lengths.first;
        unsigned int eventLength = lengths.second;

        data->SetEventTimeLow(buf[1]);
        DecodeWordTwo(buf[2], *data, mask);
        unsigned int traceLength = DecodeWordThree(buf[3], *data, mask);

        //We check the header length here to set the appropriate flags for
        // processing the rest of the header words. If we encounter a header
        // length that we do not know we will throw an error as this
        // generally indicates an issue with the data file or processing at a
        // higher level.
        switch (headerLength) {
            case STATS_BLOCK : // Manual statistics block inserted by poll
                // this is a manual statistics block inserted by the poll program
                /*stats.DoStatisticsBlock(&buf[1], modNum);
                buf += eventLength;
                numEvents = -10;*/
                continue;
            case HEADER :
                break;
            case HEADER_W_ETS :
                hasExternalTimestamp = true;
                break;
            case HEADER_W_QDC :
                hasQdc = true;
                break;
            case HEADER_W_ESUM :
                hasEnergySums = true;
                break;
            case HEADER_W_ESUM_ETS :
                hasExternalTimestamp = hasEnergySums = true;
                break;
            case HEADER_W_ESUM_QDC :
                hasEnergySums = hasQdc = true;
                break;
            case HEADER_W_ESUM_QDC_ETS :
                hasEnergySums = hasExternalTimestamp = hasQdc = true;
                break;
            case HEADER_W_QDC_ETS :
                hasQdc = hasExternalTimestamp = true;
                break;
            default:
                numSkippedBuffers++;
                cerr << "XiaListModeDataDecoder::ReadBuffer : We encountered "
                        "an unrecognized header length (" << headerLength
                     << "). " << endl
                     << "Skipped " << numSkippedBuffers
                     << " buffers in the file." << endl
                     << "Unexpected header length: " << headerLength << endl
                     << "ReadBuffer:   Buffer " << modNum << " of length "
                     << bufLen << endl
                     << "ReadBuffer:   CRATE:SLOT(MOD):CHAN "
                     << data->GetCrateNumber() << ":"
                     << data->GetSlotNumber() << "(" << modNum << "):"
                     << data->GetChannelNumber() << endl;
                return vector<XiaData *>();
        }

        if (hasExternalTimestamp) {
            //Do nothing for now
        }

        if (hasEnergySums) {
            // Skip the onboard partial sums for now
            // trailing, leading, gap, baseline
        }

        if (hasQdc) {
            static const unsigned int numQdcs = 8;
            vector<unsigned int> tmp;
            unsigned int offset = headerLength - numQdcs;
            for (unsigned int i = 0; i < numQdcs; i++) {
                tmp.push_back(buf[offset + i]);
            }
            data->SetQdc(tmp);
        }

        ///@TODO Figure out where to put this...
        //channel_counts[modNum][chanNum]++;

        ///@TODO This needs to be revised to take into account the bit
        /// resolution of the modules. I've currently set it to the maximum
        /// bit resolution of any module (16-bit).
        if (data->IsSaturated())
            data->SetEnergy(65536);

        //We set the time according to the revision and firmware.
        data->SetTime(CalculateTimeInSamples(mask, *data));

        // One last check to ensure event length matches what we think it
        // should be.
        if (traceLength / 2 + headerLength != eventLength) {
            numSkippedBuffers++;
            cerr << "XiaListModeDataDecoder::ReadBuffer : Event"
                    "length (" << eventLength << ") does not correspond to "
                         "header length (" << headerLength
                 << ") and trace length ("
                 << traceLength / 2 << "). Skipped a total of "
                 << numSkippedBuffers << " buffers in this file." << endl;
            return vector<XiaData *>();
        } else //Advance the buffer past the header and to the trace
            buf += headerLength;

        if (traceLength > 0) {
            DecodeTrace(buf, *data, traceLength);
            buf += traceLength / 2;
        }
        events.push_back(data);
    }// while(buf < bufStart + bufLen)
    return events;
}

std::pair<unsigned int, unsigned int> XiaListModeDataDecoder::DecodeWordZero(
        const unsigned int &word, XiaData &data,
        const XiaListModeDataMask &mask) {

    data.SetChannelNumber(word & mask.GetChannelNumberMask().first);
    data.SetSlotNumber((word & mask.GetSlotIdMask().first)
                               >> mask.GetSlotIdMask().second);
    data.SetCrateNumber((word & mask.GetCrateIdMask().first)
                                >> mask.GetCrateIdMask().second);
    data.SetPileup((word & mask.GetFinishCodeMask().first) != 0);

    return make_pair((word & mask.GetHeaderLengthMask().first)
                             >> mask.GetHeaderLengthMask().second,
                     (word & mask.GetEventLengthMask().first)
                             >> mask.GetEventLengthMask().second);
}

void XiaListModeDataDecoder::DecodeWordTwo(
        const unsigned int &word, XiaData &data,
        const XiaListModeDataMask &mask) {
    data.SetEventTimeHigh(word & mask.GetEventTimeHighMask().first);
    data.SetCfdFractionalTime((word & mask.GetCfdFractionalTimeMask().first)
                                      >> mask.GetCfdFractionalTimeMask().second);
    data.SetCfdForcedTriggerBit(
            (bool) ((word & mask.GetCfdForcedTriggerBitMask().first)
                    >> mask.GetCfdForcedTriggerBitMask().second));
    data.SetCfdTriggerSourceBit(
            (bool) (word & mask.GetCfdTriggerSourceMask().first)
                    >> mask.GetCfdTriggerSourceMask().second);
}

unsigned int XiaListModeDataDecoder::DecodeWordThree(
        const unsigned int &word, XiaData &data,
        const XiaListModeDataMask &mask) {
    data.SetEnergy(word & mask.GetEventEnergyMask().first);
    data.SetSaturation((bool) (word & mask.GetTraceOutOfRangeFlagMask().first));

    return ((word & mask.GetTraceLengthMask().first)
            >> mask.GetTraceLengthMask().second);
}

void XiaListModeDataDecoder::DecodeTrace(unsigned int *buf, XiaData &data,
                                         const unsigned int &traceLength) {
    vector<unsigned int> tmp;
    // sbuf points to the beginning of trace data
    unsigned short *sbuf = (unsigned short *) buf;

    // Read the trace data (2-bytes per sample, i.e. 2 samples per word)
    for (unsigned int k = 0; k < traceLength; k++)
        tmp.push_back((unsigned int &&) sbuf[k]);

    data.SetTrace(tmp);
}

double XiaListModeDataDecoder::CalculateTimeInSamples(
        const XiaListModeDataMask &mask, const XiaData &data) {
    double filterTime =
            data.GetEventTimeLow() + data.GetEventTimeHigh() * pow(2., 32);

    if (data.GetCfdFractionalTime() == 0 || data.GetCfdForcedTriggerBit())
        return filterTime;

    double cfdTime = 0, multiplier = 1;
    if (mask.GetFrequency() == 100)
        cfdTime = data.GetCfdFractionalTime() / mask.GetCfdSize();

    if (mask.GetFrequency() == 250) {
        multiplier = 2;
        cfdTime = data.GetCfdFractionalTime() / mask.GetCfdSize() -
                  data.GetCfdTriggerSourceBit();
    }

    if (mask.GetFrequency() == 500) {
        multiplier = 10;
        cfdTime = data.GetCfdFractionalTime() / mask.GetCfdSize() +
                  data.GetCfdTriggerSourceBit() - 1;
    }

    return filterTime * multiplier + cfdTime;
}

double XiaListModeDataDecoder::CalculateTimeInNs(
        const XiaListModeDataMask &mask, const XiaData &data) {
    double conversionToNs = 1. / (mask.GetFrequency() * 1.e6);
    return CalculateTimeInSamples(mask, data) * conversionToNs;
}