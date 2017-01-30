///@file XiaData.cpp
///@brief A class that holds information from the XIA LLC. Pixie-16 List
/// Mode Data
///@authors C. R. Thornsberry and S. V. Paulauskas
#ifndef XIADATA_HPP
#define XIADATA_HPP

#include <vector>

/*! \brief A pixie16 channel event
 *
 * All data is grouped together into channels.  For each pixie16 channel that
 * fires the energy, time (both trigger time and event time), and trace (if
 * applicable) are obtained.  Additional information includes the channels
 * identifier, calibrated energies, trace analysis information.
 * Note that this currently stores raw values internally through pixie word types
 *   but returns data values through native C types. This is potentially non-portable.
 */
class XiaData {
public:
    /// Default constructor.
    XiaData() { Clear(); }

    ///Default Destructor.
    ~XiaData() {};

    ///@brief Equality operator that compares checks if we have the same
    /// channel (i.e. the ID and Time are identical)
    ///@param[in] rhs : The right hand side of the comparison
    ///@return True if the two XiaData classes are equal.
    bool operator==(const XiaData &rhs) const {
        return GetId() == rhs.GetId() && GetTime() == rhs.GetTime();
    }

    ///@brief The conjugate of the equality operator
    ///@param[in] rhs : The right hand side for the comparison
    ///@return True if the two are not equal.
    bool operator!=(const XiaData &rhs) const {
        return !operator==(rhs);
    }

    ///@brief The less than operator that compares if the time of the current
    /// class is less than the time of the comparison class.
    ///@param[in] rhs : The right hand side for the comparison
    ///@return True if this instance arrived earlier than the right hand side.
    bool operator<(const XiaData &rhs) const {
        return GetTime() < rhs.GetTime();
    }

    ///@brief The conjugate of the less than operator
    ///@param[in] rhs : The right hand side for the comparison
    ///@return True if the right hand side arrived ealier than the left hand
    /// side.
    bool operator>(const XiaData &rhs) const {
        return !operator<(rhs);
    }

    ///@brief A method that will compare the times of two XiaData classes
    /// this method can be used in conjunction with sorting methods
    ///@param[in] lhs : A pointer to the left hand side of the comparison
    ///@param[in] rhs : A pointer to the right hand side of the comparison
    ///@return True if the time of arrival for right hand side is later than
    /// that of the left hand side.
    static bool CompareTime(const XiaData *lhs, const XiaData *rhs) {
        return lhs->GetTime() < rhs->GetTime();
    }

    ///@brief A method that will compare the unique ID of two XiaData classes
    ///@param[in] lhs : A pointer to the left hand side of the comparison
    ///@param[in] rhs : A pointer to the right hand side of the comparison
    ///@return Return true if left hand side has a lower ID than the right
    /// hand side.
    static bool CompareId(const XiaData *lhs, const XiaData *rhs) {
        return (lhs->GetId() < rhs->GetId());
    }

    ///@return The status of the CFD Forced Trigger Bit
    bool GetCfdForcedTriggerBit() const { return cfdForceTrig_; }

    ///@return The status of the CFD Trigger bit.
    bool GetCfdTriggerSourceBit() const { return cfdTrigSource_; }

    ///@return True if we had a pileup detected on the module
    bool IsPileup() const { return isPileup_; }

    ///@return True if the trace was flagged as a pileup
    bool IsSaturated() const { return isSaturated_; }

    ///@return True if this channel was generated on the module
    bool IsVirtualChannel() const { return isVirtualChannel_; }

    ///@return The baseline as it was calculated on the module
    double GetBaseline() const { return baseline_; }

    ///@return The energy that was calculated on the module
    double GetEnergy() const { return energy_; }

    ///@brief Method that will return the time for the channel. The actual
    /// time is a 48-bit number. We multiply 2^32 by the eventTimeHigh_ so
    /// that we account for the missing upper 16 bits of the number. The
    /// cfdTime_ contains all of the fractional time information, and so we
    /// divide by 2^16 here.
    ///@TODO Verify that this method works properly for all of the different
    /// module types and firmwares. It doesn't and this value simply needs to
    /// be set explicitly by the Decoder
    ///@return The time for the channel.
    double GetTime() const { return time_; }

    ///@return The CFD fractional time in clockticks
    unsigned int GetCfdFractionalTime() const { return cfdTime_; }

    ///@return The Channel number that recorded these data
    unsigned int GetChannelNumber() const { return chanNum_; }

    ///@return The crate number that had the module
    unsigned int GetCrateNumber() const { return crateNum_; }

    ///@return The upper 16 bits of the event time
    unsigned int GetEventTimeHigh() const { return eventTimeHigh_; }

    ///@return The lower 32 bits of the event time
    unsigned int GetEventTimeLow() const { return eventTimeLow_; }

    ///@return The upper 16 bits of the external time stamp provided to the
    /// module via the front panel
    unsigned int GetExternalTimeHigh() const { return externalTimeHigh_; }

    ///@return The lower 32 bits of the external time stamp provided to the
    /// module via the front panel
    unsigned int GetExternalTimeLow() const { return externalTimeLow_; }

    ///@return The unique ID of the channel.
    ///We can have a maximum of 208 channels in a crate, the first module
    /// (#0) is always in the second slot of the crate, and we always have 16
    /// channels
    unsigned int GetId() const {
        return crateNum_ * 208 + GetModuleNumber() * 16 + chanNum_;
    }

    ///@return the module number
    unsigned int GetModuleNumber() const {
        return slotNum_ - 2;
    }

    ///@return The slot that the module was in
    unsigned int GetSlotNumber() const { return slotNum_; }

    ///@return The energy sums recorded on the module
    std::vector<unsigned int> GetEnergySums() const { return eSums_; }

    ///@return the QDC recorded on the module
    std::vector<unsigned int> GetQdc() const { return qdc_; }

    ///@return The trace that was sampled on the module
    std::vector<unsigned int> GetTrace() const { return trace_; }

    ///@brief Sets the baseline recorded on the module if the energy sums
    /// were recorded in the data stream
    ///@param[in] a : The value to set
    void SetBaseline(const double &a) { baseline_ = a; }

    ///@brief This value is set to true if the CFD was forced to trigger
    ///@param[in] a : The value to set
    void SetCfdForcedTriggerBit(const bool &a) { cfdForceTrig_ = a; }

    ///@brief Sets the CFD fractional time calculated on-board
    ///@param[in] a : The value to set
    void SetCfdFractionalTime(const unsigned int &a) { cfdTime_ = a; }

    ///@brief Sets the CFD trigger source
    ///@param[in] a : The value to set
    void SetCfdTriggerSourceBit(const bool &a) { cfdTrigSource_ = a; }

    ///@brief Sets the channel number
    ///@param[in] a : The value to set
    void SetChannelNumber(const unsigned int &a) { chanNum_ = a; }

    ///@brief Sets the crate number
    ///@param[in] a : The value to set
    void SetCrateNumber(const unsigned int &a) { crateNum_ = a; }

    ///@brief Sets the energy calculated on-board
    ///@param[in] a : The value to set
    void SetEnergy(const double &a) { energy_ = a; }

    ///@brief Sets the energy sums calculated on-board
    ///@param[in] a : The value to set
    void SetEnergySums(const std::vector<unsigned int> &a) { eSums_ = a; }

    ///@brief Sets the upper 16 bits of the event time
    ///@param[in] a : The value to set
    void SetEventTimeHigh(const unsigned int &a) { eventTimeHigh_ = a; }

    ///@brief Sets the lower 32 bits of the event time
    ///@param[in] a : The value to set
    void SetEventTimeLow(const unsigned int &a) { eventTimeLow_ = a; }

    ///@brief Sets the upper 16 bits of the external event time
    ///@param[in] a : The value to set
    void SetExternalTimeHigh(const unsigned int &a) { externalTimeHigh_ = a; }

    ///@brief Sets the lower 32 bits of the external event time
    ///@param[in] a : The value to set
    void SetExternalTimeLow(const unsigned int &a) { externalTimeLow_ = a; }

    ///@brief Sets if we had a pileup found on-board
    ///@param[in] a : The value to set
    void SetPileup(const bool &a) { isPileup_ = a; }

    ///@brief Sets the QDCs that were calculated on-board
    ///@param[in] a : The value to set
    void SetQdc(const std::vector<unsigned int> &a) { qdc_ = a; }

    ///@brief Sets the saturation flag
    ///@param[in] a : True if we found a saturation on board
    void SetSaturation(const bool &a) { isSaturated_ = a; }

    ///@brief Sets the slot number
    ///@param[in] a : The value to set
    void SetSlotNumber(const unsigned int &a) { slotNum_ = a; }

    ///@brief Sets the calculated arrival time of the signal
    ///@param[in] a : The value to set
    void SetTime(const double &a) { time_ = a; }

    ///@brief Sets the trace recorded on board
    ///@param[in] a : The value to set
    void SetTrace(const std::vector<unsigned int> &a) { trace_ = a; }

    ///@brief Sets the flag for channels generated on-board
    ///@param[in] a : True if we this channel was generated on-board
    void SetVirtualChannel(const bool &a) { isVirtualChannel_ = a; }

    ///@brief Clear all variables and set them to some default values.
    void Clear();

private:
    bool cfdForceTrig_; /// CFD was forced to trigger.
    bool cfdTrigSource_; /// The ADC that the CFD/FPGA synched with.
    bool isPileup_; /// Pile-up flag from Pixie.
    bool isSaturated_; /// Saturation flag from Pixie.
    bool isVirtualChannel_; /// Flagged if generated virtually in Pixie DSP.

    double energy_; /// Raw pixie energy.
    double baseline_;///Baseline that was recorded with the energy sums
    double time_;

    unsigned int cfdTime_; /// CFD trigger time
    unsigned int chanNum_; /// Channel number.
    unsigned int crateNum_; ///The Crate number for the channel
    unsigned int eventTimeHigh_; /// Upper 16 bits of pixie16 event time.
    unsigned int eventTimeLow_; /// Lower 32 bits of pixie16 event time.
    unsigned int externalTimeHigh_; ///Upper 16 bits of external time stamp
    unsigned int externalTimeLow_; ///Lower 32 bits of external time stamp
    unsigned int slotNum_; ///Slot number

    std::vector<unsigned int> eSums_;///Energy sums recorded by the module
    std::vector<unsigned int> qdc_; ///QDCs recorded by the module
    std::vector<unsigned int> trace_; /// ADC trace capture.
};

#endif