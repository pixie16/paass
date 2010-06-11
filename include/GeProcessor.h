/*! \file GeProcessor.h
 *
 * Processor for germanium detectors
 */

#ifndef __GEPROCESSOR_H_
#define __GEPROCESSOR_H_

#include <string>
#include <vector>

#include "EventProcessor.h"

class GeProcessor : public EventProcessor 
{
 private:
    static const unsigned int chansPerClover = 4; /*!< number of channels per clover */
    vector<double> cloverEnergy;       /*!< array to help with addback calc */

    void ConstructAddback(double, int, const std::string &);
 public:
    GeProcessor(); // no virtual c'tors
    virtual bool Init(DetectorDriver &driver);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void) const;
};

#endif // __GEPROCESSOR_H_
