/*! \file HighResPositionData.hpp
 *  \brief Class to hold all of the information for high resolution position
 *  \author S. V. Paulauskas, T. T. King
 *  \date November 9, 2014
 */
#ifndef __HIGHRESPOSITIONDATA_HPP__
#define __HIGHRESPOSITIONDATA_HPP__

#include "ChanEvent.hpp"
#include "Constants.hpp"
#include "Globals.hpp"

//! Class for holding information for high resolution position.
class HighResPositionData {
public:
    /** Default constructor */
    HighResPositionData() {};

    /** Default destructor */
    virtual ~HighResPositionData() {};

    /** Constructor using the channel event
    * \param [in] chan : the channel event for grabbing values from */
    HighResPositionData(double ft, double fb, double bt, double bb) {
        _ft = ft;
        _fb = fb;
        _bt = bt;
        _bb = bb;
        
        _sum = _ft + _fb + _bt + _bb;
    }

    double GetTotIntegral() const {return _sum;}

    ///@return high resolution position parallel to r^hat
    double GetHighResZPos() const {
      return -1.0*(_ft + _fb - _bt - _bb) / _sum ;
    }

    ///@return high resolution position perpendicular to r^(hat) (parallel to phi^hat)
    double GetHighResYPos() const {
      return (_ft + _bt - _fb - _bb) / _sum ;      
    }

    double GetFTQdc() const {
     return _ft;
    }

    double GetBTQdc() const {
      return _bt;
    }

    double GetFBQdc() const {
      return _fb;
    }

    double GetBBQdc() const {
      return _bb;
    }

    ///@return True if the trace was successfully analyzed
    bool GetIsValid() const {
        if ( _ft > 0 && _fb > 0 && _bt > 0 && _bb > 0 ) return (true);
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
