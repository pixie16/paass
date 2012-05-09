/*! \file GeProcessor.h
 *
 * Processor for germanium detectors
 */

#ifndef __GEPROCESSOR_H_
#define __GEPROCESSOR_H_

#include <map>
#include <vector>

#include "EventProcessor.h"

class GeProcessor : public EventProcessor 
{
private:
    static const unsigned int chansPerClover = 4; /*!< number of channels per clover */

    std::map<int, int> leafToClover;   /*!< Translate a leaf location to a clover number */
    std::vector<float> timeResolution; /*!< Contatin time resolutions used */
    unsigned int numClovers;           /*!< number of clovers in map */

    double WalkCorrection(double e);

    void DeclareHistogramGranY(int dammId, int xsize, int ysize, 
			       const char *title, int halfWordsPerChan,
			       const std::vector<float> &granularity, const char *units ) const;
public:
    GeProcessor(); // no virtual c'tors
    virtual bool Init(DetectorDriver &driver);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void) const;
};

#endif // __GEPROCESSOR_H_
