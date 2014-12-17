/*! \file TimingMapBuilder.hpp
 *  \brief Class to build timing maps
 *  \author S. V. Paulauskas
 *  \date December 16, 2014
*/
#ifndef __TIMINGMAPBUILDER_HPP__
#define __TIMINGMAPBUILDER_HPP__

#include "ChanEvent.hpp"
#include "HighResTimingData.hpp"

//! A class that builds timing maps from a list of ChanEvents
class TimingMapBuilder {
public:
    /** Default constructor */
    TimingMapBuilder(){};
    /** Default destructor */
    ~TimingMapBuilder(){};

    /** Constructor taking in a list of Channel Events
     * \param [in] evts : The list of events */
    TimingMapBuilder(const std::vector<ChanEvent*> &evts);

    TimingMap GetMap(void){return(map_);};
private:
    void FillMaps(const std::vector<ChanEvent*> &evts);
    TimingMap map_;
};
#endif // __TIMINGMAPBUILDER_HPP__
