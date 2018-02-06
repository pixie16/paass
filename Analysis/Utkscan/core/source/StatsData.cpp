/** \file StatsData.cpp
 *  \brief Stores statistics data from the data stream in its original format
 */

#include <iostream>
#include <cmath>
#include <cstring>
#include <sstream>

#include "pixie16app_defs.h"
#include "Globals.hpp"
#include "StatsData.hpp"
#include "Messenger.hpp"

using std::cout;
using std::endl;
using pixie::word_t;

StatsData stats; //!< Instance of the class (not sure why -SVP)

StatsData::StatsData() {
  std::stringstream ss;
  ss << "Allocating " << sizeof(oldData) + sizeof(data)
     << " bytes for statistics data";
  Messenger m;
  m.detail(ss.str());

  bzero(oldData, sizeof(oldData));
  bzero(data, sizeof(data));

  firstTime = NAN;
}

void StatsData::DoStatisticsBlock(word_t *buf, int vsn) {
  if (memcmp(data[vsn], buf, sizeof(word_t)*statSize) != 0) {
    memcpy(oldData[vsn], data[vsn], sizeof(word_t)*statSize);
    memcpy(data[vsn], buf, sizeof(word_t)*statSize);
    if (firstTime == NAN)
	firstTime = GetRealTime();
    if (vsn == 0) {
#ifdef VERBOSE
	cout << "Got stats block at time " << GetRealTime() << endl;
#endif
    }
  }
}

double StatsData::GetCurrTime(unsigned int id) const {
  // from Pixie15DSP_r15428.var
  const size_t offset  = 0x4a340;
  const size_t ltPosHi = 0x4a37f - offset;
  const size_t ltPosLo = 0x4a38f - offset;

  int mod = id / NUMBER_OF_CHANNELS;
  int ch  = id % NUMBER_OF_CHANNELS;

  double d;

  d = data[mod][ltPosHi + ch] * pow(2.0, 32);
  d += data[mod][ltPosLo + ch];

  d *= 16.0 * 1.0e-6 / SYSTEM_CLOCK_MHZ;

  return d;
}

double StatsData::GetDiffPeaks(unsigned int id) const {
  // from Pixie16DSP_r15428.var
  const size_t offset = 0x4a340;
  const size_t peaksPosHi = 0x4a39f - offset;
  const size_t peaksPosLo = 0x4a3af - offset;

  int mod = id / NUMBER_OF_CHANNELS;
  int ch  = id % NUMBER_OF_CHANNELS;

  double d;

  d = data[mod][peaksPosHi + ch] - oldData[mod][peaksPosHi + ch];
  d *= pow(2.0, 32);
  d += data[mod][peaksPosLo + ch] - oldData[mod][peaksPosLo + ch];

  return d;
}

double StatsData::GetDiffTime(unsigned int id) const {
  // from Pixie16DSP_r15428.var
  const size_t offset = 0x4a340;
  const size_t ltPosHi = 0x4a37f - offset;
  const size_t ltPosLo = 0x4a38f - offset;

  int mod = id / NUMBER_OF_CHANNELS;
  int ch  = id % NUMBER_OF_CHANNELS;

  double d;

  d = data[mod][ltPosHi + ch] - oldData[mod][ltPosHi + ch];
  d *= pow(2.0, 32);
  d += data[mod][ltPosLo + ch] - oldData[mod][ltPosLo + ch];

  d *= 16.0 * 1.0e-6 / SYSTEM_CLOCK_MHZ;

  return d;
}

double StatsData::GetRealTime(unsigned int mod) const {
    // from Pixie16DSP_r15428.var
    const size_t offset  = 0x4a340;
    const size_t rtPosHi = 0x4a340 - offset;
    const size_t rtPosLo = 0x4a341 - offset;

    double d = data[mod][rtPosHi] * pow(2.0, 32) +  data[mod][rtPosLo];
    d *= Globals::get()->clockInSeconds();

    return d;
}
