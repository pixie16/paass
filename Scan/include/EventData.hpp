/** \file EventData.hpp
 * \brief Structure holding event data
 */
#ifndef EVENT_DATA_H
#define EVENT_DATA_H

#include <string>
#include "Globals.hpp"

/** \brief Simple structure holding basic parameters needed for correlation
 * of events in the same place. */
class EventData {
public:
    /** Time is always needed, by default status is true,
     * Energy is 0 (i.e. N/A), location -1 (N/A),
     * and event type is an empty string.
     * \param [in] ptime : the time to set
     * \param [in] pstatus : the status to set
     * \param [in] penergy : the energy to set
     * \param [in] plocation : the location to set
     * \param [in] ptype : the type to set */
    EventData(double ptime, bool pstatus = true, double penergy = 0,
              int plocation = -1, std::string ptype = "") {
        time = ptime;
        status = pstatus;
        energy = penergy;
        location = plocation;
        type = ptype;
    }

    /** Time, energy, location type of constructor
     * \param [in] ptime : the time to set
     * \param [in] penergy : the energy to set
     * \param [in] plocation : the location to set
     * \param [in] pstatus : the status to set
     * \param [in] ptype : the type to set */
    EventData(double ptime, double penergy, int plocation = -1,
              bool pstatus = true, std::string ptype = "") {
        time = ptime;
        energy = penergy;
        location = plocation;
        status = pstatus;
        type = ptype;
    }

    bool status; //!< the status of the event
    double time;//!< the time of the event
    double energy;//!< the energy of the event
    int location;//!< the location of the detector
    std::string type;//!< the detector type
};
#endif
