#include "ChanEvent.hpp"

/**
 * Sort by increasing raw time
 */
bool CompareTime(const ChanEvent *a, const ChanEvent *b) {
    return (a->GetTime() < b->GetTime());
}

/**
 * Sort by increasing corrected time
 */
bool CompareCorrectedTime(const ChanEvent *a, const ChanEvent *b) {
    return (a->GetCorrectedTime() < b->GetCorrectedTime());
}

/* Initialization of static memeber to the value defined in Globals.hpp */
double ChanEvent::pixieEnergyContraction = Globals::get()->energyContraction();

/**
 * Channel event constructor
 *
 * All numerical values are set to -1
 */
ChanEvent::ChanEvent() {
    ZeroNums();
}

/**
 * Channel event number zeroing
 *
 * This zeroes all the numerical values to -1, leaving internal objects
 * which should have constructors untouched
 */
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

//! Find the identifier in the map for the channel event
const Identifier& ChanEvent::GetChanID() const {
    return DetectorLibrary::get()->at(modNum, chanNum);
}

//! Calculate a channel index
int ChanEvent::GetID() const {
    return DetectorLibrary::get()->GetIndex(modNum, chanNum);
}

/**
 * Channel event zeroing

 * All numerical values are set to -1, and the trace,
 * and traceinfo vectors are cleared and the channel
 * identifier is zeroed using its identifier::zeroid method.
 */
//! [Zero Channel]
void ChanEvent::ZeroVar() {
    ZeroNums();
    trace.clear();
}
//! [Zero Channel]
