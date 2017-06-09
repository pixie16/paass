/** \file LitePositionProcessor.hpp
 * \brief Handle QDCs for the position sensitive detector,
 *   a minimal number of spectra are created
 *   @authors D. Miller, K. Miernik
 */
#ifndef __LITEPOSITIONPROCESSOR_HPP_
#define __LITEPOSITIONPROCESSOR_HPP_

#include <string>
#include <vector>

#include "EventProcessor.hpp"

class ChanEvent;

//! Version of PositionProcessor that is light weight
class LitePositionProcessor : public EventProcessor {
public:
    /** Default Constructor */
    LitePositionProcessor();

    /** Default Destructor */
    ~LitePositionProcessor() {};

    /** Initialize the processor
    * \param [in] event : the event to use for init
    * \return true if init was successful */
    bool Init(RawEvent &event);

    /** Process an event
* \param [in] event : the event to process
* \return true if the processing was successful */
    bool Process(RawEvent &event);

    /** Declare plots for processor */
    void DeclarePlots(void);

protected:
    static const std::string configFile; //!< Configuration file name
    static const int numQdcs = 8;//!< Number of QDCs to calculate

    float qdcLen[numQdcs]; //!< the length of each qdc in pixie samples
    float qdcPos[numQdcs]; //!< the ending sample number for each QDC position
    float totLen;          //!< calculated length of all qdcs excluding baseline qdc
    int whichQdc;          //!< which qdc we are using for position determination
    int numLocations;      //!< The number of locations
    float posScale;        //!< an arbitrary scale for the position parameter to physical units
    std::vector<float> minNormQdc; //!< the minimum normalized qdc observed for a location
    std::vector<float> maxNormQdc; //!< the maximum normalized qdc observed for a location

    /** Find the matching edge of the SSD
    * \param [in] match : the matching edge
    * \param [in] begin : an iterator to the beginning of the event
    * \param [in] end : an iterator to the end of the event
    * \return a pointer to the matching ChanEvent */
    ChanEvent *FindMatchingEdge(ChanEvent *match, std::vector<ChanEvent *>::const_iterator begin,
                                std::vector<ChanEvent *>::const_iterator end) const;
};

#endif // __LITEPOSITIONPROCESSOR_HPP_
