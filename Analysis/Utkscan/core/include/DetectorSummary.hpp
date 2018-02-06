/*! \file DetectorSummary.hpp
 *  \brief The class that generates the summary of the detectors in the analysis
 *  \author Unknown
 *  \date Unknown
 */
#ifndef __DETECTOR_SUMMARY_HPP
#define __DETECTOR_SUMMARY_HPP

#include <string>
#include <vector>

#include "ChanEvent.hpp"
#include "Globals.hpp"
#include "Trace.hpp"

class ChanEvent;

/** \brief Summary of all channels of one detector type
 *
 * For each group of detectors that exists in the analysis, a detector summary
 * is created.  The detector summary includes the multiplicity, maximum
 * deposited energy, and physical location (strip number, detector number ...)
 * where the maximum energy was deposited for a type of detectors.  Also
 * pointers to all channels of this type are stored in a vector, as well as the
 * channel event where the maximum energy was deposited.  Lastly, the detector
 * summary records the detector name to which it applies.
 */
class DetectorSummary {
public:
    /** Default Constructor */
    DetectorSummary();

    /** Constructor taking a string and the full channel list
     * \param [in] str : the type to make the summary for
     * \param [in] fullList : the full list of channels in the event */
    DetectorSummary(const std::string &str, const std::vector<ChanEvent *> &fullList);

    /** Zero the summary */
    void Zero();

    /** Add an event to the summary
     * \param [in] ev : the event to add */
    void AddEvent(ChanEvent *ev);

    /** Set the detector name
     * \param [in] a : the name of the detector */
    void SetName(const std::string& a) {name = a;}

    /** \return the max event in the summary (constant) */
    const ChanEvent* GetMaxEvent(void) const {return maxEvent;};

    /** \return the max event with the ability to change it
     * \param [in] fake : a bool to allow overloading the function name */
    ChanEvent* GetMaxEvent(bool fake) {return maxEvent;};

    /** \return the multiplicity of the summary */
    int GetMult() const {return eventList.size();}

    /** \return get the detector name */
    const std::string& GetName() const {return name;};

    /** \return the list of al channels in the raw event with this detector type */
    const std::vector<ChanEvent*>& GetList() const {return eventList;};
private:
    std::string name;                  /**< name associated with this summary */
    std::string type;                  /**< detector type associated with this summary */
    std::string subtype;               /**< detector subtype associated with this summary */
    std::string tag;               /**< detector tag associated with this summary */
    std::vector<ChanEvent*> eventList; /**< list of events associated with this detector group */
    ChanEvent* maxEvent;               /**< event with maximum energy deposition */
};
#endif
