/**
  \file DetectorDriver.h

  \brief Header file for the detector driver program.

  SNL - 7-2-07
  Defines two classes, Calibration and DetectorDriver
  Calibration is resonsible for the calibration information
  read into the analysis from the file cal.txt.  DetectorDriver
  controls the flow of event processing.
*/

#ifndef __DETECTORDRIVER_H_
#define __DETECTORDRIVER_H_ 1

#include <set>
#include <string>
#include <vector>

#include "TraceAnalyzer.h"

#include "param.h"

// forward declarations
class Calibration;
class RawEvent;
class ChanEvent;
class EventProcessor;

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
    vector<EventProcessor *> vecProcess; /**< vector of processors to handle each event */
    
    TraceAnalyzer traceSub;     /**< object which analyzes traces of channels to extract
				   energy and time information */
    set<string> knownDetectors; /**< list of valid detectors that can 
				   be used as detector types */
 public:    
    vector<Calibration> cal;    /**<the calibration vector*/ 
    
    int ProcessEvent(const string &);
    int ThreshAndCal(ChanEvent *);
    int Init(void);
    int PlotRaw(const ChanEvent *) const;
    int PlotCal(const ChanEvent *) const;
    const set<string>& GetKnownDetectors(void);
    const set<string>& GetUsedDetectors(void) const;

    void DeclarePlots(void) const; /**< declare the necessary damm plots */
    bool SanityCheck(void) const;  /**< check whether everything makes sense */

    const vector<EventProcessor *>& GetProcessors(void) const
	{return vecProcess;}; /**< return the list of processors */
    vector<EventProcessor *> GetProcessors(const string &type) const;

    DetectorDriver();
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

#endif // __DETECTORDRIVER_H_
