///@file ChannelConfiguration.hpp
///@brief Class that holds information about channel specific information. Including things like the trace delay,
///@author S. V. Paulauskas
///@date May 31, 2017
#ifndef PAASS_CHANNELCONFIGURATION_HPP
#define PAASS_CHANNELCONFIGURATION_HPP

#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <tuple>

#include "TrapFilterParameters.hpp"

/// This class holds all of the information that can be set on a channel by channel basis. Fitting parameters,
/// trace delays, detector type, subtypes, tag lists, etc. This class replaces the Identifier class.
class ChannelConfiguration {
public:
    ChannelConfiguration() {}

    ~ChannelConfiguration() {}

    ///Constructor taking arguments and setting values, this constructor is used to maintain functionality with the
    /// previous Identifier class.
    ///@param [in] atype : the type to set
    ///@param [in] subType : the subType to set
    ///@param [in] loc : the location to set
    ChannelConfiguration(const std::string &atype, const std::string &subType, const unsigned int &loc) {
        type_ = atype;
        subtype_ = subType;
        location_ = loc;
    }

    ///Insert a tag to the ChannelConfiguration
    ///@param [in] s : the name of the tag to insert
    ///@param [in] n : the value of the tag to insert 
    void AddTag(const std::string &s) { tags_.insert(s); }

    ///@return the baseline threshold
    double GetBaselineThreshold() const { return baselineThreshold_; }

    ///@return the tuple containing the CFD Parameters.
    std::tuple<double, double, double> GetCfdParameters() const { return cfdParameters_; }

    ///@return The value of the starting position for particle discrimination in trace samples
    unsigned int GetDiscriminationStartInSamples() const { return discriminationStartInSamples_; }

    ///@return the energy filter parameters
    TrapFilterParameters GetEnergyFilterParameters() const { return energyFilterParameters_; }

    ///@return the pair of fitting parameters to use in fits.
    std::pair<double, double> GetFittingParameters() const { return fittingParameters_; }

    ///@return The value of the private variable location
    unsigned int GetLocation() const { return location_; }

    ///@return The name of the place associated with the channel 
    std::string GetPlaceName() const { return type_ + "_" + subtype_ + "_" + std::to_string(location_); }

    ///@return the value of the private variable subtype
    std::string GetSubtype() const { return subtype_; }

    ///@return Get the tag list
    std::set<std::string> GetTags() const { return tags_; }

    ///@return the trigger filter parameters
    TrapFilterParameters GetTriggerFilterParameters() const { return triggerFilterParameters_; }

    ///@return The value of the private variable type
    std::string GetType() const { return type_; }

    ///@return The value of the trace delay
    unsigned int GetTraceDelayInSamples() const { return traceDelayInSamples_; }

    ///@return The waveform bounds in samples
    std::pair<unsigned int, unsigned int> GetWaveformBoundsInSamples() const { return waveformBoundsInSeconds_; }

    ///Check if an identifier has a tag
    ///@param [in] s : the tag to search for
    ///@return true if the tag is in the identifier 
    bool HasTag(const std::string &s) const { return tags_.find(s) != tags_.end(); }

    ///Print all of the info for the ChannelConfiguration
    void Print(void) const {
        std::cout << std::setw(10) << type_ << std::setw(10) << subtype_ << std::setw(4) << location_ << "    ";
        for (std::set<std::string>::const_iterator it = tags_.begin(); it != tags_.end(); it++) {
            if (it != tags_.begin())
                std::cout << ", ";
            std::cout << *it;
        }
        std::cout << std::endl;
    };

    ///Sets the baseline threshold that we use to reject noisey signals. This is measured as the standard deviation
    /// of the baseline.
    void SetBaselineThreshold(const double &a) { baselineThreshold_ = a; }

    ///Sets the tuple containing the CFD parameters. Parameters are held in order of <F, D, L>. L is an optional
    /// parameter for most of the algorithms.
    ///@param[in] a : the parameters that we want to set.
    void SetCfdParameters(std::tuple<double, double, double> &a) { cfdParameters_ = a; }

    ///Sets the start position for the tail sums for doing particle discrimination
    void SetDiscriminationStartInSamples(const unsigned int &a) { discriminationStartInSamples_ = a; }

    ///Sets the energy filter parameters
    ///@param [in] a : the parameters that we want to set
    void SetEnergyFilterParameters(const TrapFilterParameters &a) { energyFilterParameters_ = a; }

    /// Sets the pair of parameters that are needed for fitting analysis
    ///@param[in] a : The pair of parameters to set
    void SetFittingParameters(const std::pair<double, double> &a) { fittingParameters_ = a; }

    ///Sets the location
    ///@param [in] a : sets the location for the channel
    void SetLocation(const unsigned int &a) { location_ = a; }

    ///Sets the subtype of the channel
    ///@param [in] a : the subtype to set 
    void SetSubtype(const std::string &a) { subtype_ = a; }

    /// Sets the trace delay in units of trace samples
    ///@param[in] a : The value for the trace delay in units of samples.
    void SetTraceDelayInSamples(const unsigned int &a) { traceDelayInSamples_ = a; }

    ///Sets the trigger filter parameters
    ///@param [in] a : the parameters that we want to set
    void SetTriggerFilterParameters(const TrapFilterParameters &a) { triggerFilterParameters_ = a; }

    ///Sets the type
    ///@param [in] a : the type to set
    void SetType(const std::string &a) { type_ = a; }

    ///Sets the bounds for the waveform measured from the maximum value of the trace in trace samples.
    ///@param[in] a : A pair containing the low bound (first) and high bound (second) for the waveform.
    void SetWaveformBoundsInSamples(const std::pair<unsigned int, unsigned int> &a) { waveformBoundsInSeconds_ = a; }

    ///Zeroes some of the variables in the class. The DAMM ID and detector location variable are reset to -1 and the
    /// detector type and sub type are both reset to "" when an identifier object is zeroed.
    void Zero() {
        location_ = 9999;
        type_ = "";
        subtype_ = "";
        tags_.clear();
    }

    ///Equality operator for ChannelConfiguration, we will not check every parameter as the location should be
    /// unique for a given type/subtype.
    ///@param [in] rhs : the ChannelConfiguration to compare to
    ///@return true if the type, subtype, and location are identical 
    bool operator==(const ChannelConfiguration &rhs) const {
        return type_ == rhs.type_ && subtype_ == rhs.subtype_ && location_ == rhs.location_;
    }

    ///Not - Equality operator for ChannelConfiguration
    ///@param [in] x : the ChannelConfiguration to compare to
    ///@return true if the type, subtype, or location do not match 
    bool operator!=(const ChannelConfiguration &rhs) const { return !operator==(rhs); }

    ///Less-then operator needed for STL containers
    ///@param [in] rhs : the ChannelConfiguration to compare
    ///@return true if the type, subtype, or location are less than those in rhs 
    bool operator<(const ChannelConfiguration &rhs) const {
        if (type_.compare(rhs.type_) > 0)
            return false;
        else if (type_.compare(rhs.type_) < 0)
            return true;
        else {
            if (subtype_.compare(rhs.subtype_) > 0)
                return false;
            else if (subtype_.compare(rhs.subtype_))
                return true;
            else
                return (location_ < rhs.location_);
        }
    }

    ///Greater-than operator needed for STL containers
    ///@param[in] rhs : The right hand side that we are comparing with.
    ///@return The negative of the less than operator.
    bool operator>(const ChannelConfiguration &rhs) const { return !operator<(rhs); }

private:
    double baselineThreshold_; ///< The threshold for the baseline to handle noisy traces.
    std::tuple<double, double, double> cfdParameters_; ///< The parameters to be used with the CFD routines
    unsigned int discriminationStartInSamples_; ///< The position from the max that we'll do particle discrimination
    TrapFilterParameters energyFilterParameters_; ///< Parameters to use for energy filter calculations
    std::pair<double, double> fittingParameters_; ///< The parameters to use for the fitting routines
    unsigned int location_; ///< Specifies the real world location of the channel.
    std::string subtype_; ///< Specifies the detector sub type
    std::set<std::string> tags_; ///< A list of associated tags
    unsigned int traceDelayInSamples_; ///< The trace delay to help find the location of waveforms in traces
    TrapFilterParameters triggerFilterParameters_; ///< Parameters to use for trigger filter calculations
    std::string type_; ///< Specifies the detector type
    std::pair<unsigned int, unsigned int> waveformBoundsInSeconds_; ///< The waveform range for the channel
};


#endif //PAASS_CHANNELCONFIGURATION_HPP
