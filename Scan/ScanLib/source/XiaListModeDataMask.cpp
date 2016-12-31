/// @file XiaListModeDataMask.cpp
/// @brief Class that provides the data masks for XIA list mode data
/// @author S. V. Paulauskas
/// @date December 29, 2016
#include <sstream>

#include "XiaListModeDataMask.hpp"

using namespace std;

FIRMWARE XiaListModeDataMask::ConvertStringToFirmware(const std::string &type) {
    FIRMWARE firmware = UNKNOWN;
    stringstream msg;
    if (type == "R29432")
        firmware = R29432;
    else if (type == "R30474")
        firmware = R30474;
    else if (type == "R30980")
        firmware = R30980;
    else if (type == "R30981")
        firmware = R30981;
    else if (type == "R34688")
        firmware = R34688;
    else {
        msg << "XiaListModeDataMask::CovnertStringToFirmware : "
            << "Unrecognized firmware option - " << type << endl;
        throw invalid_argument(msg.str());
    }
    return firmware;
}

///The CFD Fractional Time always starts on bit 16 of Word 2. The range of
/// this value changes.
pair<unsigned int, unsigned int>
XiaListModeDataMask::GetCfdFractionalTimeMask() const {
    if (firmware_ == UNKNOWN || frequency_ == 0)
        throw invalid_argument(BadMaskErrorMessage
                                       ("GetCfdFractionalTimeMask"));
    unsigned int mask = 0;
    if (frequency_ == 100) {
        switch (firmware_) {
            case R29432:
                mask = 0xFFFF0000;
                break;
            case R30474:
            case R30980:
            case R30981:
            case R34688:
                mask = 0x7FFF0000;
                break;
            case UNKNOWN:
                break;
        }
    } else if (frequency_ == 250) {
        switch (firmware_) {
            case R29432:
                mask = 0x7FFF0000;
                break;
            case R30474:
            case R30980:
            case R30981:
            case R34688:
                mask = 0x3FFF0000;
                break;
            case UNKNOWN:
                break;
        }
    } else if (frequency_ == 500) {
        switch (firmware_) {
            case R29432:
            case R30474:
            case R30980:
            case R30981:
            case R34688:
                mask = 0x1FFF0000;
                break;
            case UNKNOWN:
                break;
        }
    }
    return make_pair(mask, 16);
}

pair<unsigned int, unsigned int>
XiaListModeDataMask::GetCfdForcedTriggerBitMask() const {
    if (firmware_ == UNKNOWN || frequency_ == 0)
        throw invalid_argument(BadMaskErrorMessage
                                       ("GetCfdFractionalTimeMask"));
    unsigned int mask = 0;
    unsigned int bit = 0;
    if (frequency_ == 100) {
        switch (firmware_) {
            case R29432:
                mask = 0;
                bit = 0;
                break;
            case R30474:
            case R30980:
            case R30981:
            case R34688:
                mask = 0x80000000;
                bit = 31;
                break;
            case UNKNOWN:
                break;
        }
    } else if (frequency_ == 250) {
        switch (firmware_) {
            case R30474:
            case R30980:
            case R30981:
            case R34688:
                mask = 0x80000000;
                bit = 31;
                break;
            case R29432:
            case UNKNOWN:
                break;
        }
    }
    return make_pair(mask, bit);
}

pair<unsigned int, unsigned int>
XiaListModeDataMask::GetCfdTriggerSourceMask() const {
    if (firmware_ == UNKNOWN || frequency_ == 0)
        throw invalid_argument(BadMaskErrorMessage
                                       ("GetCfdFractionalTimeMask"));
    unsigned int mask = 0;
    unsigned int bit = 0;
    if (frequency_ == 250) {
        switch (firmware_) {
            case R29432:
                mask = 0x80000000;
                bit = 31;
                break;
            case R30474:
            case R30980:
            case R30981:
            case R34688:
                mask = 0x40000000;
                bit = 30;
                break;
            case UNKNOWN:
                break;
        }
    } else if (frequency_ == 500) {
        switch (firmware_) {
            case R29432:
            case R30474:
            case R30980:
            case R30981:
            case R34688:
                mask = 0xE0000000;
                bit = 29;
                break;
            case UNKNOWN:
                break;
        }
    }
    return make_pair(mask, bit);
}

/// The energy always starts out on Bit 0 of Word 3 so we do not need to
/// define a variable for the bit.
pair<unsigned int, unsigned int> XiaListModeDataMask::GetEventEnergyMask()
const {
    if (firmware_ == UNKNOWN || frequency_ == 0)
        throw invalid_argument(BadMaskErrorMessage
                                       ("GetCfdFractionalTimeMask"));
    unsigned int mask = 0;
    switch (firmware_) {
        case R29432:
        case R30474:
        case R30980:
        case R30981:
            mask = 0x00007FFF;
            break;
        case R34688:
            mask = 0x0000FFFF;
            break;
        case UNKNOWN:
            break;
    }
    return make_pair(mask, 0);
}

//The Trace-out-of-range flag moves around on us. For most revisions it on
// bit 15 of Word 3, but for the most recent firmware its been moved to bit 31.
pair<unsigned int, unsigned int>
XiaListModeDataMask::GetTraceOutOfRangeFlagMask() const {
    if (firmware_ == UNKNOWN || frequency_ == 0)
        throw invalid_argument(BadMaskErrorMessage
                                       ("GetCfdFractionalTimeMask"));

    unsigned int mask = 0;
    unsigned int bit = 0;
    switch (firmware_) {
        case R29432:
        case R30474:
        case R30980:
        case R30981:
            mask = 0x00008000;
            bit = 15;
            break;
        case R34688:
            mask = 0x80000000;
            bit = 31;
            break;
        case UNKNOWN:
            break;
    }
    return make_pair(mask, bit);
}

//Trace Length always starts on bit 16 of Word 3.
pair<unsigned int, unsigned int> XiaListModeDataMask::GetTraceLengthMask()
const {
    if (firmware_ == UNKNOWN || frequency_ == 0)
        throw invalid_argument(BadMaskErrorMessage
                                       ("GetCfdFractionalTimeMask"));
    unsigned int mask = 0;
    switch (firmware_) {
        case R29432:
        case R30474:
        case R30980:
        case R30981:
            mask = 0xFFFF0000;
            break;
        case R34688:
            mask = 0x7FFF0000;
            break;
        case UNKNOWN:
            break;
    }
    return make_pair(mask, 16);
}

string XiaListModeDataMask::BadMaskErrorMessage(const std::string &func) const {
    stringstream msg;
    msg << "XiaListModeDataMask::" << func << " : "
        << "Could not obtain a mask for firmware code " << firmware_
        << " and frequency " << frequency_ << ". Check your settings.";
    return msg.str();
}