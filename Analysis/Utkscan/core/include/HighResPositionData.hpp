/*! \file HighResPositionData.hpp
 *  \brief Class to hold all of the information for high resolution timing
 *  \author S. V. Paulauskas, T. T. King
 *  \date November 9, 2014
 */
#ifndef __HIGHRESPOSITIONDATA_HPP__
#define __HIGHRESPOSITIONDATA_HPP__

#include "ChanEvent.hpp"
#include "Constants.hpp"
#include "Globals.hpp"

//! Class for holding information for high resolution timing. All times more
//! precise than the filter time will be in nanoseconds (phase, highResTime).
class HighResPositionData {
public:
    /** Default constructor */
    HighResPositionData() {};

    /** Default destructor */
    virtual ~HighResPositionData() {};

    /** Constructor using the channel event
    * \param [in] chan : the channel event for grabbing values from */
    HighResPositionData(std::vector< ChanEvent *> &evt) {
        for (std::vector <ChanEvent *>::iterator it = evt.begin(); it != evt.end(); it++){
          if( (*it)->GetChanID().HasTag("FT") && _ft <= 0 ) _ft = (*it)->GetTrace().GetQDC();
          if( (*it)->GetChanID().HasTag("FB") && _fb <= 0 ) _fb = (*it)->GetTrace().GetQDC();
          if( (*it)->GetChanID().HasTag("BT") && _bt <= 0 ) _bt = (*it)->GetTrace().GetQDC();
          if( (*it)->GetChanID().HasTag("BB") && _bb <= 0 ) _bb = (*it)->GetTrace().GetQDC();
        }
        _sum = _ft + _fb + _bt + _bb;
    }

    double GetTotIntegral(void) {return _sum;}

    ///@return high resolution position parallel to r^hat
    double GetHighResZPos(void){
      return (_ft + _fb - _bt - _bb) / _sum * 48.0;
    }

    ///@return high resolution position perpendicular to r^(hat) (parallel to phi^hat)
    double GetHighResYPos(void){
      return (_ft + _bt - _fb - _bb) / _sum * 50.8 ;
    }

    ///@return True if the trace was successfully analyzed
    bool GetIsValid() const {
        if ( _ft > 0 && _fb > 0 && _bt > 0 && _bb > 0 )return (true);
        else return (false);
    }

private:
    double _ft;
    double _fb;
    double _bt;
    double _bb;
    double _sum;

};

/** Defines a map to hold timing data for a channel. */
//typedef std::map<TimingDefs::TimingIdentifier, HighResPositionData> PositionMap;
#endif // __HIGHRESPOSITIONDATA_HPP__
