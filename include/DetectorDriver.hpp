/**
  \file DetectorDriver.hpp

  \brief Header file for the detector driver program.

  SNL - 7-2-07
  Defines two classes, Calibration and DetectorDriver
  Calibration is resonsible for the calibration information
  read into the analysis from the file cal.txt.  DetectorDriver
  controls the flow of event processing.
*/

#ifndef __DETECTORDRIVER_HPP_
#define __DETECTORDRIVER_HPP_ 1

#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Plots.hpp"
#include "Globals.hpp"
#include "MapFile.hpp"
#include "ChanEvent.hpp"

// forward declarations
class Calibration;
class RawEvent;
class EventProcessor;
class TraceAnalyzer;

using std::pair;
using std::set;
using std::string;
using std::vector;

/**
  \brief DetectorDriver controls event processing

  This class controls the processing of each event and includes the
  calibration of all raw energies, the plotting of raw and calibrated
  energies, and any experiment specific processing requirements
*/
class DetectorDriver {    
 private: 
    DetectorDriver();
    DetectorDriver (const DetectorDriver&);
    DetectorDriver& operator= (DetectorDriver const&);
    static DetectorDriver* instance;

    vector<EventProcessor *> vecProcess; /**< vector of processors to handle each event */
    
    vector<TraceAnalyzer *> vecAnalyzer; /**< object which analyzes traces of channels to extract
				   energy and time information */
    set<string> knownDetectors; /**< list of valid detectors that can 
				   be used as detector types */
    
    pair<double, time_t> pixieToWallClock; /**< rough estimate of pixie to wall clock */ 


    virtual void DeclareHistogram1D(int dammId, int xSize, const char* title) {
        histo.DeclareHistogram1D(dammId, xSize, title);
    }
    virtual void DeclareHistogram2D(int dammId, int xSize, int ySize, const char* title) {
        histo.DeclareHistogram2D(dammId, xSize, ySize, title);
    }

 public:    
    static DetectorDriver* get();
    vector<Calibration> cal;    /**<the calibration vector*/ 

    Plots histo;
    virtual void plot(int dammId, double val1, double val2 = -1, double val3 = -1, const char* name="h") {
        histo.Plot(dammId, val1, val2, val3, name);
    }
    
    int ProcessEvent(const string &, RawEvent& rawev);
    int ThreshAndCal(ChanEvent *, RawEvent& rawev);
    int Init(RawEvent& rawev);

    int PlotRaw(const ChanEvent *);
    int PlotCal(const ChanEvent *);

    void DeclarePlots(MapFile& theMapFile); /**< declare the necessary damm plots */
    bool SanityCheck(void) const;  /**< check whether everything makes sense */

    void CorrelateClock(double d, time_t t) {
	pixieToWallClock=std::make_pair(d, t);
    }
    time_t GetWallTime(double d) const {
	return (time_t)((d - pixieToWallClock.first)*pixie::clockInSeconds + pixieToWallClock.second);
    }
    const vector<EventProcessor *>& GetProcessors(void) const
	{return vecProcess;}; /**< return the list of processors */
    vector<EventProcessor *> GetProcessors(const string &type) const;
    const set<string> &GetUsedDetectors(void) const;

    ~DetectorDriver();

    void ReadCal();
};

/**
  \brief vector to hold all channel calibrations

  For all combinations of module number and channel number contained
  in the map.txt file there is a corresponding entry in the vector
  calibration.  The calibration information for a specific module and
  channel number can be retieved from the calibration vector at the
  position of module number * 16 + channel number.
 */
class Calibration {
 private:
    int id;                 /**< id of the detector determined as module # * 16 + channel number */
    string detType;         /**< type of detector */
    string detSubtype;      /**< sub type of detector */
    int detLocation;        /**< physical location of detector (strip#, det#) */
    unsigned int numCal;    /**< the number of calibrations for this channel */
    unsigned int polyOrder; /**< the order of the calibration */
    vector<float> thresh;   /**< the lower limit for each respective calibration */
    vector<float> val;      /**< the individual calibration coefficients in increasing order */
    
 public:
    double Calibrate(double raw); /**< return a calibrated energy for raw value */
    Calibration();

    friend void DetectorDriver::ReadCal(void);
};

#endif // __DETECTORDRIVER_HPP_
