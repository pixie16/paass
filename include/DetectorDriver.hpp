/**
  \file DetectorDriver.hpp

  \brief Header file for the detector driver program.

  SNL - 7-2-07
  KM - 1-24-13
  Defines DetectorDriver which controls the flow of event processing.
*/

#ifndef __DETECTORDRIVER_HPP_
#define __DETECTORDRIVER_HPP_ 1

#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Plots.hpp"
#include "Globals.hpp"
#include "ChanEvent.hpp"
#include "Messenger.hpp"
#include "WalkCorrector.hpp"
#include "Calibrator.hpp"

// forward declarations
class Calibration;
class RawEvent;
class EventProcessor;
class TraceAnalyzer;

/**
  \brief DetectorDriver controls event processing

  This class controls the processing of each event and includes the
  calibration of all raw energies, the plotting of raw and calibrated
  energies, and any experiment specific processing requirements
*/
class DetectorDriver {    
 public:    
    static DetectorDriver* get();

    WalkCorrector walk;
    Calibrator cali;

    Plots histo;
    virtual void plot(int dammId, double val1, double val2 = -1,
                      double val3 = -1, const char* name="h") {
        histo.Plot(dammId, val1, val2, val3, name);
    }
    
    int ProcessEvent(RawEvent& rawev);
    int ThreshAndCal(ChanEvent *, RawEvent& rawev);
    int Init(RawEvent& rawev);

    int PlotRaw(const ChanEvent *);
    int PlotCal(const ChanEvent *);

    void DeclarePlots(); /**< declare the necessary damm plots */
    void SanityCheck(void) const;  /**< check whether everything makes sense */

    void CorrelateClock(double d, time_t t) {
        pixieToWallClock=std::make_pair(d, t);
    }

    time_t GetWallTime(double d) const {
        return (time_t)((d - pixieToWallClock.first) * 
                        Globals::get()->clockInSeconds() +
                        pixieToWallClock.second);
    }

    const std::vector<EventProcessor *>& GetProcessors(void) const {
        return vecProcess;
    } /**< return the list of processors */

    std::vector<EventProcessor *> GetProcessors(const std::string &type) const;

    const std::set<std::string> &GetUsedDetectors(void) const;

    virtual ~DetectorDriver();


 private: 
    DetectorDriver();
    DetectorDriver (const DetectorDriver&);
    DetectorDriver& operator= (DetectorDriver const&);
    static DetectorDriver* instance;

    std::vector<EventProcessor*> vecProcess; /**< vector of processors to handle each event */
    
    std::vector<TraceAnalyzer*> vecAnalyzer; /**< object which analyzes traces of channels to extract
				   energy and time information */
    std::set<std::string> knownDetectors; /**< list of valid detectors that can 
				   be used as detector types */
    
    std::pair<double, time_t> pixieToWallClock; /**< rough estimate of pixie to wall clock */ 

    virtual void DeclareHistogram1D(int dammId, int xSize, const char* title) {
        histo.DeclareHistogram1D(dammId, xSize, title);
    }
    virtual void DeclareHistogram2D(int dammId, int xSize, int ySize,
                                    const char* title) {
        histo.DeclareHistogram2D(dammId, xSize, ySize, title);
    }

    void LoadProcessors(Messenger& m);

    void ReadCalXml();
    void ReadWalkXml();

};

#endif // __DETECTORDRIVER_HPP_
