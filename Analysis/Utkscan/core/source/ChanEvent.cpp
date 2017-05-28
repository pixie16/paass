/** \file ChanEvent.cpp
 * \brief A Class to define what a channel event is
 */
#include <limits>

#include "ChanEvent.hpp"
#include "DetectorLibrary.hpp"

bool CompareCorrectedTime(const ChanEvent *a, const ChanEvent *b) {
    return (a->GetWalkCorrectedTime() < b->GetWalkCorrectedTime());
}

bool CompareTime(const ChanEvent *a, const ChanEvent *b) {
    return (a->GetTime() < b->GetTime());
}

const Identifier &ChanEvent::GetChanID() const {
    return DetectorLibrary::get()->at(GetModuleNumber(), GetChannelNumber());
}

int ChanEvent::GetID() const {
    return (int) DetectorLibrary::get()->GetIndex(GetModuleNumber(), GetChannelNumber());
}
