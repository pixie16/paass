/// @file XiaListModeDataDecoder.hpp
/// @brief Class that handles decoding list mode data from XIA Pixie-16
/// modules.
/// @author S. V. Paulauskas
/// @date December 23, 2016
#include <iostream>
#include <sstream>

#include <cmath>

#include "XiaListModeDataDecoder.hpp"

using namespace std;

enum HEADER_CODES {
    STATS_BLOCK = 1, HEADER = 4, HEADER_W_TS = 6, HEADER_W_ESUMS = 8,
    HEADER_W_QDC = 12, HEADER_W_TS_ESUM = 10, HEADER_W_ESUM_QDC = 14,
    HEADER_W_TS_QDC_ESUM = 16
};

vector<XiaData> XiaListModeDataDecoder::DecodeBuffer(unsigned int *buf) {
    unsigned int *bufStart = buf;
    ///@NOTE : These two pieces here are the Pixie Module Data Header. They
    /// tell us the number of words read from the module (bufLen) and the VSN
    /// of the module (module number). Do these technically belong here? They
    /// are not part of decoding the XIA data. In addition, we are passing in
    /// the bufLen, but not using is in ReadSpill.
    ///@TODO : Reevaluate where these two things go.
    unsigned int bufLen = *buf++;
    unsigned int modNum = *buf++;

    //A buffer length of zero is an issue, we'll throw a length error.
    if (bufLen == 0)
        throw length_error("Unpacker::ReadBuffer - The buffer length was "
                                   "sized 0. This is a huge issue.");

    //For empty buffers we just return an empty vector.
    static const unsigned int emptyBufferLength = 2;
    if (bufLen == emptyBufferLength)
        return vector<XiaData>();

    stringstream msg;
    vector<XiaData> events;
    XiaData *lastVirtualChannel = NULL;

    while (buf < bufStart + bufLen) {
        XiaData currentEvt;
        bool hasExternalTimestamp = false;
        bool hasQdc = false;
        bool hasEnergySums = false;

        unsigned int chanNum = (buf[0] & 0x0000000F);
        unsigned int slotNum = (buf[0] & 0x000000F0) >> 4;
        unsigned int crateNum = (buf[0] & 0x00000F00) >> 8;
        unsigned int headerLength = (buf[0] & 0x0001F000) >> 12;
        unsigned int eventLength = (buf[0] & 0x1FFE0000) >> 17;

        currentEvt.virtualChannel = ((buf[0] & 0x20000000) != 0);
        currentEvt.saturatedBit = ((buf[0] & 0x40000000) != 0);
        currentEvt.pileupBit = ((buf[0] & 0x80000000) != 0);

        switch (headerLength) {
            case STATS_BLOCK : // Manual statistics block inserted by poll
                // this is a manual statistics block inserted by the poll program
                /*stats.DoStatisticsBlock(&buf[1], modNum);
                buf += eventLength;
                numEvents = -10;*/
                continue;
            case HEADER :
                break;
            case HEADER_W_TS :
                hasExternalTimestamp = true;
                break;
            case HEADER_W_QDC :
                hasQdc = true;
                break;
            case HEADER_W_ESUMS :
                hasEnergySums = true;
                break;
            case HEADER_W_TS_ESUM :
                hasExternalTimestamp = hasEnergySums = true;
                break;
            case HEADER_W_ESUM_QDC :
                hasEnergySums = hasQdc = true;
                break;
            case HEADER_W_TS_QDC_ESUM :
                hasEnergySums = hasExternalTimestamp = hasQdc = true;
            default:
                msg << "XiaListModeDataDecoder::ReadBuffer : We encountered an "
                        "unrecognized header length ("
                    << headerLength << "). Skipping the remaining buffer."
                    << endl << "ReadBuffer: Unexpected header length: "
                    << headerLength << endl << "ReadBuffer:   Buffer "
                    << modNum << " of length " << bufLen << endl
                    << "ReadBuffer:   CHAN:SLOT:CRATE " << chanNum << ":"
                    << slotNum << ":" << crateNum << endl;
                throw length_error(msg.str());
        }

        unsigned int lowTime = buf[1];
        unsigned int highTime = buf[2] & 0x0000FFFF;
        unsigned int cfdTime = (buf[2] & 0xFFFF0000) >> 16;
        unsigned int energy = buf[3] & 0x0000FFFF;
        unsigned int traceLength = (buf[3] & 0xFFFF0000) >> 16;

        if(hasExternalTimestamp) {
            //Do nothing for now
        }

        if (hasEnergySums) {
            // Skip the onboard partial sums for now
            // trailing, leading, gap, baseline
        }

        if (hasQdc) {
            int offset = headerLength - 8;
            for (int i = 0; i < currentEvt.numQdcs; i++) {
                currentEvt.qdcValue[i] = buf[offset + i];
            }
        }

        currentEvt.chanNum = chanNum;
        currentEvt.modNum = modNum += 100 * crateNum;
        currentEvt.slotNum = slotNum;

        /*if(currentEvt.virtualChannel){
            DetectorLibrary* modChan = DetectorLibrary::get();

            currentEvt.modNum += modChan->GetPhysicalModules();
            if(modChan->at(modNum, chanNum).HasTag("construct_trace")){
                lastVirtualChannel = currentEvt;
            }
        }*/

        ///@TODO Figure out where to put this...
        //channel_counts[modNum][chanNum]++;

        currentEvt.energy = energy;
        if (currentEvt.saturatedBit) { currentEvt.energy = 16383; }

        currentEvt.trigTime = lowTime;
        currentEvt.cfdTime = cfdTime;
        currentEvt.eventTimeHi = highTime;
        currentEvt.eventTimeLo = lowTime;
        currentEvt.time = highTime * pow(2., 32.) + lowTime;

        // One last check
        if (traceLength / 2 + headerLength != eventLength) {
            msg << "ReadBuffer: Bad event length (" << eventLength
                << ") does not correspond with length of header ("
                << headerLength << ") and length of trace (" << traceLength
                << ")";
            throw length_error(msg.str());
        }

        buf += headerLength;
        if (traceLength > 0) {
            // sbuf points to the beginning of trace data
            unsigned short *sbuf = (unsigned short *) buf;

            currentEvt.reserve(traceLength);

            if (lastVirtualChannel != NULL &&
                lastVirtualChannel->adcTrace.empty()) {
                lastVirtualChannel->assign(traceLength, 0);
            }
            // Read the trace data (2-bytes per sample, i.e. 2 samples per word)
            for (unsigned int k = 0; k < traceLength; k++) {
                currentEvt.push_back(sbuf[k]);

                if (lastVirtualChannel != NULL) {
                    lastVirtualChannel->adcTrace[k] += sbuf[k];
                }
            }
            buf += traceLength / 2;
        } // if(traceLength > 0)
        events.push_back(currentEvt);
    }
    return events;
}