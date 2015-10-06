/** \file ReadBuffData.RevA.cpp
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
#include "Globals.hpp"
#include "RawEvent.hpp"

using pixie::word_t;
using pixie::halfword_t;
using std::cout;
using std::endl;
using std::vector;

/** \brief extract channel information from raw data
 *
 * ReadBuffData extracts channel information from the raw data array and place
 * it into a ChanEvent structure .  A pointer to each of the ChanEvent objects
 * is placed in the eventlist vector for later sorting.
 * \param [in] buf : the buffer to process
 * \param [in] bufLen : the length of the buffer
 * \param [in] eventList : the event list to add the extracted buffer to
 * \return An unused integer
 */
int ReadBuffDataA(word_t *buf, unsigned long *bufLen,
		 vector<ChanEvent*> &eventList) {
  unsigned long bufSkippedWords;
  word_t evtPattern;
  word_t bufNData, modNum, runTask, runStartTime[3], eventTime[2];
  word_t chanTrigTime = 0, eventEnergy = 0, chanLength = 0;

  /* Initialize indicator and counter */
  unsigned long totalSkippedWords = 0;
  unsigned long numEvents = 0;

  static const double HIGH_MULT = pow(2., 32.);

  /* Determine the number of words in the buffer */
  *bufLen = bufNData = buf[totalSkippedWords++];

  /* Read the module number */
  modNum=buf[totalSkippedWords++];

  /* Read Run Task */
  runTask=buf[totalSkippedWords++] & 0x0FFF;

  /* Read Run Start Time */
  runStartTime[0]=buf[totalSkippedWords];
  runStartTime[1]=buf[totalSkippedWords+1];
  runStartTime[2]=buf[totalSkippedWords+2];
  totalSkippedWords += 3;

  if( bufNData > BUFFER_HEAD_LENGTH ) {   /* Check if buffer contains event */
      /* Loop over the buffer */
      bufSkippedWords = 0;
      numEvents=0;
      do {
          /* Read Event Pattern */
          evtPattern=buf[totalSkippedWords];
          /* Read the Event Time */
          eventTime[0]=buf[totalSkippedWords+1];
          eventTime[1]=buf[totalSkippedWords+2];

          totalSkippedWords += EVENT_HEAD_LENGTH;
          bufSkippedWords += EVENT_HEAD_LENGTH;

          if (bufSkippedWords > bufNData) {
	      cout << "Bad buffer " << totalSkippedWords << endl;
              return readbuff::ERROR;
          }

          if( evtPattern != 0 ) {
              for(int ch = 0; ch < NUMBER_OF_CHANNELS; ch++) {
                  /* Check if Channel j has been hit */
                  if( pixie::TstBit(ch, evtPattern) ) {
                      /* Read Channel Length */
                      if( runTask == LIST_MODE_RUN0 || runTask == LIST_MODE_RUN1 ) {
                          chanLength=buf[totalSkippedWords];

                          if( chanLength>10000 ) {
			      cout << "Bad ChanLen " << chanLength << endl;
                              return readbuff::ERROR;
                          }

                          /* Read Channel trigger time */
                          chanTrigTime=buf[totalSkippedWords+1];
                          /* Read Channel event energy */
                          eventEnergy=buf[totalSkippedWords+2];
                          /* Skip the remaining channel header data */
                          totalSkippedWords += CHANNEL_HEAD_LENGTH;
                          bufSkippedWords += CHANNEL_HEAD_LENGTH;
                      } else if ( runTask == LIST_MODE_RUN3 ) {
                          /* Read Channel trigger time */
                          chanTrigTime=buf[totalSkippedWords];
                          /* Read Channel event energy */
                          eventEnergy=buf[totalSkippedWords+1];
                          totalSkippedWords += 2;
                          bufSkippedWords += 2;
                      } else {
			  cout << "Run task read out is invalid " << runTask << endl;
			  return readbuff::ERROR;
		      }
                      ChanEvent *currentEvt = new ChanEvent();

		      currentEvt->chanNum = ch;
                      currentEvt->modNum  = modNum;
		      if ( (eventEnergy & 0xFFFF0000) == 0) {
			  currentEvt->energy  = eventEnergy;
		      } else {
			  // this is some peculiar pixie event, tag with a fixed energy
			  currentEvt->energy  = 64000;
		      }
		      currentEvt->trigTime    = chanTrigTime;
		      currentEvt->eventTimeHi = eventTime[0];
		      currentEvt->eventTimeLo = eventTime[1];
		      currentEvt->runTime0    = runStartTime[0];
		      currentEvt->runTime1    = runStartTime[1];
		      currentEvt->runTime2    = runStartTime[2];

		      // new event time constructed from upper 32 bits of event time and
		      // using the trigger time for the lower 32 bits, this should
		      // be immune to slow filter lengths.
                      currentEvt->time = (double)(eventTime[0] * HIGH_MULT + chanTrigTime);

                      /* Check if trace data follows the channel header */
                      if( chanLength > CHANNEL_HEAD_LENGTH && runTask == LIST_MODE_RUN0 ) {
			  halfword_t *hbuf = (halfword_t *)&buf[totalSkippedWords];
                          // Read the trace data (2-bytes per sample, i.e. 2 samples per word)
                          int numSamples = 2 * (chanLength - CHANNEL_HEAD_LENGTH);
                          for(int k = 0; k < numSamples; k ++) {
			      currentEvt->trace.push_back(hbuf[k]);
                          }

                          totalSkippedWords += chanLength - CHANNEL_HEAD_LENGTH;
                          bufSkippedWords   += chanLength - CHANNEL_HEAD_LENGTH;
                      }
                      eventList.push_back(currentEvt);
                  } // if channel hit
              } // check channel hitpattern
          } else { // if non-0 hitpattern
	      //patch 07/04/2006 for the events with pattern=0
              ChanEvent *currentEvt = new ChanEvent;

	      currentEvt->modNum      = modNum;
	      currentEvt->chanNum     = -1; // used to be set to 0, with id set to -1
	      currentEvt->energy      = 65535;
	      currentEvt->trigTime    = chanTrigTime;
	      currentEvt->eventTimeHi = eventTime[0];
	      currentEvt->eventTimeLo = eventTime[1];
	      currentEvt->runTime0    = runStartTime[0];
	      currentEvt->runTime1    = runStartTime[1];
	      currentEvt->runTime2    = runStartTime[2];
	      currentEvt->time        = 0;

              eventList.push_back(currentEvt);
          }
          numEvents++;
      } while( bufSkippedWords < (bufNData - BUFFER_HEAD_LENGTH) );
  } else {// if buffer has data
      cout << "ERROR BufNData " << bufNData << endl;
      cout << "ERROR IN ReadBuffData" << endl;
      cout << "LIST UNKNOWN" << endl;
      return readbuff::ERROR;
  }

  return numEvents;
}
