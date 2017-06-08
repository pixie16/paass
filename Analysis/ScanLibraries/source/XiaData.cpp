///@file XiaData.cpp
///@brief A class that holds information from the XIA LLC. Pixie-16 List
/// Mode Data
///@authors C. R. Thornsberry and S. V. Paulauskas
#include "XiaData.hpp"

///Clears all of the variables. The vectors are all cleared using the clear()
/// method. This method is called when the class is first initalizied so that
/// it has some default values for the software to use in the event that they
/// are needed.
void XiaData::Clear() {
    cfdForceTrig_ = cfdTrigSource_ = isPileup_ = isSaturated_ = false;
    isVirtualChannel_ = false;

    energy_ = baseline_ = 0.0;

    chanNum_ = crateNum_ = slotNum_ = cfdTime_ = 0;
    eventTimeHigh_ = eventTimeLow_ = externalTimeLow_ = externalTimeHigh_ = 0;

    eSums_.clear();
    qdc_.clear();
    trace_.clear();
}