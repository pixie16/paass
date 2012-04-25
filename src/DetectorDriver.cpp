/*    the detector driver class.

      The main analysis program.  A complete event is create in PixieStd
      passed into this class.  See manual for further details.

      SNL - 7-2-07
      SNL - 7-12-07
            Add root analysis. If the ROOT program has been
            detected on the computer system the and the
            makefile has the useroot flag declared, ROOT
            analysis will be included.
      DTM - Oct. '09
            Significant structural/cosmetic changes. Event processing is
	    now primarily handled by individual event processors which
	    handle their own DetectorDrivers

      SVP - Oct. '10
            Added the VandleProcessor for use with VANDLE.
            Added the PulserProcessor for use with Pulsers.
            Added the WaveformProcessor to determine ps time resolutions.
*/

/*!
  \file DetectorDriver.cpp

  \brief event processing

  In this file are the details for experimental processing of a raw event
  created by ScanList() in PixieStd.cpp.  Event processing includes things
  which do not change from experiment to experiment (such as energy
  calibration and raw parameter plotting) and things that do (differences
  between MTC and RMS experiment, for example).
*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <iterator>

#include "DetectorDriver.h"
#include "DetectorLibrary.hpp"
#include "RandomPool.h"
#include "RawEvent.h"

#include "damm_plotids.h"

#include "DssdProcessor.h"
#include "GeProcessor.h"
#include "IonChamberProcessor.h"
#include "McpProcessor.h"
#include "MtcProcessor.h"
#include "ScintProcessor.h"
#include "TraceFilterer.h"
#include "VandleProcessor.h"
#include "PulserProcessor.h"

#include "DoubleTraceAnalyzer.h"
#include "TraceAnalyzer.h"
#include "TraceExtracter.h"
#include "WaveformAnalyzer.h"

#ifdef useroot
#include "RootProcessor.h"
#endif

using namespace std;

/* rawevent declared in PixieStd.cpp
 *
 * driver relies on the respective DetectorSummary addresses remaining constant
 */
extern RawEvent rawev;

// pool of random numbers declared in RandomPool.cpp
extern RandomPool randoms;

/*!
  detector driver constructor

  Creates instances of all event processors
*/
DetectorDriver::DetectorDriver()
{
    vecAnalyzer.push_back(new DoubleTraceAnalyzer());
    vecAnalyzer.push_back(new TraceExtracter("ssd", "implant"));

    vecProcess.push_back(new ScintProcessor());
    // vecProcess.push_back(new GeProcessor());
    vecProcess.push_back(new IonChamberProcessor());
    vecProcess.push_back(new McpProcessor());
    vecProcess.push_back(new DssdProcessor());
    vecProcess.push_back(new MtcProcessor());
    // vecProcess.push_back(new PulserProcessor());
    // vecProcess.push_back(new VandleProcessor());
    
#ifdef useroot 
    // and finally the root processor
    vecProcess.push_back(new RootProcessor("tree.root", "tree"));
#endif
}

/*!
  detector driver deconstructor

  frees memory for all event processors
 */
DetectorDriver::~DetectorDriver()
{
    for (vector<EventProcessor *>::iterator it = vecProcess.begin();
	 it != vecProcess.end(); it++) {
	delete *it;
    }

    vecProcess.clear();

    for (vector<TraceAnalyzer *>::iterator it = vecAnalyzer.begin();
	 it != vecAnalyzer.end(); it++) {
	delete *it;
    }

    vecAnalyzer.clear();
}

const set<string>& DetectorDriver::GetUsedDetectors() const
{
    extern DetectorLibrary modChan;

    return modChan.GetUsedDetectors();
}

/*!
  Called from PixieStd.cpp during initialization.
  The calibration file cal.txt is read using the function ReadCal() and 
  checked to make sure that all channels have a calibration.
*/

int DetectorDriver::Init(void)
{
    // initialize the trace analysis routine
    for (vector<TraceAnalyzer *>::iterator it = vecAnalyzer.begin();
	 it != vecAnalyzer.end(); it++) {
	(*it)->Init();
	(*it)->SetLevel(20); //! Plot traces
    }

    // initialize processors in the event processing vector
    for (vector<EventProcessor *>::iterator it = vecProcess.begin();
	 it != vecProcess.end(); it++) {
	(*it)->Init(*this);	
    }

    /*
      Read in the calibration parameters from the file cal.txt
    */
    //cout << "read in the calibration parameters" << endl;
    ReadCal();

    rawev.GetCorrelator().Init();

    return 0;
}

/*!
  \brief controls event processing

  The ProcessEvent() function is called from ScanList() in PixieStd.cpp
  after an event has been constructed. This function is passed the mode
  the analysis is currently in (the options are either "scan" or
  "standaloneroot").  The function checks the thresholds for the individual
  channels in the event and calibrates their energies. 
  The raw and calibrated energies are plotted if the appropriate DAMM spectra
  have been created.  Then experiment specific processing is performed.  
  Currently, both RMS and MTC processing is available.  After all processing
  has occured, appropriate plotting routines are called.
*/
int DetectorDriver::ProcessEvent(const string &mode){   
    /*
      Begin the event processing looping over all the channels
      that fired in this particular event.
    */
    plot(dammIds::misc::D_NUMBER_OF_EVENTS, GENERIC_CHANNEL);
    
    const vector<ChanEvent *> &eventList = rawev.GetEventList();
    for(size_t i=0; i < eventList.size(); i++) {
	ChanEvent *chan = eventList[i];  

        PlotRaw(chan);
	ThreshAndCal(chan); // check threshold and calibrate
	PlotCal(chan);       
    } //end chan by chan event processing
 
    // have each processor in the event processing vector handle the event
    for (vector<EventProcessor *>::iterator iProc = vecProcess.begin();
	 iProc != vecProcess.end(); iProc++) {
	if ( (*iProc)->HasEvent() ) {
	     (*iProc)->Process(rawev);
	}
    }

    return 0;   
}

// declare plots for all the event processors
void DetectorDriver::DeclarePlots(void) const
{
    for (vector<TraceAnalyzer *>::const_iterator it = vecAnalyzer.begin();
	 it != vecAnalyzer.end(); it++) {
	(*it)->DeclarePlots();
    }

    for (vector<EventProcessor *>::const_iterator it = vecProcess.begin();
	 it != vecProcess.end(); it++) {
	(*it)->DeclarePlots();
    }
    
    rawev.GetCorrelator().DeclarePlots();
}

// sanity check for all our expectations
bool DetectorDriver::SanityCheck(void) const
{
    return true;
}

/*!
  \brief check threshold and calibrate each channel.

  Check the thresholds and calibrate the energy for each channel using the
  calibrations contained in the calibration vector filled during ReadCal()
*/

int DetectorDriver::ThreshAndCal(ChanEvent *chan)
{   
    // retrieve information about the channel
    Identifier chanId = chan->GetChanID();
    int id            = chan->GetID();
    string type       = chanId.GetType();
    string subtype    = chanId.GetSubtype();
    Trace &trace      = chan->GetTrace();

    double energy = 0.;

    if (type == "ignore" || type == "") {
	return 0;
    }
    /*
      If the channel has a trace get it, analyze it and set the energy.
    */
    if ( !trace.empty() ) {
        plot(dammIds::misc::D_HAS_TRACE,id);

	for (vector<TraceAnalyzer *>::iterator it = vecAnalyzer.begin();
	     it != vecAnalyzer.end(); it++) {	
	    (*it)->Analyze(trace, type, subtype);
	}

	if (trace.HasValue("filterEnergy") ) {     
	    if (trace.GetValue("filterEnergy") > 0) {
		energy = trace.GetValue("filterEnergy");
		plot(dammIds::misc::offsets::D_FILTER_ENERGY + id, energy);
	    } else {
		energy = 2;
	    }
	}
	if (trace.HasValue("calcEnergy") ) {	    
	    energy = trace.GetValue("calcEnergy");
	    chan->SetEnergy(energy);
	} else if (!trace.HasValue("filterEnergy")) {
	    energy = chan->GetEnergy() + randoms.Get();
	    energy /= ChanEvent::pixieEnergyContraction;
	}
	if (trace.HasValue("phase") ) {
	    double phase = trace.GetValue("phase");
	    chan->SetHighResTime( phase * pixie::adcClockInSeconds + 
				  chan->GetTrigTime() * pixie::filterClockInSeconds);
	}
    } else {
	// otherwise, use the Pixie on-board calculated energy
	// add a random number to convert an integer value to a 
	//   uniformly distributed floating point

	energy = chan->GetEnergy() + randoms.Get();
	energy /= ChanEvent::pixieEnergyContraction;
    }
    /*
      Set the calibrated energy for this channel
    */
    chan->SetCalEnergy( cal[id].Calibrate(energy) );

    /*
      update the detector summary
    */    
    rawev.GetSummary(type)->AddEvent(chan);
    DetectorSummary *summary = rawev.GetSummary(type + ':' + subtype, false);
    if (summary != NULL)
	summary->AddEvent(chan);

    return 1;
}

/*!
  Plot the raw energies of each channel into the damm spectrum number assigned
  to it in the map file with an offset as defined in damm_plotids.h
*/
int DetectorDriver::PlotRaw(const ChanEvent *chan) const
{
    int id = chan->GetID();
    float energy = chan->GetEnergy() / ChanEvent::pixieEnergyContraction;
    
    plot(dammIds::misc::offsets::D_RAW_ENERGY + id, energy);
    
    return 0;
}

/*!
  Plot the calibrated energies of each channel into the damm spectrum number
  assigned to it in the map file with an offset as defined in damm_plotids.h
*/
int DetectorDriver::PlotCal(const ChanEvent *chan) const
{
    int id = chan->GetID();
    // int dammid = chan->GetChanID().GetDammID();
    float calEnergy = chan->GetCalEnergy();
    
    plot(dammIds::misc::offsets::D_CAL_ENERGY + id, calEnergy);
    if (!chan->IsSaturated() && !chan->IsPileup())
	plot(dammIds::misc::offsets::D_CAL_ENERGY_REJECT + id, calEnergy);
    return 0;
}

vector<EventProcessor *> DetectorDriver::GetProcessors(const string& type) const
{
  vector<EventProcessor *> retVec;

  for (vector<EventProcessor *>::const_iterator it = vecProcess.begin();
       it != vecProcess.end(); it++) {
    if ( (*it)->GetTypes().count(type) > 0 )
      retVec.push_back(*it);
  }

  return retVec;
}

/*!
  Read in the calibration for each channel according to the data in cal.txt
*/
void DetectorDriver::ReadCal()
{
    /*
      The file cal.txt contains the calibration for each channel.  The first
      five variables describe the detector's physical location (strip number,
      detector number, ...), the detector type, the detector subtype, the number
      of calibrations, and their polynomial order.  Using this information, the
      rest of a channel's calibration is read in as -- lower threshold for the 
      current calibration, followed by the polynomial constants in increasing
      polynomial order.  The lower thresholds and polynomial values are read in
      for each distinct calibration specified by the number of calibrations.
    */

    // lookup table for information from map.txt (from PixieStd.cpp)
    extern DetectorLibrary modChan;
    Identifier lookupID;

    /*
      Values used to read in the thresholds and polynomials from cal.txt
      The numbers can not be read directly into the vectors
    */
    float thresh;
    float val;

    /*
      The channels module number, channel number, detector location
      the number of calibrations, polynomial order, if the detector
      should be ignored, the detector type and subtype.
    */
    unsigned int detLocation;
    string detType, detSubtype;

    const string calFilename("cal.txt");

    ifstream calFile(calFilename.c_str());

    // make sure there is a generic calibration for each channel in the map
    cal.resize(modChan.size());

    if (!calFile) {
      cout << "Can not open file " << calFilename << endl;
    } else {
      cout << "reading in calibrations from " << calFilename << endl;
      while (calFile) {
            /*
              While the end of the calibration file has not been reached,
              increment the number of lines read and if the first input on a
	      line is a number, read in the first five parameters for a channel
            */
            if ( isdigit(calFile.peek()) ) {
                 calFile >> detLocation >> detType >> detSubtype;
		lookupID.SetLocation(detLocation);
		lookupID.SetType(detType);
		lookupID.SetSubtype(detSubtype);
		// find the identifier in the map
		DetectorLibrary::iterator mapIt = 
		    find(modChan.begin(), modChan.end(), lookupID); 
		if (mapIt == modChan.end()) {
		    cout << "Can not match detector type " << detType
			 << " and subtype " << detSubtype 
			 << " with location " << detLocation
			 << " to a channel in the map." << endl;
		    exit(EXIT_FAILURE);
		}
		size_t id = distance(modChan.begin(), mapIt);
		Calibration &detCal = cal.at(id);
		//? make this a member function of Calibration
		detCal.id = id;
		calFile	>> detCal.numCal >> detCal.polyOrder;
		detCal.thresh.clear();
		detCal.val.clear();
		detCal.detLocation = detLocation;
		detCal.detType = detType;
		detCal.detSubtype = detSubtype;
                /*
                  For the number of calibrations read in the
                  thresholds and the polynomial values
                */
                for (unsigned int i = 0; i < detCal.numCal; i++) {
                    calFile >> thresh;
                    detCal.thresh.push_back(thresh);

                    for(unsigned int j = 0; j < detCal.polyOrder+1; j++){
                        /*
                          For the calibration order, read in the polynomial 
                          constants in ascending order
                        */
                        calFile >> val;
			detCal.val.push_back(val);
                    } // finish looping on polynomial order
                } // finish looping on number of calibrations

                /*
                  Add the value of MAX_PAR from the param.h file
                  as the upper limit of all calibrations
                */
                detCal.thresh.push_back(MAX_PAR);
            } else {
                // this is a comment, skip line 
                calFile.ignore(1000,'\n');
            }            
        } // end while (!calFile) loop - end reading cal.txt file
    }
    calFile.close();

    // check to make sure every channel has a calibration, no default
    //   calibration is allowed
    DetectorLibrary::const_iterator mapIt = modChan.begin();
    vector<Calibration>::iterator calIt = cal.begin();
    for (;mapIt != modChan.end(); mapIt++, calIt++) {
	string type = mapIt->GetType();
	if (type == "ignore" || type == "") {
	    continue;
	};
	if (calIt->detType!= type) {
	    if (mapIt->HasTag("uncal")) {
		// set the remaining fields properly
		calIt->detType     = type;
		calIt->detSubtype  = mapIt->GetSubtype();
		calIt->detLocation = mapIt->GetLocation(); 
		continue;
	    }
	    cout << "Uncalibrated detector found for type " << type
		 << " at location " << mapIt->GetLocation() 
		 << ". No default calibration is given, please correct." 
		 << endl;
	    exit(EXIT_FAILURE);
	}
    }
    /*
      Print the calibration values that have been read in
    */
    //cout << "calibration parameters are: " << cal.size() << endl;
    
    cout << setw(4)  << "mod" 
         << setw(4)  << "ch"
	 << setw(4)  << "loc"
	 << setw(10) << "type"
         << setw(8)  << "subtype"
	 << setw(5)  << "cals"
	 << setw(6)  << "order"
	 << setw(31) << "cal values: low-high thresh, coeffs" << endl;
 
    //? calibration print command?
    for(size_t a = 0; a < cal.size(); a++){
      cout << setw(4)  << int(a/16) 
	   << setw(4)  << (a % 16)
	   << setw(4)  << cal[a].detLocation 
	   << setw(10) << cal[a].detType
           << setw(8)  << cal[a].detSubtype 
	   << setw(5)  << cal[a].numCal
           << setw(6)  << cal[a].polyOrder;      
        for(unsigned int b = 0; b < cal[a].numCal; b++){
	    cout << setw(6) << cal[a].thresh[b];
            cout << " - " << setw(6) << cal[a].thresh[b+1];
            for(unsigned int c = 0; c < cal[a].polyOrder+1; c++){
	      cout << setw(7) << setprecision(5) 
		   << cal[a].val[b*(cal[a].polyOrder+1)+c];
            }

        }
        
        cout << endl;
    }
}

/*!
  Construct calibration parameters using Zero() method
*/
Calibration::Calibration() : 
    id(-1), detType(""), detSubtype(""), detLocation(-1),
    numCal(1), polyOrder(1)
{
    thresh.push_back(0);
    thresh.push_back(MAX_PAR);
    // simple linear calibration
    val.push_back(0); // constant coeff
    val.push_back(1); // coeff linear in raw energy
}

double Calibration::Calibrate(double raw)
{
    /*
      Make sure we don't have any calibration values below the lowest
      calibration theshold or any calibrated energies above the 
      maximum threshold value set in cal.txt
    */
    if(raw < thresh[0]) {
        return 0;
    } 

    if(raw >= thresh[numCal]) {
	return thresh[numCal] - 1;
    }

    double calVal = 0;
    /*
      Begin threshold check and calibration, first
      loop over the number of calibrations
    */
    for(unsigned int a = 0; a < numCal; a++) {
        //check to see if energy falls in this calibration range
        if (raw >= thresh[a] && raw < thresh[a+1]) {
            //loop over the polynomial order
            for(unsigned int b = 0; b < polyOrder+1; b++) {
		calVal += pow(raw,(double)b) * val[a*(polyOrder+1) + b];
            }
	    break;
        }
    }

    return calVal;
}

/*!
  This function is called from the scan program
  when scan is either killed or ended.  If
  ROOT has been enabled, close the ROOT files.
  If ROOT is not enabled do nothing.
*/
extern "C" void detectorend_()
{
    //cout << "ending, no rootfile " << endl;       
}

