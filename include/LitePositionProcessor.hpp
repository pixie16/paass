/** \file LitePositionProcessor.hpp
 * \brief Handle QDCs for the position sensitive detector,
 *   a minimal number of spectra are created
 */

#ifndef __LITEPOSITIONPROCESSOR_HPP_
#define __LITEPOSITIONPROCESSOR_HPP_

#include <string>
#include <vector>

#include "EventProcessor.hpp"

class ChanEvent;

//! LitePositionProcessor
class LitePositionProcessor : public EventProcessor {
public:
    LitePositionProcessor();
    ~LitePositionProcessor(){};
    virtual bool Init(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
protected:
    static const std::string configFile;
    static const int numQdcs = 8;

    float qdcLen[numQdcs]; //!< the length of each qdc in pixie samples
    float qdcPos[numQdcs]; //!< the ending sample number for each QDC position
    float totLen;          //!< calculated length of all qdcs excluding baseline qdc
    int whichQdc;          //!< which qdc we are using for position determination
    int numLocations;      //!< The number of locations
    float posScale;        //!< an arbitrary scale for the position parameter to physical units
    std::vector<float> minNormQdc; //!< the minimum normalized qdc observed for a location
    std::vector<float> maxNormQdc; //!< the maximum normalized qdc observed for a location

    /*! \return The matching edge of the input event */
    ChanEvent* FindMatchingEdge(ChanEvent *match,
				std::vector<ChanEvent*>::const_iterator begin,
				std::vector<ChanEvent*>::const_iterator end) const;
};

#endif // __LITEPOSITIONPROCESSOR_HPP_
