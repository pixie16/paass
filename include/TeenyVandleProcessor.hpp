/** \file TeenyVandleProcessor.hpp
 * 
 * Class for handling Analysis for Teeny Vandle
 */

#ifndef __TEENYVANDLEPROCESSOR_HPP_
#define __TEENYVANDLEPROCESSOR_HPP_

#include "EventProcessor.hpp"
#include "TimingInformation.hpp"

class TeenyVandleProcessor : public EventProcessor, 
			     public TimingInformation
{
 public:
    TeenyVandleProcessor(); // no virtual c'tors
    virtual void DeclarePlots(void);
    virtual bool Process(RawEvent &event);
 protected:
    TimingDataMap tVandleMap;
 private:
    virtual bool RetrieveData(RawEvent &event);
    virtual void AnalyzeData(void);
    int counter;
}; //Class TeenyVandleProcessor
#endif // __TEENYVANDLEROCESSOR_HPP_
