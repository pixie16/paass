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
#include <sstream>

#include "pugixml.hpp"
#include "PathHolder.hpp"
#include "Exceptions.hpp"
#include "DetectorDriver.hpp"
#include "DetectorLibrary.hpp"
#include "MapFile.hpp"
#include "RandomPool.hpp"
#include "RawEvent.hpp"
#include "TimingInformation.hpp"
#include "TreeCorrelator.hpp"

#include "DammPlotIds.hpp"

#include "DssdProcessor.hpp"
#include "Hen3Processor.hpp"
#include "GeProcessor.hpp"
#include "GeCalibProcessor.hpp"
#include "Ge4Hen3Processor.hpp"
#include "ImplantSsdProcessor.hpp"
#include "IonChamberProcessor.hpp"
#include "McpProcessor.hpp"
#include "MtcProcessor.hpp"
#include "PositionProcessor.hpp"
#include "PulserProcessor.hpp"
#include "BetaScintProcessor.hpp"
#include "NeutronScintProcessor.hpp"
#include "LiquidScintProcessor.hpp"
#include "SsdProcessor.hpp"
#include "TraceFilterer.hpp"
#include "TriggerLogicProcessor.hpp"
#include "VandleProcessor.hpp"

#include "CfdAnalyzer.hpp"
#include "DoubleTraceAnalyzer.hpp"
#include "FittingAnalyzer.hpp"
#include "TauAnalyzer.hpp"
#include "TraceAnalyzer.hpp"
#include "TraceExtracter.hpp"
#include "WaveformAnalyzer.hpp"

#ifdef useroot
#include "RootProcessor.hpp"
#include "ScintRoot.hpp"
#include "VandleRoot.hpp"
#endif

using namespace std;
using namespace dammIds::raw;

/*!
  detector driver constructor

  Creates instances of all event processors
*/

using namespace dammIds::raw;

DetectorDriver* DetectorDriver::instance = NULL;

/** Instance is created upon first call */
DetectorDriver* DetectorDriver::get() {
    if (!instance) {
        instance = new DetectorDriver();
    }
    return instance;
}

DetectorDriver::DetectorDriver() : 
    histo(OFFSET, RANGE) 
{
    Messenger m;
    try {
        m.start("Loading Processors");
        LoadProcessors(m);
    } catch (GeneralException &e) {
        // Any exception in registering plots in Processors 
        // and possible other exceptions in creating Processors
        // will be intercepted here
        m.fail();
        cout << "Exception caught at DetectorDriver::DetectorDriver" << endl;
        cout << "\t" << e.what() << endl;
        exit(EXIT_FAILURE);
    } catch (GeneralWarning &w) {
        cout << "Warning found at DetectorDriver::DetectorDriver" << endl;
        cout << "\t" << w.what() << endl;
    }
    m.done();
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

void DetectorDriver::LoadProcessors(Messenger& m) {
    pugi::xml_document doc;

    PathHolder* conf_path = new PathHolder();
    string xmlFileName = conf_path->GetFullPath("Config.xml");
    delete conf_path;

    pugi::xml_parse_result result = doc.load_file(xmlFileName.c_str());
    if (!result) {
        stringstream ss;
        ss << "DetectorDriver: error parsing file " << xmlFileName;
        ss << " : " << result.description();
        throw IOException(ss.str());
    }

    pugi::xml_node driver = doc.child("Configuration").child("DetectorDriver");
    for (pugi::xml_node processor = driver.child("Processor"); processor;
         processor = processor.next_sibling("Processor")) {
        string name = processor.attribute("name").value();

        if (name == "BetaScintProcessor") {
            vecProcess.push_back(new BetaScintProcessor());
        } else if (name == "DssdProcessor") {
            vecProcess.push_back(new DssdProcessor());
        } else if (name == "GeProcessor") {
            vecProcess.push_back(new GeProcessor()); 
        } else if (name == "Ge4Hen3Processor") {
            vecProcess.push_back(new Ge4Hen3Processor()); 
        } else if (name == "GeCalibProcessor") {
            vecProcess.push_back(new GeCalibProcessor()); 
        } else if (name == "Hen3Processor") {
            vecProcess.push_back(new Hen3Processor()); 
        } else if (name == "ImplantSsdProcessor") {
            vecProcess.push_back(new ImplantSsdProcessor()); 
        } else if (name == "IonChamberProcessor") {
            vecProcess.push_back(new IonChamberProcessor()); 
        } else if (name == "LiquidScintProcessor") {
            vecProcess.push_back(new LiquidScintProcessor());
        } else if (name == "LogicProcessor") {
            vecProcess.push_back(new LogicProcessor()); 
        } else if (name == "McpProcessor") {
            vecProcess.push_back(new McpProcessor()); 
        } else if (name == "MtcProcessor") {
            vecProcess.push_back(new MtcProcessor());
        } else if (name == "NeutronScintProcessor") {
            vecProcess.push_back(new NeutronScintProcessor());
        } else if (name == "PositionProcessor") {
            vecProcess.push_back(new PositionProcessor());
        } else if (name == "PulserProcessor") {
            vecProcess.push_back(new PulserProcessor());
        } else if (name == "SsdProcessor") {
            vecProcess.push_back(new SsdProcessor());
        } else if (name == "TriggerLogicProcessor") {
            vecProcess.push_back(new TriggerLogicProcessor());
        } else if (name == "VandleProcessor") {
            vecProcess.push_back(new VandleProcessor());
        }
#ifdef useroot
        else if (name == "RootProcessor") {
            vecProcess.push_back(new RootProcessor("tree.root", "tree"));
        } else if (name == "ScintROOT") {
            vecProcess.push_back(new ScintROOT());
        } else if (name == "VandleROOT") {
            vecProcess.push_back(new VandleROOT());
        }
#endif
        else {
            stringstream ss;
            ss << "DetectorDriver: unknown processor type" << name;
            throw GeneralException(ss.str());
        }
        m.detail(name + " loaded");
    }

    for (pugi::xml_node analyzer = driver.child("Analyzer"); analyzer;
         analyzer = analyzer.next_sibling("Analyzer")) {
        string name = analyzer.attribute("name").value();

        vecAnalyzer.push_back(new WaveformAnalyzer());
        if (name == "DoubleTraceAnalyzer") {
            vecAnalyzer.push_back(new DoubleTraceAnalyzer());
        } else if (name == "TauAnalyzer") {
            vecAnalyzer.push_back(new TauAnalyzer());
        } else if (name == "TracePlotter") {
            vecAnalyzer.push_back(new TracePlotter());
        } else if (name == "TraceExtracter") {
            vecAnalyzer.push_back(new TraceExtracter("ssd", "top"));
        }
#if defined(pulsefit) || defined(dcfd)
        else if (name == "WaveformAnalyzer") {
            vecAnalyzer.push_back(new WaveformAnalyzer());
        }
#endif
#ifdef pulsefit
        else if (name == "FittingAnalyzer") {
            vecAnalyzer.push_back(new FittingAnalyzer());
        }
#elif dcfd
        else if (name == "CfdAnalyzer") {
            vecAnalyzer.push_back(new CfdAnalyzer());
        }
#endif
        else {
            stringstream ss;
            ss << "DetectorDriver: unknown analyzer type" << name;
            throw GeneralException(ss.str());
        }
        m.detail(name + " loaded");
    }
}

/*!
  Called from PixieStd.cpp during initialization.
  The calibration file cal.txt is read using the function ReadCal() and 
  checked to make sure that all channels have a calibration.
*/

int DetectorDriver::Init(RawEvent& rawev)
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
        (*it)->Init(rawev);	
    }

    /*
      Read in the calibration parameters from the file cal.txt
    */
    //cout << "read in the calibration parameters" << endl;
    ReadCal();
    ReadWalk();

    TimingInformation readFiles;
    readFiles.ReadTimingConstants();
    readFiles.ReadTimingCalibration();

    rawev.GetCorrelator().Init(rawev);

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
int DetectorDriver::ProcessEvent(const string &mode, RawEvent& rawev){   
    /*
      Begin the event processing looping over all the channels
      that fired in this particular event.
    */
    plot(dammIds::raw::D_NUMBER_OF_EVENTS, dammIds::GENERIC_CHANNEL);
    
    try {
        for (vector<ChanEvent*>::const_iterator it = rawev.GetEventList().begin();
            it != rawev.GetEventList().end(); ++it) {
            string place = (*it)->GetChanID().GetPlaceName();

            // skip empty channel
            if (place == "__-1")
                continue;

            // check threshold and calibrate
            PlotRaw((*it));
            ThreshAndCal((*it), rawev);
            PlotCal((*it));

            // Do not activate places if saturated or pileup
            if ( (*it)->IsSaturated() || (*it)->IsPileup() )
                continue;

            double time = (*it)->GetTime();
            double energy = (*it)->GetCalEnergy();
            int location = (*it)->GetChanID().GetLocation();

            EventData data(time, energy, location);
            TreeCorrelator::get()->place(place)->activate(data);
        } 
    
        // have each processor in the event processing vector handle the event
        /* First round is preprocessing, where process result must be guaranteed
        * to not to be dependent on results of other Processors. */
        for (vector<EventProcessor*>::iterator iProc = vecProcess.begin();
        iProc != vecProcess.end(); iProc++) {
            if ( (*iProc)->HasEvent() ) {
                (*iProc)->PreProcess(rawev);
            }
        }
        /* In the second round the Process is called, which may depend on other
        * Processors. */
        for (vector<EventProcessor *>::iterator iProc = vecProcess.begin();
        iProc != vecProcess.end(); iProc++) {
            if ( (*iProc)->HasEvent() ) {
                (*iProc)->Process(rawev);
            }
        }
    } catch (GeneralException &e) {
        // Any exception in activation of basic places, PreProcess and Process
        // will be intercepted here
        cout << "Exception caught at DetectorDriver::ProcessEvent" << endl;
        cout << "\t" << e.what() << endl;
        exit(EXIT_FAILURE);
    } catch (GeneralWarning &w) {
        cout << "Warning caught at DetectorDriver::ProcessEvent" << endl;
        cout << "\t" << w.what() << endl;
    }

    return 0;   
}

// declare plots for all the event processors
void DetectorDriver::DeclarePlots(MapFile& theMapFile)
{
    try {
        for (vector<TraceAnalyzer *>::const_iterator it = vecAnalyzer.begin();
        it != vecAnalyzer.end(); it++) {
            (*it)->DeclarePlots();
        }

        for (vector<EventProcessor *>::const_iterator it = vecProcess.begin();
        it != vecProcess.end(); it++) {
            (*it)->DeclarePlots();
        }
        
        // Declare plots for each channel
        DetectorLibrary* modChan = DetectorLibrary::get();

        DeclareHistogram1D(D_HIT_SPECTRUM, S7, "channel hit spectrum");
        DeclareHistogram1D(D_SUBEVENT_GAP, SE, "time btwn chan-in event,10ns bin");
        DeclareHistogram1D(D_EVENT_LENGTH, SE, "time length of event, 10 ns bin");
        DeclareHistogram1D(D_EVENT_GAP, SE, "time between events, 10 ns bin");
        DeclareHistogram1D(D_EVENT_MULTIPLICITY, S7, "number of channels in event");
        DeclareHistogram1D(D_BUFFER_END_TIME, SE, "length of buffer, 1 ms bin");
        DeclareHistogram2D(DD_RUNTIME_SEC, SE, S6, "run time - s");
        DeclareHistogram2D(DD_DEAD_TIME_CUMUL, SE, S6, "dead time - cumul");
        DeclareHistogram2D(DD_BUFFER_START_TIME, SE, S6, "dead time - 0.1%");
        DeclareHistogram2D(DD_RUNTIME_MSEC, SE, S7, "run time - ms");
        DeclareHistogram1D(D_NUMBER_OF_EVENTS, S4, "event counter");

        DetectorLibrary::size_type maxChan = (theMapFile ? modChan->size() : 192);

        for (DetectorLibrary::size_type i = 0; i < maxChan; i++) {	 
            if (theMapFile && !modChan->HasValue(i)) {
                continue;
            }
            stringstream idstr; 
            
            if (theMapFile) {
                const Identifier &id = modChan->at(i);

                idstr << "M" << modChan->ModuleFromIndex(i)
                << " C" << modChan->ChannelFromIndex(i)
                << " - " << id.GetType()
                << ":" << id.GetSubtype()
                << " L" << id.GetLocation();
            } else {
                idstr << "id " << i;
            }
            DeclareHistogram1D(D_RAW_ENERGY + i, SE, ("RawE " + idstr.str()).c_str() );
            DeclareHistogram1D(D_FILTER_ENERGY + i, SE, ("FilterE " + idstr.str()).c_str() );
            DeclareHistogram1D(D_SCALAR + i, SE, ("Scalar " + idstr.str()).c_str() );
#if !defined(REVD) && !defined(REVF)
            DeclareHistogram1D(D_TIME + i, SE, ("Time " + idstr.str()).c_str() ); 
#endif
            DeclareHistogram1D(D_CAL_ENERGY + i, SE, ("CalE " + idstr.str()).c_str() );
            DeclareHistogram1D(D_CAL_ENERGY_REJECT + i, SE, ("CalE NoSat " + idstr.str()).c_str() );
        }
        DeclareHistogram1D(D_HAS_TRACE, S7, "channels with traces");
    } catch (exception &e) {
        // Any exception in histogram declaration will be intercepted here
        cout << "Exception caught at DetectorDriver::DeclareHistograms" << endl;
        cout << "\t" << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

// sanity check for all our expectations
void DetectorDriver::SanityCheck(void) const
{
    /** Use Exceptions to throw an exception here if sanity check was 
     * not succesful */
}

/*!
  \brief check threshold and calibrate each channel.

  Check the thresholds and calibrate the energy for each channel using the
  calibrations contained in the calibration vector filled during ReadCal()
*/

int DetectorDriver::ThreshAndCal(ChanEvent *chan, RawEvent& rawev)
{   
    // retrieve information about the channel
    Identifier chanId = chan->GetChanID();
    int id            = chan->GetID();
    string type       = chanId.GetType();
    string subtype    = chanId.GetSubtype();
    bool hasStartTag  = chanId.HasTag("start");
    Trace &trace      = chan->GetTrace();

    RandomPool* randoms = RandomPool::get();

    double energy = 0.;

    if (type == "ignore" || type == "") {
        return 0;
    }
    /*
      If the channel has a trace get it, analyze it and set the energy.
    */
    if ( !trace.empty() ) {
        plot(D_HAS_TRACE, id);

	for (vector<TraceAnalyzer *>::iterator it = vecAnalyzer.begin();
	     it != vecAnalyzer.end(); it++) {	
            (*it)->Analyze(trace, type, subtype);
	}

	if (trace.HasValue("filterEnergy") ) {     
	    if (trace.GetValue("filterEnergy") > 0) {
            energy = trace.GetValue("filterEnergy");
            plot(D_FILTER_ENERGY + id, energy);
	    } else {
            energy = 2;
	    }
	}
	if (trace.HasValue("calcEnergy") ) {	    
	    energy = trace.GetValue("calcEnergy");
	    chan->SetEnergy(energy);
	} else if (!trace.HasValue("filterEnergy")) {
	    energy = chan->GetEnergy() + randoms->Get();
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

	energy = chan->GetEnergy() + randoms->Get();
	energy /= ChanEvent::pixieEnergyContraction;
    }
    /*
      Set the calibrated energy for this channel
    */
    chan->SetCalEnergy(cal[id].Calibrate(energy));

    /** Apply the walk correction. */
    double time = chan->GetTime();
    double ch = chan->GetEnergy();
    double walk_correction = walk.GetCorrection(chanId, ch);
    chan->SetCorrectedTime(time - walk_correction);
    
    /*
      update the detector summary
    */    
    rawev.GetSummary(type)->AddEvent(chan);
    DetectorSummary *summary;
    
    summary = rawev.GetSummary(type + ':' + subtype, false);
    if (summary != NULL)
	summary->AddEvent(chan);

    if(hasStartTag) {
	summary = 
	    rawev.GetSummary(type + ':' + subtype + ':' + "start", false);
	if (summary != NULL)
	    summary->AddEvent(chan);
    }
    
    return 1;
}

/*!
  Plot the raw energies of each channel into the damm spectrum number assigned
  to it in the map file with an offset as defined in DammPlotIds.hpp
*/
int DetectorDriver::PlotRaw(const ChanEvent *chan)
{
    int id = chan->GetID();
    float energy = chan->GetEnergy() / ChanEvent::pixieEnergyContraction;
    
    plot(D_RAW_ENERGY + id, energy);
    
    return 0;
}

/*!
  Plot the calibrated energies of each channel into the damm spectrum number
  assigned to it in the map file with an offset as defined in DammPlotIds.hpp
*/
int DetectorDriver::PlotCal(const ChanEvent *chan)
{
    int id = chan->GetID();
    // int dammid = chan->GetChanID().GetDammID();
    float calEnergy = chan->GetCalEnergy();
    
    plot(D_CAL_ENERGY + id, calEnergy);
    if (!chan->IsSaturated() && !chan->IsPileup())
        plot(D_CAL_ENERGY_REJECT + id, calEnergy);
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
    DetectorLibrary* modChan = DetectorLibrary::get();
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

    PathHolder* conf_path = new PathHolder();
    string calFilename = conf_path->GetFullPath("cal.txt");
    delete conf_path;

    ifstream calFile(calFilename.c_str());

    // make sure there is a generic calibration for each channel in the map
    cal.resize(modChan->size());

    if (!calFile) {
        throw IOException("Could not open file " + calFilename);
    } else {
      cout << "Reading in calibrations from " << calFilename << endl;
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
		    find(modChan->begin(), modChan->end(), lookupID); 
		if (mapIt == modChan->end()) {
		    cout << "Can not match detector type " << detType
			 << " and subtype " << detSubtype 
			 << " with location " << detLocation
			 << " to a channel in the map." << endl;
		    exit(EXIT_FAILURE);
		}
		size_t id = distance(modChan->begin(), mapIt);
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
                  Add the value of MAX_PAR from the Globals.hpp file
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
    DetectorLibrary::const_iterator mapIt = modChan->begin();
    vector<Calibration>::iterator calIt = cal.begin();
    for (;mapIt != modChan->end(); mapIt++, calIt++) {
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
   
    if (verbose::CALIBRATION_INIT) {
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
}

void DetectorDriver::ReadWalk() {
    pugi::xml_document doc;

    PathHolder* conf_path = new PathHolder();
    string xmlFileName = conf_path->GetFullPath("Config.xml");
    delete conf_path;

    pugi::xml_parse_result result = doc.load_file(xmlFileName.c_str());
    if (!result) {
        stringstream ss;
        ss << "DetectorDriver::ReadWalk: error parsing file " << xmlFileName;
        ss << " : " << result.description();
        cout << ss.str() << endl;
    }

    Messenger m;
    m.detail("Loading Walk Corrections");

    pugi::xml_node map = doc.child("Configuration").child("Map");
    bool verbose = map.attribute("verbose").as_bool();
    for (pugi::xml_node module = map.child("Module"); module;
         module = module.next_sibling("Module")) {
        int module_number = module.attribute("number").as_int();
        for (pugi::xml_node channel = module.child("Channel"); channel;
             channel = channel.next_sibling("Channel")) {
            int ch_number = channel.attribute("number").as_int(-1);
            if (ch_number < 0) {
                stringstream ss;
                ss << "DetectorDriver::ReadWalk: Illegal channel number "
                   << "found " << ch_number << " in cofiguration file.";
                throw GeneralException(ss.str());
            }
            Identifier chanID = DetectorLibrary::get()->at(module_number,
                                                           ch_number);
            for (pugi::xml_node walkcorr = channel.child("WalkCorrection");
                walkcorr; walkcorr = walkcorr.next_sibling("WalkCorrection")) {
                string model = walkcorr.attribute("model").as_string("None");
                stringstream pars(walkcorr.text().as_string());
                vector<double> parameters;
                while (true) {
                    double p;
                    pars >> p;
                    if (pars) 
                        parameters.push_back(p);
                    else
                        break;
                }
                if (verbose) {
                    stringstream ss;
                    ss << "Mod" << module_number << " Ch" << ch_number << ": ";
                    ss << "Model: " << model;
                    for (vector<double>::iterator it = parameters.begin();
                         it != parameters.end(); ++it)
                        ss << " " << (*it);
                    m.detail(ss.str());
                }
                walk.AddChannel(chanID, model, parameters);
            }
        }
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
        if (raw >= thresh[a] && raw < thresh[a+1]) {
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

