/// @file XiaListModeDataDecoder.hpp
/// @brief Class that handles decoding list mode data from XIA Pixie-16
/// modules.
/// @author S. V. Paulauskas
/// @date December 23, 2016
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <bitset>

#include <stdlib.h>
#include <cmath>

#include "HelperEnumerations.hpp"
#include "XiaListModeDataDecoder.hpp"

using namespace std;
using namespace DataProcessing;

vector<XiaData *> XiaListModeDataDecoder::DecodeBuffer(unsigned int *buf, const XiaListModeDataMask &mask) {

    unsigned int *bufStart = buf;


    ///@NOTE : These two pieces here are the Pixie Module Data Header. They
    /// tell us the number of words read from the module (bufLen) and the VSN
    /// of the module (module number).
    unsigned int bufLen = *buf++;
    unsigned int modNum = *buf++;

    //A buffer length of zero is an issue, we'll throw a length error.
    if (bufLen == 0)
        throw length_error("Unpacker::ReadBuffer - The buffer length was sized 0. This is a huge issue.");

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

        // We check the header length here to set the appropriate flags for
        // processing the rest of the header words. If we encounter a header
        // length that we do not know we will throw an error as this
        // generally indicates an issue with the data file or processing at a
        // higher level.
        switch (headerLength) {
            case STATS_BLOCK : // Manual statistics block inserted by poll
                // this is a manual statistics block inserted by the poll program
                //stats.DoStatisticsBlock(&buf[1], modNum);
                buf += eventLength;
                //numEvents = -10;
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


        if (hasQdc) {
            static const unsigned int numQdcs = 8;
            vector<unsigned int> tmp;
            unsigned int offset = headerLength - numQdcs;
            for (unsigned int i = 0; i < numQdcs; i++) {
                tmp.push_back(buf[offset + i]);
            }
            data->SetQdc(tmp);
        }


        // printf("bufLen %u \n", bufLen);
        // printf("headerLength %u \n", headerLength);

        if (hasExternalTimestamp) {
          /// set least significant 32 bits of 48 bit external time stamp

          // data->SetExternalTimeLow(buf[4]);
          data->SetExternalTimeLow(buf[headerLength-2]);
          /// set most significant 16 bits of 48 bit external time stamp
          // if (buf[headerLength-2]!=buf[4])
          // {
          //   printf("\n");
          //   printf("headerLength-2  = %u \n", headerLength-2);
          //   printf("SetExternalTimeLow %u \n", buf[headerLength-2]);
          //   printf("SetExternalTimeLow %u \n", buf[4]);
          // }

          // data->SetExternalTimeHigh(DecodeExternalTimeHigh(buf[5], *data, mask));
          data->SetExternalTimeHigh(DecodeExternalTimeHigh(buf[headerLength-1], *data, mask));
          /// stores 48 bit external time stamp as an XiaData member -> double externalTimeStamp_
          // if (buf[headerLength-1]!=buf[5])
          // {
          //   printf("\n");
          //   printf("headerLength-1  = %u \n", headerLength-1);
          //   printf("SetExternalTimeHigh %u \n", buf[headerLength-1]);
          //   printf("SetExternalTimeHigh %u \n", buf[5]);
          // }

        //   printf("\n");
        //   printf("header length (1 counting) = %u \n", headerLength);
        //   printf("ETS Word1 (Last Buf Word - 1;ETS_low) = %u \n", buf[headerLength - 2]);
        //   cout << "ETS Word1 Binary= " << std::bitset<32>(buf[headerLength - 2]) << endl
        //        << endl;

        //   if (data->GetExternalTimeLow() > 4000000000) {
        //       cout << "Approching UInt max" << endl
        //            << endl;
        //   }

        //   printf("ETS Word2 (Last Buf Word; CCI_ETS_HIGH)=                    %u \n", buf[headerLength - 1]);
        //   cout << "ETS Word2 Binary= " << std::bitset<32>(buf[headerLength - 1]) << endl<<endl;

        //   printf("Header Word4 (Trace ORF, Trace Len, EVT En)= %u \n", buf[headerLength - 3]);
        //   cout << "Header Word4 Binary= " << std::bitset<32>(buf[headerLength - 3]) << endl;

        //   printf("\n");

          data->SetExternalTimeStamp(CalculateExternalTimeStamp(*data));
          if(headerLength);
          // printf("headerLength %u, ", headerLength);
          // printf("ExternalTimeHigh %u,", data->GetExternalTimeHigh());
          // printf("ExternalTimeLow %u,", data->GetExternalTimeLow());
          // printf("ExternalTimeStamp %llu \n", data->GetExternalTimeStamp());

        }

        if (hasEnergySums) {
            // Skip the onboard partial sums for now
            // trailing, leading, gap, baseline
        }


        ///@TODO Figure out where to put this...
        //channel_counts[modNum][chanNum]++;

        ///@TODO This needs to be revised to take into account the bit
        /// resolution of the modules. I've currently set it to 10 more than maximum
        /// bit resolution of any module (16-bit).
        if (data->IsSaturated())
            data->SetEnergy(65546);

        //We set the time according to the revision and firmware.
        pair<double, double> times = CalculateTimeInSamples(mask, *data);
        data->SetTimeSansCfd(times.first);
        data->SetTime(times.second);

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

std::pair<unsigned int, unsigned int> XiaListModeDataDecoder::DecodeWordZero(const unsigned int &word, XiaData &data,
                                                                             const XiaListModeDataMask &mask) {

    data.SetChannelNumber(word & mask.GetChannelNumberMask().first);
    data.SetSlotNumber((word & mask.GetSlotIdMask().first) >> mask.GetSlotIdMask().second);
    // Crate number in Pixie list-mode data is ignored
    data.SetCrateNumber(0);
    data.SetPileup((word & mask.GetFinishCodeMask().first) != 0);

    //We have to check if we have one of these three firmwares since they
    // have the Trace-Out-of-Range flag in this word.
    switch (mask.GetFirmware()) {
        case R17562:
        case R20466:
        case R27361:
            data.SetSaturation((bool) ((word & mask.GetTraceOutOfRangeFlagMask().first)
                    >> mask.GetTraceOutOfRangeFlagMask().second));
            break;
        default:
            break;
    }

    return make_pair((word & mask.GetHeaderLengthMask().first) >> mask.GetHeaderLengthMask().second,
                     (word & mask.GetEventLengthMask().first) >> mask.GetEventLengthMask().second);
}

unsigned int XiaListModeDataDecoder::DecodeExternalTimeHigh(const unsigned int &word, XiaData &data,
                                                               const XiaListModeDataMask &mask) {
    return (word & mask.GetExternalTimeHighMask().first);
}

void XiaListModeDataDecoder::DecodeWordTwo(const unsigned int &word, XiaData &data, const XiaListModeDataMask &mask) {
      data.SetEventTimeHigh(word & mask.GetEventTimeHighMask().first);
    data.SetCfdFractionalTime((word & mask.GetCfdFractionalTimeMask().first) >> mask.GetCfdFractionalTimeMask().second);
    data.SetCfdForcedTriggerBit((bool) ((word & mask.GetCfdForcedTriggerBitMask().first) >> mask.GetCfdForcedTriggerBitMask().second));
    data.SetCfdTriggerSourceBit((bool) ((word & mask.GetCfdTriggerSourceMask().first) >> mask.GetCfdTriggerSourceMask().second));
}

unsigned int XiaListModeDataDecoder::DecodeWordThree(const unsigned int &word, XiaData &data,
                                                     const XiaListModeDataMask &mask) {
    data.SetEnergy(word & mask.GetEventEnergyMask().first);

    //Reverse the logic that we used in DecodeWordZero, since if we do not
    // have these three firmwares we need to check this word for the
    // Trace-Out-of-Range flag.
    switch (mask.GetFirmware()) {
        case R17562:
        case R20466:
        case R27361:
            break;
        default:
            data.SetSaturation((bool) ((word & mask.GetTraceOutOfRangeFlagMask().first)
                    >> mask.GetTraceOutOfRangeFlagMask().second));
            break;
    }

    return ((word & mask.GetTraceLengthMask().first) >> mask.GetTraceLengthMask().second);
}

void XiaListModeDataDecoder::DecodeTrace(unsigned int *buf, XiaData &data, const unsigned int &traceLength) {
    vector<unsigned int> tmp;
    // sbuf points to the beginning of trace data
    unsigned short *sbuf = (unsigned short *) buf;

    // Read the trace data (2-bytes per sample, i.e. 2 samples per word)
    for (unsigned int k = 0; k < traceLength; k++)
        tmp.push_back((unsigned int &&) sbuf[k]);

    data.SetTrace(tmp);
}

pair<double, double> XiaListModeDataDecoder::CalculateTimeInSamples(const XiaListModeDataMask &mask,
                                                                    const XiaData &data) {
    double filterTime = data.GetEventTimeLow() + data.GetEventTimeHigh() * pow(2., 32);

    double cfdTime = 0, multiplier = 1;
    if (mask.GetFrequency() == 100)
        cfdTime = data.GetCfdFractionalTime() / mask.GetCfdSize();

    if (mask.GetFrequency() == 250) {
        multiplier = 2;
        cfdTime = data.GetCfdFractionalTime() / mask.GetCfdSize() - data.GetCfdTriggerSourceBit();
        }

    if (mask.GetFrequency() == 500) {
        multiplier = 10; // This appears to be wrong based on the documentation in V3.07 of the Pixie Manual (T.T. King Feb,7 2019)
        cfdTime = data.GetCfdFractionalTime() / mask.GetCfdSize() + data.GetCfdTriggerSourceBit() - 1; 
        //From the Pixie Manual v 3.07 it seems that the 500Mhz has 4 interlaced ADCs so its list mode has a 2bit CfdTriggerSource.
        //These methods will need to be updated to account for this, and soon. (T.T. King Feb,7 2019)
    }
    
    //Moved here so we can use the multiplier to adjust the clock tick units. So GetTime() returns the ADC ticks and GetTimeSansCfd() returns Filter Ticks
    //(For 250MHZ) This way GetTime() always returns 4ns clock ticks, and GetTimeSansCfd() returns the normal 8ns ticks  
    if (data.GetCfdFractionalTime() == 0 || data.GetCfdForcedTriggerBit())
        return make_pair(filterTime, filterTime * multiplier);
    
    return make_pair(filterTime, filterTime * multiplier + cfdTime);
}

double XiaListModeDataDecoder::CalculateTimeInNs(const XiaListModeDataMask &mask, const XiaData &data) {
    double conversionToNs = 1. / (mask.GetFrequency() * 1.e6);
    return CalculateTimeInSamples(mask, data).second * conversionToNs;
}

// double XiaListModeDataDecoder::CalculateExternalTimeStamp(const XiaData &data) {
//     double externalTimeStamp = data.GetExternalTimeLow() + data.GetExternalTimeHigh() * pow(2., 32);
//     return externalTimeStamp;
// }

unsigned long long XiaListModeDataDecoder::CalculateExternalTimeStamp(const XiaData &data) {
    unsigned long long tmp_ExternalTimeLow=data.GetExternalTimeLow();
    unsigned long long tmp_ExternalTimeHigh=data.GetExternalTimeHigh();
    unsigned long long externalTimeStamp = tmp_ExternalTimeHigh << 32 | tmp_ExternalTimeLow;
    return externalTimeStamp;
}
