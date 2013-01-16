#ifndef EVENT_DATA_H
#define EVENT_DATA_H

#include <string>
#include "Globals.hpp"

/** Simple structure holding basic parameters needed for correlation
 * of events in the same place. */
class EventData {
    public:
        /** Time is always needed, by default status is true,  
         * Energy is 0 (i.e. N/A), location -1 (N/A),
         * and event type is an empty string.*/
        EventData(double ptime, bool pstatus = true, double penergy = 0,
                  int plocation = -1, std::string ptype = "") {
            time = ptime;

            status = pstatus;
            energy = penergy;
            location = plocation;
            type = ptype;
        }

        /** Time, energy, location type of constructor */
        EventData(double ptime, double penergy, int plocation = -1,
                  bool pstatus = true, std::string ptype = "") {
            time = ptime;
            energy = penergy;
            location = plocation;

            status = pstatus;
            type = ptype;
        }

        bool status;
        double time;
        double energy;
        int location;
        std::string type;
};

#endif
