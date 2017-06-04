///@file ProcessedXiaData.hpp
///@brief An XiaData object that has undergone additional processing.
///@author S. V. Paulauskas and C. R. Thornsberry
///@date December 2, 2016
#ifndef PIXIESUITE_PROCESSEDXIADATA_HPP
#define PIXIESUITE_PROCESSEDXIADATA_HPP

#include "Trace.hpp"
#include "XiaData.hpp"

///This class contains additional information about the XiaData after
/// additional processing has been done. The processing includes, but is not
/// limited to energy/time calibrations, high resolution timing analysis,
/// trace analysis, etc. 
class ProcessedXiaData : public XiaData {
public:
    /// Default constructor.
    ProcessedXiaData() {}

    ///Constructor taking the base class as an argument so that we can set
    /// the trace information properly
    ///@param[in] evt : The event that we are going to assign here.
    ProcessedXiaData(XiaData &evt) : XiaData(evt) {
        trace_ = evt.GetTrace();
        trace_.SetIsSaturated(evt.IsSaturated());
        walkCorrectedTime_ = 0;
    };

    /// Default Destructor.
    ~ProcessedXiaData() {}

    ///@return The calibrated energy for the channel
    double GetCalibratedEnergy() const { return calibratedEnergy_; }

    ///@return The sub-sampling arrival time of the signal in nanoseconds.
    double GetHighResTimeInNs() const { return highResTimeInNs_; }

    ///@return A constant reference to the trace.
    const Trace &GetTrace() const { return trace_; }

    ///@return An editable trace.
    Trace &GetTrace() { return trace_; }

    ///@return The Walk corrected time of the channel
    double GetWalkCorrectedTime() const { return walkCorrectedTime_; }

    ///Set the calibrated energy
    ///@param [in] a : the calibrated energy
    void SetCalibratedEnergy(const double &a) { calibratedEnergy_ = a; }

    ///Set to True if we would like to ignore this channel
    ///@param[in] a : The value that we want to set
    void SetIsIgnored(const bool &a) { isIgnored_ = a; }

    ///Set to true if the energy and time are not bogus values.
    ///@param[in] a : The value that we would like to set
    void SetIsValidData(const bool &a) { isValidData_ = a; }

    ///Set the high resolution time (Filter time (sans CFD) + phase ).
    ///@param [in] a : the high resolution time
    void SetHighResTime(const double &a) { highResTimeInNs_ = a; }

    ///Sets the trace appropriately
    ///@param[in] a : The trace that we want to set
    void SetTrace(const std::vector<unsigned int> &a) { trace_ = a; }

    ///Set the Walk corrected time
    ///@param [in] a : the walk corrected time */
    void SetWalkCorrectedTime(const double &a) { walkCorrectedTime_ = a; }

private:
    Trace trace_; ///< A Trace object to handle the Trace related stuff.

    bool isIgnored_; ///< True if we ignore this event.
    bool isValidData_; ///< True if the energy and High Res time are valid.

    double calibratedEnergy_; ///< The energy after calibration
    double highResTimeInNs_; ///< High Res time taken from pulse fits (in ns).
    double walkCorrectedTime_; ///< The time after walk corrections
};

#endif //PIXIESUITE_PROCESSEDXIADATA_HPP
