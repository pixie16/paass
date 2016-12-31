/// @file XiaListModeDataMask.hpp
/// @brief Class that provides the data masks for XIA list mode data
/// @author S. V. Paulauskas
/// @date December 29, 2016
#ifndef PIXIESUITE_XIALISTMODEDATAMASK_HPP
#define PIXIESUITE_XIALISTMODEDATAMASK_HPP

#include <string>
#include <utility>

///An enum for the different firmware revisions for the Pixie-16 modules.
/// * R29432 is valid from 02/15/2014 and 07/28/2014
/// * R30474, R30980, R30981 is valid from 07/28/2014 and 03/08/2016
/// * R29432 is valid from 03/08/2016
/// * UNKNOWN is used for unspecified firmware revisions.
///These dates do not imply that the particular data set being analyzed was
/// taken with the expected firmware. These dates are meant only to help
/// guide the user if they do not know the particular firmware that was used
/// to obtain their data set.
enum FIRMWARE {
    R29432, R30474, R30980, R30981, R34688, UNKNOWN
};

///A class that provides the necessary data masks and bit shifts to decode the
/// XIA Pixie-16 List Mode Data headers. To decode the data we apply the mask
/// to the 32-bit header words, then shift the result by the specified amount
/// to remove the lease significant bits. We do not include a method for the
/// Event Time Low header word (Word 1) since this value takes the entire
/// 32-bit word. The values of the bit shifts are taken from documentation
/// provided by XIA LLC.
class XiaListModeDataMask {
public:
    ///Default constructor
    XiaListModeDataMask() {
        frequency_ = 0;
        firmware_ = UNKNOWN;
    }

    ///Constructor accepting a string with the firmware type and the frequency
    ///@param[in] firmware : The value we want to set for the firmware
    ///@param[in] freq : The value in MS/s or MHz that we want to assign to the
    /// frequency.
    XiaListModeDataMask(const FIRMWARE &firmware, const unsigned int &freq) {
        firmware_ = firmware;
        frequency_ = freq;
    }

    ///Default Destructor
    ~XiaListModeDataMask() {};

    ///Getter for the Mask and Shift of the Channel Number.
    ///@return The pair of the mask and bit shift to use to decode the data.
    std::pair<unsigned int, unsigned int> GetChannelNumberMask() const {
        return std::make_pair(0x0000000F, 0);
    };

    ///Getter for the Mask and Shift of the Slot Id.
    ///@return The pair of the mask and bit shift to use to decode the data.
    std::pair<unsigned int, unsigned int> GetSlotIdMask() const {
        return std::make_pair(0x000000F0, 4);
    };

    ///Getter for the Mask and Shift of the Crate ID.
    ///@return The pair of the mask and bit shift to use to decode the data.
    std::pair<unsigned int, unsigned int> GetCrateIdMask() const {
        return std::make_pair(0x00000F00, 8);
    };

    ///Getter for the Mask and Shift of the Header Length.
    ///@return The pair of the mask and bit shift to use to decode the data.
    std::pair<unsigned int, unsigned int> GetHeaderLengthMask() const {
        return std::make_pair(0x0001F000, 12);
    };

    ///Getter for the Mask and Shift of the Event Length.
    ///@return The pair of the mask and bit shift to use to decode the data.
    std::pair<unsigned int, unsigned int> GetEventLengthMask() const {
        return std::make_pair(0x1FFE0000, 17);
    };

    ///Getter for the Mask and Shift of the Finish Code.
    ///@return The pair of the mask and bit shift to use to decode the data.
    std::pair<unsigned int, unsigned int> GetFinishCodeMask() const {
        return std::make_pair(0x80000000, 31);
    };

    ///Getter for the Mask and Shift of the Event Time High.
    ///@return The pair of the mask and bit shift to use to decode the data.
    std::pair<unsigned int, unsigned int> GetEventTimeHighMask() const {
        return std::make_pair(0x0000FFFF, 0);
    };

    ///Getter for the Mask and Shift of the Event Time High.
    ///@return The pair of the mask and bit shift to use to decode the data.
    std::pair<unsigned int, unsigned int> GetCfdFractionalTimeMask() const;

    ///Getter for the Mask and Shift of the Cfd Forced Trigger Bit mask.
    ///@return The pair of the mask and bit shift to use to decode the data.
    std::pair<unsigned int, unsigned int> GetCfdForcedTriggerBitMask() const;

    ///Getter for the Mask and Shift of the CFD Trigger Source Bit.
    ///@return The pair of the mask and bit shift to use to decode the data.
    std::pair<unsigned int, unsigned int> GetCfdTriggerSourceMask() const;

    ///Getter for the Mask and Shift of the Energy.
    ///@return The pair of the mask and bit shift to use to decode the data.
    std::pair<unsigned int, unsigned int> GetEventEnergyMask() const;

    ///Getter for the Mask and Shift of the Trace-out-of-range Flag.
    ///@return The pair of the mask and bit shift to use to decode the data.
    std::pair<unsigned int, unsigned int> GetTraceOutOfRangeFlagMask() const;

    ///Getter for the Mask and Shift of the Trace Length.
    ///@return The pair of the mask and bit shift to use to decode the data.
    std::pair<unsigned int, unsigned int> GetTraceLengthMask() const;

    ///Getter for the value of the FIRMWARE so that we can test that things
    /// are working as expected.
    ///@return The current value of the internal firmware_ variable.
    FIRMWARE GetFirmware() const { return firmware_; }

    ///Getter for the value of the frequency that we're using.
    ///@return The current value of the internal frequency_ variable
    unsigned int GetFrequency() const { return  frequency_; }

    ///Sets the firmware version
    ///@param[in] firmware : The firmware type that we would like to set.
    void SetFirmware(const FIRMWARE &firmware) {
        firmware_ = firmware;
    }

    ///Sets the firmware version
    ///@param[in] type : The string that we are going to convert to the
    /// more useful FIRMWARE enum.
    void SetFirmware(const std::string &type) {
        firmware_ = ConvertStringToFirmware(type);
    };

    ///Sets the frequency of the module that we are working with.
    ///@param[in] freq : The frequency of the module in MS/s or MHz that we
    /// are working with.
    void SetFrequency(const unsigned int &freq) { frequency_ = freq; }


private:
    ///The firmware version that we are using.
    FIRMWARE firmware_;
    ///The frequency of the module that we want to decode.
    unsigned int frequency_;

    std::string BadMaskErrorMessage(const std::string &func) const;

    FIRMWARE ConvertStringToFirmware(const std::string &type);
};

#endif //PIXIESUITE_XIALISTMODEDATAMASK_HPP
