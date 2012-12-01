/** \file PositionProcessor.hpp
 * \brief Handle QDCs for the position sensitive detector
 */

#ifndef __POSITIONPROCESSOR_HPP_
#define __POSITIONPROCESSOR_HPP_

#include <string>
#include <vector>

#include "EventProcessor.hpp"
#include "RawEvent.hpp"

class ChanEvent;

class PositionProcessor : public EventProcessor
{
private:
    static const std::string configFile;
    static const int numQdcs = 8;
    static const int matchingTimeCut = 5; //< maximum difference between edge and sum timestamps

    float qdcLen[numQdcs]; //< the length of each qdc in pixie samples
    float qdcPos[numQdcs]; //< the ending sample number for each QDC position
    float totLen;          //< calculated length of all qdcs excluding baseline qdc
    int whichQdc;          //< which qdc we are using for position determinatio
    static const int maxNumLocations = 12;
    int numLocations;
    float posScale;        //< an arbitrary scale for the position parameter to physical units
    std::vector<float> minNormQdc; //< the minimum normalized qdc observed for a location
    std::vector<float> maxNormQdc; //< the maximum normalized qdc observed for a location
    
    ChanEvent* FindMatchingEdge(ChanEvent *match,
				std::vector<ChanEvent*>::const_iterator begin, 
				std::vector<ChanEvent*>::const_iterator end) const;				
    ChanEvent* FindMatchingEdge(ChanEvent *match,
				std::vector<ChanEvent*>::const_reverse_iterator begin, 
				std::vector<ChanEvent*>::const_reverse_iterator end) const;				
public:
    PositionProcessor(); // no virtual c'tors
    virtual bool Init(RawEvent& rawev);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
};
    
#endif // __POSITIONPROCESSOR_HPP_
