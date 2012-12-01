/** \file VandleProcessor.hpp
 * \brief Class for handling Vandle Bars
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
     
 protected:
    //define the maps
    BarMap  barMap; 
    TimingDataMap bigMap;
    TimingDataMap smallMap;
    TimingDataMap startMap;
    TimingDataMap tvandleMap;

 private:
    virtual bool RetrieveData(RawEvent &event);
    
    virtual double CorrectTOF(const double &TOF, 
			      const double &corRadius, 
			      const double &z0) {return((z0/corRadius)*TOF);};

    virtual void AnalyzeData(RawEvent& rawev);
    virtual void BuildBars(const TimingDataMap &endMap, 
                           const std::string &type, 
			   BarMap &barMap);
    virtual void ClearMaps(void);
    virtual void CrossTalk(void);
    virtual void FillMap(const vector<ChanEvent*> &eventList, 
                         const std::string type,
			 TimingDataMap &eventMap);
    virtual void Tvandle(void);
    virtual void WalkBetaVandle(const TimingInformation::TimingDataMap &beta, 
                                const TimingInformation::BarData &bar);

    bool hasDecay;
    double decayTime;
    int counter;

    typedef std::pair<unsigned int, unsigned int> CrossTalkKey; 
    typedef std::map<CrossTalkKey, double> CrossTalkMap;
    std::map<CrossTalkKey, double> crossTalk;
}; //Class VandleProcessor
#endif // __VANDLEPROCESSOR_HPP_
