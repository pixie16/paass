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
    int minCloverLoc;                  /*!< smallest clover location in map */
    unsigned int numClovers;           /*!< number of clovers in map */
    vector<double> cloverEnergy;       /*!< array to help with addback calc */
    vector<int> cloverMultiplicity;    /*!< array for multiplicity */

    void ConstructAddback(const ChanEvent *ch);	
    unsigned int GetCloverNum(int location) const {
	return (location - minCloverLoc) / chansPerClover;
    }
    void ClearClovers();
 public:
    GeProcessor(); // no virtual c'tors
    virtual bool Init(DetectorDriver &driver);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void) const;
};

#endif // __GEPROCESSOR_H_
