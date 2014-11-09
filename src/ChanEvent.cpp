/** \file ChanEvent.cpp
 * \brief A Class to define what a channel event is
 */

#include "ChanEvent.hpp"

bool CompareCorrectedTime(const ChanEvent *a, const ChanEvent *b) {
    return (a->GetCorrectedTime() < b->GetCorrectedTime());
}

bool CompareTime(const ChanEvent *a, const ChanEvent *b){
    return (a->GetTime() < b->GetTime());
}

void ChanEvent::ZeroNums() {
    energy        = -1;
    calEnergy     = -1;
    time          = -1;
    calTime       = -1;
    correctedTime = -1;
    highResTime   = -1;

    trigTime    = pixie::U_DELIMITER;
    eventTimeLo = pixie::U_DELIMITER;
    eventTimeHi = pixie::U_DELIMITER;
    runTime0    = pixie::U_DELIMITER;
    runTime1    = pixie::U_DELIMITER;
    runTime2    = pixie::U_DELIMITER;
    chanNum     = -1;
    modNum      = -1;
    for (int i=0; i < numQdcs; i++)
        qdcValue[i] = pixie::U_DELIMITER;
}

unsigned long ChanEvent::GetQdcValue(int i) const {
    if (i < 0 || i >= numQdcs)
        return pixie::U_DELIMITER;
    return qdcValue[i];
}

const Identifier& ChanEvent::GetChanID() const {
    return DetectorLibrary::get()->at(modNum, chanNum);
}

int ChanEvent::GetID() const {
    return DetectorLibrary::get()->GetIndex(modNum, chanNum);
}

//! [Zero Channel]
void ChanEvent::ZeroVar() {
    ZeroNums();
    trace.clear();
}
//! [Zero Channel]
