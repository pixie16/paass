/** \file PositionProcessor.hpp
 * \brief Handle QDCs for the position sensitive detector
 * @authors D. Miller, K. Miernik
 */

#ifndef __POSITIONPROCESSOR_HPP__
#define __POSITIONPROCESSOR_HPP__

#include <string>
#include <vector>

#include "EventProcessor.hpp"
#include "RawEvent.hpp"

class ChanEvent;

//! Processor to calculate the position in SSDs
class PositionProcessor : public EventProcessor {
private:
    static const int numQdcs = 8; //!< number of QDCs to compute
    static const int matchingTimeCut = 5; //!< maximum difference between edge and sum timestamps

    float qdcLen[numQdcs]; //!< the length of each qdc in pixie samples
    float qdcPos[numQdcs]; //!< the ending sample number for each QDC position
    float totLen;          //!< calculated length of all qdcs excluding baseline qdc
    int whichQdc;          //!< which qdc we are using for position determinatio
    static const int maxNumLocations = 12; //!< maximum number of locations
    int numLocations; //!< number of locations in the processor
    float posScale;        //!< an arbitrary scale for the position parameter to physical units
    std::vector<float> minNormQdc; //!< the minimum normalized qdc observed for a location
    std::vector<float> maxNormQdc; //!< the maximum normalized qdc observed for a location

    /** Find the matching edge of the SSD
    * \param [in] match : the matching edge
    * \param [in] begin : an iterator to the beginning of the event
    * \param [in] end : an iterator to the end of the event
    * \return a pointer to the matching ChanEvent */
    ChanEvent *FindMatchingEdge(ChanEvent *match,
                                std::vector<ChanEvent *>::const_iterator begin,
                                std::vector<ChanEvent *>::const_iterator end) const;

    /** Find the matching edge of the SSD
    * \param [in] match : the matching edge
    * \param [in] begin : an iterator to the beginning of the event
    * \param [in] end : an iterator to the end of the event
    * \return a pointer to the matching ChanEvent */
    ChanEvent *FindMatchingEdge(ChanEvent *match,
                                std::vector<ChanEvent *>::const_reverse_iterator begin,
                                std::vector<ChanEvent *>::const_reverse_iterator end) const;

public:
    PositionProcessor(); // no virtual c'tors
    /*! \brief Reads in QDC parameters from an input file
    *
    *   The file format allows comment lines at the beginning
    *   Followed by QDC lengths
    *   Which QDC to use for position calculation
    *     followed by the amount to scale the [0,1] result by to physical units
    *   And min and max values of the normalized QDC for each location in form:
    *      (location) (min) (max)
    *   Note that QDC 0 is considered to be a baseline section of the trace for
    *     baseline removal for the other QDCs
    * \param [in] rawev : the raw event to get the data from
    * \return true if everything was successful */
    virtual bool Init(RawEvent &rawev);

    /** \brief Process the QDC data involved in top/bottom side for a strip
    *
    *  Note: QDC lengths are HARD-CODED at the moment for the plots and to
    *  determine the position
    * \param [in] event : the event to process
    * \return true if the processing was successful
    */
    virtual bool Process(RawEvent &event);

    /** Declares the plots for the processor */
    virtual void DeclarePlots(void);
};

#endif // __POSITIONPROCESSOR_HPP_
