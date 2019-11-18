/// @file XiaListModeDataMask.cpp
/// @brief Class that provides the data masks for XIA list mode data
/// @author S. V. Paulauskas
/// @date December 29, 2016
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "XiaListModeDataMask.hpp"

using namespace std;
using namespace DataProcessing;

FIRMWARE XiaListModeDataMask::ConvertStringToFirmware(const std::string &type) {
    FIRMWARE firmware = UNKNOWN;
    unsigned int firmwareNumber = 0;
    stringstream msg;

    //First convert the string into a number
    if (type.find("R") == 0 || type.find("r") == 0) {
        string tmp(type.begin() + 1, type.end());
        firmwareNumber = (unsigned int)atoi(tmp.c_str());
    } else
        firmwareNumber = (unsigned int)atoi(type.c_str());

    if (firmwareNumber >= 17562 && firmwareNumber < 20466)
        firmware = R17562;
    else if (firmwareNumber >= 20466 && firmwareNumber < 27361)
        firmware = R20466;
    else if (firmwareNumber >= 27361 && firmwareNumber < 29432)
        firmware = R27361;
    else if (firmwareNumber >= 29432 && firmwareNumber < 30474)
        firmware = R29432;
    else if (firmwareNumber >= 30474 && firmwareNumber < 30980)
        firmware = R30474;
    else if (firmwareNumber >= 30980 && firmwareNumber < 30981)
        firmware = R30980;
    else if (firmwareNumber >= 30981 && firmwareNumber < 34688)
        firmware = R30981;
    else if (firmwareNumber == 35207)  //14b 500MHz RevF
        firmware = R35207; //TODO we are hard matching this one because we are unsure of its structure. once we have some time we should see if its any different than the 34688->42950 structure (it shouldnt be)
    else if (firmwareNumber >= 34688 && firmwareNumber <= 42950)  //42950 is is the newest 16b:250 firmware, received at the end of August 2019
        firmware = R34688;
    else {
        msg << "XiaListModeDataMask::ConvertStringToFirmware : "
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
        throw invalid_argument(BadMaskErrorMessage("GetCfdFractionalTimeMask"));
    unsigned int mask = 0;
    if (frequency_ == 100) {
        switch (firmware_) {
            case R17562:
            case R29432:
                mask = 0xFFFF0000;
                break;
            case R30474:
            case R30980:
            case R30981:
            case R34688:
                mask = 0x7FFF0000;
                break;
            default:
                break;
        }
    } else if (frequency_ == 250) {
        switch (firmware_) {
            case R20466:
                mask = 0xFFFF0000;
                break;
            case R27361:
            case R29432:
                mask = 0x7FFF0000;
                break;
            case R30474:
            case R30980:
            case R30981:
                mask = 0x3FFF0000;
                break;
            case R34688:
                mask = 0x3FFF0000;
                break;
            default:
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
            case R35207:
                mask = 0x1FFF0000;
                break;
            default:
                break;
        }
    }
    return make_pair(mask, 16);
}

std::pair<unsigned int, unsigned int> XiaListModeDataMask::GetEventLengthMask()
    const {
    if (firmware_ == UNKNOWN)
        throw invalid_argument(BadMaskErrorMessage("GetEventLengthMask"));
    unsigned int mask = 0;
    unsigned int bit = 0;
    switch (firmware_) {
        case R17562:
        case R20466:
        case R27361:
            mask = 0x3FFE0000;
            bit = 17;
            break;
        case R29432:
        case R30474:
        case R30980:
        case R30981:
        case R34688:
            mask = 0x7FFE0000;
            bit = 17;
            break;
        case R35207:
            mask = 0x7FFE0000;
            bit = 17;
            break;
        default:
            break;
    }
    return make_pair(mask, bit);
}

pair<unsigned int, unsigned int>
XiaListModeDataMask::GetCfdForcedTriggerBitMask() const {
    if (firmware_ == UNKNOWN || frequency_ == 0)
        throw invalid_argument(BadMaskErrorMessage("GetCfdForcedTriggerBitMask"));
    unsigned int mask = 0;
    unsigned int bit = 0;
    if (frequency_ == 100) {
        switch (firmware_) {
            case R30474:
            case R30980:
            case R30981:
            case R34688:
                mask = 0x80000000;
                bit = 31;
                break;
            default:
                break;
        }
    } else if (frequency_ == 250) {
        switch (firmware_) {
            case R30474:
            case R30980:
            case R30981:
                mask = 0x80000000;
                bit = 31;
            case R34688:
                mask = 0x80000000;
                bit = 31;
                break;
            default:
                break;
        }
    }
    return make_pair(mask, bit);
}

pair<unsigned int, unsigned int>
XiaListModeDataMask::GetCfdTriggerSourceMask() const {
    if (firmware_ == UNKNOWN || frequency_ == 0)
        throw invalid_argument(BadMaskErrorMessage("GetCfdTriggerSourceMask"));
    unsigned int mask = 0;
    unsigned int bit = 0;
    if (frequency_ == 250) {
        switch (firmware_) {
            case R27361:
            case R29432:
                mask = 0x80000000;
                bit = 31;
                break;
            case R30474:
            case R30980:
            case R30981:
                mask = 0x40000000;
                bit = 30;
            case R34688:
                mask = 0x40000000;
                bit = 30;
                break;
            default:
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
            case R35207:
                mask = 0xE0000000;
                bit = 29;
            default:
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
        throw invalid_argument(BadMaskErrorMessage("GetEventEnergyMask"));
    unsigned int mask = 0;
    switch (firmware_) {
        case R29432:
        case R30474:
        case R30980:
        case R30981:
            mask = 0x00007FFF;
            break;
        case R17562:
        case R20466:
        case R27361:
        case R35207:
        case R34688:
            mask = 0x0000FFFF;
            break;
        default:
            break;
    }
    return make_pair(mask, 0);
}

//The Trace-out-of-range flag moves around on us. For most revisions it on
// bit 15 of Word 3, but for the most recent firmware its been moved to bit 31.
pair<unsigned int, unsigned int>
XiaListModeDataMask::GetTraceOutOfRangeFlagMask() const {
    if (firmware_ == UNKNOWN || frequency_ == 0)
        throw invalid_argument(BadMaskErrorMessage("GetTraceOutOfRangeFlagMask"));

    unsigned int mask = 0;
    unsigned int bit = 0;
    switch (firmware_) {
        case R17562:
        case R20466:
        case R27361:
            mask = 0x40000000;
            bit = 30;
            break;
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
	    case R35207:
            mask = 0x80000000;
            bit = 31;
            break;
        default:
            break;
    }
    return make_pair(mask, bit);
}

//Trace Length always starts on bit 16 of Word 3.
pair<unsigned int, unsigned int> XiaListModeDataMask::GetTraceLengthMask()
    const {
    if (firmware_ == UNKNOWN || frequency_ == 0)
        throw invalid_argument(BadMaskErrorMessage("GetTraceLengthMask"));
    unsigned int mask = 0;
    switch (firmware_) {
        case R17562:
        case R20466:
        case R27361:
        case R29432:
        case R30474:
        case R30980:
        case R30981:
            mask = 0xFFFF0000;
            break;
        case R34688:
            mask = 0x7FFF0000;
            break;
        case R35207:
	        mask = 0x7FFF0000;
            break;
        default:
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

double XiaListModeDataMask::GetCfdSize() const {
    if (firmware_ == UNKNOWN || frequency_ == 0)
        throw invalid_argument(BadMaskErrorMessage("GetCfdSize"));
    if (frequency_ == 500)
        return 8192.;

    double val = 0;
    if (frequency_ == 100) {
        switch (firmware_) {
            case R17562:
            case R29432:
                val = 65536;
                break;
            case R30474:
            case R30980:
            case R30981:
            case R34688:
                val = 32768;
                break;
            default:
                break;
        }
    } else if (frequency_ == 250) {
        switch (firmware_) {
            case R20466:
                val = 65536;
                break;
            case R27361:
            case R29432:
                val = 32768;
                break;
            case R30980:
            case R30981:
                val = 16384;
            case R34688:
            case R30474:
                val = 16384;
                break;
            default:
                break;
        }
    }

    return val;
}
