/** \file VandleProcessor.hpp
 * 
 * Class for handling Vandle Bars
 */

#ifndef __VANDLEPROCESSOR_HPP_
#define __VANDLEPROCESSOR_HPP_

#include "EventProcessor.hpp"
#include "TimingInformation.hpp"

class VandleProcessor : public EventProcessor, 
			public TimingInformation
{
 public:
    VandleProcessor(); // no virtual c'tors
    virtual void DeclarePlots(void);
    virtual bool Process(RawEvent &event);
    virtual bool Init(DetectorDriver &);
     
 protected:
    //define the maps
    BarMap  barMap; 
    TimingDataMap bigMap;
    TimingDataMap smallMap;
    TimingDataMap startMap;

 private:
    virtual bool RetrieveData(RawEvent &event);
    
    virtual double CorrectTOF(const double &TOF, 
			      const double &corRadius, 
			      const double &z0);

    virtual void AnalyzeData(void);
    virtual void BuildBars(const TimingDataMap &endMap, const std::string &type, 
			   BarMap &barMap);
    virtual void ClearMaps(void);
    virtual void CrossTalk(void);

    bool hasDecay;
    double decayTime;

    typedef std::pair<unsigned int, unsigned int> CrossTalkKey; 
    typedef std::map<CrossTalkKey, double> CrossTalkMap;
    std::map<CrossTalkKey, double> crossTalk;

    int counter;
}; //Class VandleProcessor
#endif // __VANDLEPROCESSOR_HPP_
