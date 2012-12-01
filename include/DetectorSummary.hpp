#ifndef __DETECTOR_SUMMARY_HPP
#define __DETECTOR_SUMMARY_HPP

#include <vector>
#include <string>
#include "ChanEvent.hpp"
#include "pixie16app_defs.h"
#include "Globals.hpp"
#include "Trace.hpp"

/** \brief Summary of all channels of one detector type
 * For each group of detectors that exists in the analysis, a detector summary
 * is created.  The detector summary includes the multiplicity, maximum
 * deposited energy, and physical location (strip number, detector number ...)
 * where the maximum energy was deposited for a type of detectors.  Also 
 * pointers to all channels of this type are stored in a vector, as well as the
 * channel event where the maximum energy was deposited.  Lastly, the detector
 * summary records the detector name to which it applies.
 */
class DetectorSummary
{
private:
    std::string name;                  /**< name associated with this summary */
    std::string type;                  /**< detector type associated with this summary */
    std::string subtype;               /**< detector subtype associated with this summary */
    std::string tag;               /**< detector tag associated with this summary */
    std::vector<ChanEvent*> eventList; /**< list of events associated with this detector group */
    ChanEvent* maxEvent;               /**< event with maximum energy deposition */
public:
    DetectorSummary();
    DetectorSummary(const std::string &str, const std::vector<ChanEvent *> &fullList);
    void Zero();
    void AddEvent(ChanEvent *ev); /**< Add a channel event to the summary */

    void SetName(const std::string& a) {name = a;} /**< Set the detector type name */
    
    const ChanEvent* GetMaxEvent(void) const 
    {return maxEvent;}          /**< Get the max event */
    ChanEvent* GetMaxEvent(bool fake)
    {return maxEvent;}          /**< Get the max event with possibility to alter */
    int GetMult() const 
    {return eventList.size();} /**< Get the multiplicity */
    const std::string& GetName() const          
    {return name;}              /**< Get the detector type name */
    const std::vector<ChanEvent*>& GetList() const 
    {return eventList;}         /**< Get the list of all channels in the
				   rawevent that are of this detector type */
};

#endif
