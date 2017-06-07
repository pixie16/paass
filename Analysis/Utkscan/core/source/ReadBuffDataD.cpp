/** \file ReadBuffDataD.cpp
 * \brief retrieve data from raw buffer array ibuf
 */
/*----------------------------------------------------------------------
 * Copyright (c) 2005, XIA LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided
 * that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above
 *     copyright notice, this list of conditions and the
 *     following disclaimer.
 *   * Redistributions in binary form must reproduce the
 *     above copyright notice, this list of conditions and the
 *     following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *   * Neither the name of XIA LLC nor the names of its
 *     contributors may be used to endorse or promote
 *     products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *----------------------------------------------------------------------*/

#include <iostream>
#include <vector>

#include <cmath>

// data related to pixie packet structure
#include "pixie16app_defs.h"

// our event structure
#include "DetectorLibrary.hpp"
#include "Globals.hpp"
#include "ChanEvent.hpp"
#include "Trace.hpp"
#include "StatsData.hpp"

using pixie::word_t;
using pixie::halfword_t;
using std::cout;
using std::endl;
using std::vector;

extern StatsData stats;

/*! \brief extract channel information from raw data

  ReadBuffData extracts channel information from the raw data arrays
  and places it into a structure called evt.  A pointer to each
  of the evt objects is placed in the eventlist vector for later time
  sorting.
  \param [in] buf : the buffer to process
  \param [in] bufLen : the length of the buffer
  \param [in] eventList : the event list to add the extracted buffer to
  \return An unused integer
*/
int ReadBuffDataD(word_t *buf, unsigned long *bufLen,
                   vector<ChanEvent*> &eventList) {
    // multiplier for high bits of 48-bit time
    static const double HIGH_MULT = pow(2., 32.);
    word_t modNum;
    unsigned long numEvents = 0;
    word_t *bufStart = buf;
    /* Determine the number of words in the buffer */
    *bufLen = *buf++;
    /* Read the module number */
    modNum = *buf++;
    ChanEvent *lastVirtualChannel = NULL;
    if(*bufLen > 0) {   // check if the buffer has data
        if(*bufLen == 2) {
            // this is an empty channel
            return 0;
        }
        do {
            ChanEvent *currentEvt = new ChanEvent;
            // decoding event data... see pixie16app.c
            // buf points to the start of channel data
            word_t chanNum      = (buf[0] & 0x0000000F);
            word_t slotNum      = (buf[0] & 0x000000F0) >> 4;
            word_t crateNum     = (buf[0] & 0x00000F00) >> 8;
            word_t headerLength = (buf[0] & 0x0001F000) >> 12;
            word_t eventLength  = (buf[0] & 0x1FFE0000) >> 17;
            currentEvt->virtualChannel = ((buf[0] & 0x20000000) != 0);
            currentEvt->saturatedBit   = ((buf[0] & 0x40000000) != 0);
            currentEvt->pileupBit      = ((buf[0] & 0x80000000) != 0);
            // Rev. D header lengths not clearly defined in pixie16app_defs
            //! magic numbers here for now
            // make some sanity checks
            if(headerLength == stats.headerLength) {
                // this is a manual statistics block inserted by the poll program
                stats.DoStatisticsBlock(&buf[1], modNum);
                buf += eventLength;
                numEvents = readbuff::STATS;
                continue;
            }
            if(headerLength != 4  && headerLength != 8 &&
                headerLength != 12 && headerLength != 16) {
                cout << "  Unexpected header length: " << headerLength << endl;
                cout << "    Buffer " << modNum << " of length " << *bufLen << endl;
                cout << "    CHAN:SLOT:CRATE "
                     << chanNum << ":" << slotNum << ":" << crateNum << endl;
                // advance to next event and continue
                // buf += EventLength;
                // continue;
                // skip the rest of this buffer
                return readbuff::ERROR;
                //return numEvents;
            }
            word_t lowTime     = buf[1];
            word_t highTime    = buf[2] & 0x0000FFFF;
            word_t cfdTime     = (buf[2] & 0xFFFF0000) >> 16;
            word_t energy      = buf[3] & 0x0000FFFF;
            word_t traceLength = (buf[3] & 0xFFFF0000) >> 16;
            if(headerLength == 8 || headerLength == 16) {
                // skip the onboard partial sums for now
                //   trailing, leading, gap, baseline
            }
            if(headerLength >= 12) {
                int offset = headerLength - 8;
                for(int i=0; i < currentEvt->numQdcs; i++) {
                        currentEvt->qdcValue[i] = buf[offset + i];
                }
            }
            // one last sanity check
            if(traceLength / 2 + headerLength != eventLength) {
                cout << "  Bad event length (" << eventLength
                    << ") does not correspond with length of header (" << headerLength
                    << ") and length of trace (" << traceLength << ")" << endl;
                buf += eventLength;
                continue;
            }
            // handle multiple crates
            modNum += 100 * crateNum;
            currentEvt->chanNum = chanNum;
            currentEvt->modNum = modNum;
            if(currentEvt->virtualChannel) {
                DetectorLibrary* modChan = DetectorLibrary::get();
                currentEvt->modNum += modChan->GetPhysicalModules();
                if(modChan->at(modNum, chanNum).HasTag("construct_trace")) {
                        lastVirtualChannel = currentEvt;
                }
            }
            currentEvt->energy = energy;
            //KM 2012-10-24 reinstating removal of saturated
            if(currentEvt->saturatedBit)
                currentEvt->energy = 16383;
            currentEvt->trigTime = lowTime;
            currentEvt->cfdTime  = cfdTime;
            currentEvt->eventTimeHi = highTime;
            currentEvt->eventTimeLo = lowTime;
            currentEvt->time = highTime * HIGH_MULT + lowTime;
            buf += headerLength;
            /* Check if trace data follows the channel header */
            if(traceLength > 0) {
                // sbuf points to the beginning of trace data
                halfword_t *sbuf = (halfword_t *)buf;
                currentEvt->trace.reserve(traceLength);
                if(currentEvt->saturatedBit)
                    currentEvt->trace.SetValue("saturation", 1);
                if(lastVirtualChannel != NULL && lastVirtualChannel->trace.empty()) {
                    lastVirtualChannel->trace.assign(traceLength, 0);
                }
                // Read the trace data (2-bytes per sample, i.e. 2 samples per word)
                for(unsigned int k = 0; k < traceLength; k ++) {
                    currentEvt->trace.push_back(sbuf[k]);
                    if(lastVirtualChannel != NULL) {
                        lastVirtualChannel->trace[k] += sbuf[k];
                    }
                }
                buf += traceLength / 2;
            }
            eventList.push_back(currentEvt);
            numEvents++;
        } while(buf < bufStart + *bufLen);
    } else {  // if buffer has data
        cout << "ERROR BufNData " << *bufLen << endl;
        cout << "ERROR IN ReadBuffData" << endl;
        cout << "LIST UNKNOWN" << endl;
        return readbuff::ERROR;
    }
    return numEvents;
}
