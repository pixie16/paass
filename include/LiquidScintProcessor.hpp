/** \file LiquidScintProcessor.hpp
 *
 * Processor for liquid scintillator detectors
 */

#ifndef __LIQUIDSCINTPROCESSOR_HPP_
#define __LIQUIDSCINTPROCESSOR_HPP_

#include "EventProcessor.hpp"
#include "Trace.hpp"

class LiquidScintProcessor : public EventProcessor,
		       public TimingInformation
{
public:
    LiquidScintProcessor(); // no virtual c'tors
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);

    /** KM 10/20/12 
    * Here's some old comment which I leave as a good joke
    * about multiple inheritance class:
    */
    // nice and simple raw derived class
private:
   unsigned int counter;
};

#endif // __LIQUIDSCINTPROCSSEOR_HPP_
