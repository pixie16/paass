///@file DetectorSummary.hpp
///@brief A class that generates summaries of detector types/subtypes/tags that are requested by the user. I'm not sure who
/// wrote the original version of this file.
///@author D. Miller, K. Miernik, S. V. Paulauskas
///@date May 28, 2017
#ifndef __DETECTOR_SUMMARY_HPP__
#define __DETECTOR_SUMMARY_HPP__

#include <string>
#include <vector>

#include "ChanEvent.hpp"

///For each group of detectors that exists in the analysis, a detector summary is created.  The detector summary includes the 
/// multiplicity, maximum deposited energy, and physical location (strip number, detector number ...) where the maximum energy 
/// was deposited for a type of detectors.  Also pointers to all channels of this type are stored in a vector, as well as the 
/// channel event where the maximum energy was deposited.  Lastly, the detector summary records the detector name to which it 
/// applies.
class DetectorSummary {
public:
    ///Default Constructor 
    DetectorSummary();

    ///Constructor taking a string and the full channel list
    ///@param [in] str : the type to make the summary for
    ///@param [in] fullList : the full list of channels in the event 
    DetectorSummary(const std::string &str, const std::vector<ChanEvent *> &fullList);

    /// Zero the summary 
    void Zero();

    /// Add an event to the summary
    ///@param [in] ev : the event to add 
    void AddEvent(ChanEvent *ev);

    /// Set the detector name
    ///@param [in] a : the name of the detector 
    void SetName(const std::string &a) { name_ = a; }

    ///@return the max event in the summary (constant) 
    const ChanEvent *GetMaxEvent(void) const { return maxEvent_; }

    ///@return the max event with the ability to change it
    ///@param [in] fake : a bool to allow overloading the function name 
    ChanEvent *GetMaxEvent(bool fake) { return maxEvent_; }

    ///@return the multiplicity of the summary 
    int GetMult() const { return eventList_.size(); }

    ///@return get the detector name 
    const std::string &GetName() const { return name_; }

    ///@return the list of al channels in the raw event with this detector type 
    const std::vector<ChanEvent *> &GetList() const { return eventList_; }

    ///To use the detector summary in the STL map it is necessary to define the behavior of the "<" operator.  The "<" operator
    /// between a and b is defined as whether the name for a is less than the name for b.
    ///@param [in] a : the Summary on the LHS
    ///@param [in] b : the summary on the RHS
    ///@return true if LHS is less than RHS
    bool operator<(const DetectorSummary &rhs) { return this->name_ < rhs.GetName(); }

private:
    std::string name_; //!<name associated with this summary
    std::string type_; //!<detector type associated with this summary
    std::string subtype_; //!<detector subtype associated with this summary
    std::string tag_; //!<detector tag associated with this summary
    std::vector<ChanEvent *> eventList_; //!<list of events associated with this detector group 
    ChanEvent *maxEvent_; //!<event with maximum energy deposition
};

#endif
