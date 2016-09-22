/*! \file DetectorDriver.cpp
 *   \brief Main driver for event processing
 * \author S. N. Liddick
 * \date July 2, 2007
*/
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>

#include "pugixml.hpp"

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "DetectorLibrary.hpp"
#include "Exceptions.hpp"
#include "HighResTimingData.hpp"
#include "RandomPool.hpp"
#include "RawEvent.hpp"
#include "TreeCorrelator.hpp"

#include "BetaScintProcessor.hpp"
#include "DoubleBetaProcessor.hpp"
#include "Hen3Processor.hpp"
#include "GeProcessor.hpp"
#include "GeCalibProcessor.hpp"
#include "IonChamberProcessor.hpp"
#include "LiquidScintProcessor.hpp"
#include "LogicProcessor.hpp"
#include "McpProcessor.hpp"
#include "NeutronScintProcessor.hpp"
#include "PositionProcessor.hpp"
#include "PspmtProcessor.hpp"
#include "SsdProcessor.hpp"
#include "TeenyVandleProcessor.hpp"
#include "TemplateProcessor.hpp"
#include "VandleProcessor.hpp"
#include "ValidProcessor.hpp"

#include "CfdAnalyzer.hpp"
#include "FittingAnalyzer.hpp"
#include "TauAnalyzer.hpp"
#include "TraceAnalyzer.hpp"
#include "TraceExtractor.hpp"
#include "TraceFilterAnalyzer.hpp"
#include "WaaAnalyzer.hpp"
#include "WaveformAnalyzer.hpp"

#include "TemplateExpProcessor.hpp"

#ifdef useroot
#include "RootProcessor.hpp"
#endif

using namespace std;
using namespace dammIds::raw;

DetectorDriver* DetectorDriver::instance = NULL;

DetectorDriver* DetectorDriver::get() {
    if (!instance)
        instance = new DetectorDriver();
    return instance;
}

DetectorDriver::DetectorDriver() : histo(OFFSET, RANGE, "DetectorDriver") {
    cfg_ = Globals::get()->configfile();
    Messenger m;
    try {
        m.start("Loading Processors");
        LoadProcessors(m);
    } catch (GeneralException &e) {
        /// Any exception in registering plots in Processors
        /// and possible other exceptions in creating Processors
        /// will be intercepted here
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

DetectorDriver::~DetectorDriver() {
    for (vector<EventProcessor *>::iterator it = vecProcess.begin();
	 it != vecProcess.end(); it++)
        delete(*it);
    vecProcess.clear();

    for (vector<TraceAnalyzer *>::iterator it = vecAnalyzer.begin();
	 it != vecAnalyzer.end(); it++)
        delete(*it);
    vecAnalyzer.clear();
    instance = NULL;
}

void DetectorDriver::LoadProcessors(Messenger& m) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(cfg_.c_str());
    if (!result) {
        stringstream ss;
        ss << "DetectorDriver: error parsing file " << cfg_;
        ss << " : " << result.description();
        throw IOException(ss.str());
    }

    DetectorLibrary::get();

    pugi::xml_node driver = doc.child("Configuration").child("DetectorDriver");
    for (pugi::xml_node processor = driver.child("Processor"); processor;
        processor = processor.next_sibling("Processor")) {
        string name = processor.attribute("name").value();

        m.detail("Loading " + name);
        if (name == "BetaScintProcessor") {
            double gamma_beta_limit =
                processor.attribute("gamma_beta_limit").as_double(200.e-9);
            if (gamma_beta_limit == 200.e-9)
                m.warning("Using default gamme_beta_limit = 200e-9", 1);
            double energy_contraction =
                processor.attribute("energy_contraction").as_double(1.0);
            if (energy_contraction == 1)
                m.warning("Using default energy contraction = 1", 1);
            vecProcess.push_back(new BetaScintProcessor(gamma_beta_limit,
                                                        energy_contraction));
        } else if (name == "GeProcessor") {
            double gamma_threshold =
                processor.attribute("gamma_threshold").as_double(1.0);
            if (gamma_threshold == 1.0)
                m.warning("Using default gamma_threshold = 1.0", 1);
            double low_ratio =
                processor.attribute("low_ratio").as_double(1.0);
            if (low_ratio == 1.0)
                m.warning("Using default low_ratio = 1.0", 1);
            double high_ratio =
                processor.attribute("high_ratio").as_double(3.0);
            if (high_ratio == 3.0)
                m.warning("Using default high_ratio = 3.0", 1);
            double sub_event =
                processor.attribute("sub_event").as_double(100.e-9);
            if (sub_event == 100.e-9)
                m.warning("Using default sub_event = 100e-9", 1);
            double gamma_beta_limit =
                processor.attribute("gamma_beta_limit").as_double(200.e-9);
            if (gamma_beta_limit == 200.e-9)
                m.warning("Using default gamme_beta_limit = 200e-9", 1);
            double gamma_gamma_limit =
                processor.attribute("gamma_gamma_limit").as_double(200.e-9);
            if (gamma_gamma_limit == 200.e-9)
                m.warning("Using default gamma_gamma_limit = 200e-9", 1);
            double cycle_gate1_min =
                processor.attribute("cycle_gate1_min").as_double(0.0);
            if (cycle_gate1_min == 0.0)
                m.warning("Using default cycle_gate1_min = 0.0", 1);
            double cycle_gate1_max =
                processor.attribute("cycle_gate1_max").as_double(0.0);
            if (cycle_gate1_max == 0.0)
                m.warning("Using default cycle_gate1_max = 0.0", 1);
            double cycle_gate2_min =
                processor.attribute("cycle_gate2_min").as_double(0.0);
            if (cycle_gate2_min == 0.0)
                m.warning("Using default cycle_gate2_min = 0.0", 1);
            double cycle_gate2_max =
                processor.attribute("cycle_gate2_max").as_double(0.0);
            if (cycle_gate2_max == 0.0)
                m.warning("Using default cycle_gate2_max = 0.0", 1);
            vecProcess.push_back(new GeProcessor(gamma_threshold, low_ratio,
                high_ratio, sub_event, gamma_beta_limit, gamma_gamma_limit,
                cycle_gate1_min, cycle_gate1_max, cycle_gate2_min,
                cycle_gate2_max));
        } else if (name == "GeCalibProcessor") {
            double gamma_threshold =
                processor.attribute("gamma_threshold").as_double(1);
            double low_ratio =
                processor.attribute("low_ratio").as_double(1);
            double high_ratio =
                processor.attribute("high_ratio").as_double(3);
            vecProcess.push_back(new GeCalibProcessor(gamma_threshold,
                low_ratio, high_ratio));
        } else if (name == "Hen3Processor") {
            vecProcess.push_back(new Hen3Processor());
        } else if (name == "IonChamberProcessor") {
            vecProcess.push_back(new IonChamberProcessor());
        } else if (name == "LiquidScintProcessor") {
            vecProcess.push_back(new LiquidScintProcessor());
        } else if (name == "LogicProcessor") {
            vecProcess.push_back(new LogicProcessor());
        } else if (name == "NeutronScintProcessor") {
            vecProcess.push_back(new NeutronScintProcessor());
        } else if (name == "PositionProcessor") {
            vecProcess.push_back(new PositionProcessor());
        } else if (name == "SsdProcessor") {
            vecProcess.push_back(new SsdProcessor());
        } else if (name == "VandleProcessor") {
            double res = processor.attribute("res").as_double(2.0);
            double offset = processor.attribute("offset").as_double(200.0);
            unsigned int numStarts = processor.attribute("NumStarts").as_int(2);
            vector<string> types =
                strings::tokenize(processor.attribute("types").as_string(),",");
            vecProcess.push_back(new VandleProcessor(types, res,
                offset, numStarts));
        } else if (name == "TeenyVandleProcessor") {
                vecProcess.push_back(new TeenyVandleProcessor());
        } else if (name == "DoubleBetaProcessor") {
            vecProcess.push_back(new DoubleBetaProcessor());
        } else if (name == "PspmtProcessor") {
                vecProcess.push_back(new PspmtProcessor());
        } else if (name == "TemplateProcessor") {
            vecProcess.push_back(new TemplateProcessor());
        } else if (name == "TemplateExpProcessor") {
            vecProcess.push_back(new TemplateExpProcessor());
	}
#ifdef useroot
        else if (name == "RootProcessor") {
            vecProcess.push_back(new RootProcessor("tree.root", "tree"));
        }
#endif
        else {
            stringstream ss;
            ss << "DetectorDriver: unknown processor type" << name;
            throw GeneralException(ss.str());
        }
        stringstream ss;
        for (pugi::xml_attribute_iterator ait = processor.attributes_begin();
            ait != processor.attributes_end(); ++ait) {
            ss.str("");
            ss << ait->name();
            if (ss.str().compare("name") != 0) {
                ss << " = " << ait->value();
                m.detail(ss.str(), 1);
            }
        }
    }

    for (pugi::xml_node analyzer = driver.child("Analyzer"); analyzer;
        analyzer = analyzer.next_sibling("Analyzer")) {
        string name = analyzer.attribute("name").value();
        m.detail("Loading " + name);

	if(name == "TraceFilterAnalyzer") {
	    bool findPileups = analyzer.attribute("FindPileup").as_bool(false);
	    vecAnalyzer.push_back(new TraceFilterAnalyzer(findPileups));
	} else if(name == "TauAnalyzer") {
            vecAnalyzer.push_back(new TauAnalyzer());
        } else if (name == "TraceExtractor") {
            string type = analyzer.attribute("type").as_string();
            string subtype = analyzer.attribute("subtype").as_string();
            string tag = analyzer.attribute("tag").as_string();
            vecAnalyzer.push_back(new TraceExtractor(type, subtype,tag));
        } else if (name == "WaveformAnalyzer") {
            vecAnalyzer.push_back(new WaveformAnalyzer());
        } else if (name == "CfdAnalyzer") {
            vecAnalyzer.push_back(new CfdAnalyzer());
        } else if (name == "WaaAnalyzer") {
            vecAnalyzer.push_back(new WaaAnalyzer());
        } else if (name == "FittingAnalyzer") {
            string type = analyzer.attribute("type").as_string();
            vecAnalyzer.push_back(new FittingAnalyzer(type));
        } else {
            stringstream ss;
            ss << "DetectorDriver: unknown analyzer type" << name;
            throw GeneralException(ss.str());
        }

        for (pugi::xml_attribute_iterator ait = analyzer.attributes_begin();
             ait != analyzer.attributes_end(); ++ait) {
            stringstream ss;
            ss << ait->name();
            if (ss.str().compare("name") != 0) {
                ss << " = " << ait->value();
                m.detail(ss.str(), 1);
            }
        }
    }
}

void DetectorDriver::Init(RawEvent& rawev) {
    for (vector<TraceAnalyzer *>::iterator it = vecAnalyzer.begin();
	 it != vecAnalyzer.end(); it++) {
        (*it)->Init();
        (*it)->SetLevel(20);
    }

    for (vector<EventProcessor *>::iterator it = vecProcess.begin();
         it != vecProcess.end(); it++) {
        (*it)->Init(rawev);
    }

    try {
        ReadCalXml();
        ReadWalkXml();
    } catch (GeneralException &e) {
        //! Any exception in reading calibration and walk correction
        //! will be intercepted here
        cout << endl;
        cout << "Exception caught at DetectorDriver::Init" << endl;
        cout << "\t" << e.what() << endl;
        Messenger m;
        m.fail();
        exit(EXIT_FAILURE);
    } catch (GeneralWarning &w) {
        cout << "Warning caught at DetectorDriver::Init" << endl;
        cout << "\t" << w.what() << endl;
    }
}

void DetectorDriver::ProcessEvent(RawEvent& rawev) {
    plot(dammIds::raw::D_NUMBER_OF_EVENTS, dammIds::GENERIC_CHANNEL);
    try {
        for (vector<ChanEvent*>::const_iterator it = rawev.GetEventList().begin();
             it != rawev.GetEventList().end(); ++it) {
            PlotRaw((*it));
            ThreshAndCal((*it), rawev);
            PlotCal((*it));

            string place = (*it)->GetChanID().GetPlaceName();
            if (place == "__-1")
                continue;

            if ( (*it)->IsSaturated() || (*it)->IsPileup() )
                continue;

            double time = (*it)->GetTime();
            double energy = (*it)->GetCalEnergy();
            int location = (*it)->GetChanID().GetLocation();

            EventData data(time, energy, location);
            TreeCorrelator::get()->place(place)->activate(data);
        }

        //!First round is preprocessing, where process result must be guaranteed
        //!to not to be dependent on results of other Processors.
        for (vector<EventProcessor*>::iterator iProc = vecProcess.begin();
        iProc != vecProcess.end(); iProc++)
            if ( (*iProc)->HasEvent() )
                (*iProc)->PreProcess(rawev);
        ///In the second round the Process is called, which may depend on other
        ///Processors.
        for (vector<EventProcessor *>::iterator iProc = vecProcess.begin();
        iProc != vecProcess.end(); iProc++)
            if ( (*iProc)->HasEvent() )
                (*iProc)->Process(rawev);
        // Clear all places in correlator (if of resetable type)
	for (map<string, Place*>::iterator it = 
		 TreeCorrelator::get()->places_.begin(); 
	     it != TreeCorrelator::get()->places_.end(); ++it)
	    if ((*it).second->resetable())
                (*it).second->reset();
    } catch (GeneralException &e) {
        /// Any exception in activation of basic places, PreProcess and Process
        /// will be intercepted here
        cout << "Exception caught at DetectorDriver::ProcessEvent" << endl;
        cout << "\t" << e.what() << endl;
        exit(EXIT_FAILURE);
    } catch (GeneralWarning &w) {
        cout << "Warning caught at DetectorDriver::ProcessEvent" << endl;
        cout << "\t" << w.what() << endl;
    }
}

void DetectorDriver::DeclarePlots() {
    try {
        DeclareHistogram1D(D_HIT_SPECTRUM, S7, "channel hit spectrum");
        DeclareHistogram2D(DD_RUNTIME_SEC, SE, S6, "run time - s");
        DeclareHistogram2D(DD_RUNTIME_MSEC, SE, S7, "run time - ms");

        if(Globals::get()->hasRaw()) {
            DetectorLibrary* modChan = DetectorLibrary::get();
            DeclareHistogram1D(D_NUMBER_OF_EVENTS, S4, "event counter");
            DeclareHistogram1D(D_HAS_TRACE, S8, "channels with traces");
            DeclareHistogram2D(DD_BUFFER_START_TIME, SE, S6, "dead time - 0.1%");
            DeclareHistogram2D(DD_DEAD_TIME_CUMUL, SE, S6, "dead time - cumul");
            DeclareHistogram1D(D_SUBEVENT_GAP, SE,
                               "time btwn chan-in event,10ns bin");
            DeclareHistogram1D(D_EVENT_LENGTH, SE,
                               "time length of event, 10 ns bin");
            DeclareHistogram1D(D_EVENT_GAP, SE, "time between events, 10 ns bin");
            DeclareHistogram1D(D_EVENT_MULTIPLICITY, S7,
                               "number of channels in event");
            DeclareHistogram1D(D_BUFFER_END_TIME, SE, "length of buffer, 1 ms bin");
            DetectorLibrary::size_type maxChan = modChan->size();

            for (DetectorLibrary::size_type i = 0; i < maxChan; i++) {
                if (!modChan->HasValue(i)) {
                    continue;
                }
                stringstream idstr;

                const Identifier &id = modChan->at(i);

                idstr << "M" << modChan->ModuleFromIndex(i)
                    << " C" << modChan->ChannelFromIndex(i)
                    << " - " << id.GetType()
                    << ":" << id.GetSubtype()
                    << " L" << id.GetLocation();
                DeclareHistogram1D(D_RAW_ENERGY + i, SE,
                                  ("RawE " + idstr.str()).c_str() );
                DeclareHistogram1D(D_FILTER_ENERGY + i, SE,
                                  ("FilterE " + idstr.str()).c_str() );
                DeclareHistogram1D(D_SCALAR + i, SE,
                                  ("Scalar " + idstr.str()).c_str() );
                if (Globals::get()->revision() == "A")
                    DeclareHistogram1D(D_TIME + i, SE,
                                       ("Time " + idstr.str()).c_str() );
                DeclareHistogram1D(D_CAL_ENERGY + i, SE,
                                  ("CalE " + idstr.str()).c_str() );
            }
        }

        for (vector<TraceAnalyzer *>::const_iterator it = vecAnalyzer.begin();
             it != vecAnalyzer.end(); it++) {
            (*it)->DeclarePlots();
        }

        for (vector<EventProcessor *>::const_iterator it = vecProcess.begin();
             it != vecProcess.end(); it++) {
            (*it)->DeclarePlots();
        }

    } catch (exception &e) {
        cout << "Exception caught at DetectorDriver::DeclarePlots" << endl;
        cout << "\t" << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}

int DetectorDriver::ThreshAndCal(ChanEvent *chan, RawEvent& rawev) {
    Identifier chanId = chan->GetChanID();
    int id            = chan->GetID();
    string type       = chanId.GetType();
    string subtype    = chanId.GetSubtype();
    map<string, int> tags = chanId.GetTagMap();
    bool hasStartTag  = chanId.HasTag("start");
    Trace &trace      = chan->GetTrace();

    RandomPool* randoms = RandomPool::get();

    double energy = 0.0;
    
    if (type == "ignore" || type == "")
        return(0);

    if ( !trace.empty() ) {
        plot(D_HAS_TRACE, id);

        for (vector<TraceAnalyzer *>::iterator it = vecAnalyzer.begin();
            it != vecAnalyzer.end(); it++) {
            (*it)->Analyze(trace, type, subtype, tags);
        }

        if (trace.HasValue("filterEnergy") ) {
            if (trace.GetValue("filterEnergy") > 0) {
                energy = trace.GetValue("filterEnergy");
                plot(D_FILTER_ENERGY + id, energy);
                trace.SetValue("filterEnergyCal",
                    cali.GetCalEnergy(chanId, trace.GetValue("filterEnergy")));
            } else {
                energy = 0.0;
            }

            /** Calibrate pulses numbered 2 and forth,
             * add filterEnergyXCal to the trace */
            int pulses = trace.GetValue("numPulses");
            for (int i = 1; i < pulses; ++i) {
                stringstream energyName;
                energyName << "filterEnergy" << i + 1;
                stringstream energyCalName;
                energyCalName << "filterEnergy" << i + 1 << "Cal";
                trace.SetValue(energyCalName.str(),
                    cali.GetCalEnergy(chanId,
                                      trace.GetValue(energyName.str())));
            }
        }

        if (trace.HasValue("calcEnergy") ) {
            energy = trace.GetValue("calcEnergy");
            chan->SetEnergy(energy);
        } else if (!trace.HasValue("filterEnergy")) {
            energy = chan->GetEnergy() + randoms->Get();
        }

        if (trace.HasValue("phase") ) {
	    //Saves the time in nanoseconds
            chan->SetHighResTime((trace.GetValue("phase") *
                                 Globals::get()->adcClockInSeconds() +
                                  (double)chan->GetTrigTime() *
                                  Globals::get()->filterClockInSeconds()) * 1e9);
        }
    } else {
        /// otherwise, use the Pixie on-board calculated energy
        /// add a random number to convert an integer value to a
        ///   uniformly distributed floating point
        energy = chan->GetEnergy() + randoms->Get();
	chan->SetHighResTime(0.0);
    }

    /** Calibrate energy and apply the walk correction. */
    double time, walk_correction;
    if(chan->GetHighResTime() == 0.0) {
	time = chan->GetTime(); //time is in clock ticks
	walk_correction = walk.GetCorrection(chanId, energy);
    } else {
	time = chan->GetHighResTime(); //time here is in ns
	walk_correction = walk.GetCorrection(chanId, trace.GetValue("tqdc"));
    }

    chan->SetCalEnergy(cali.GetCalEnergy(chanId, energy));
    chan->SetCorrectedTime(time - walk_correction);

    rawev.GetSummary(type)->AddEvent(chan);
    DetectorSummary *summary;

    summary = rawev.GetSummary(type + ':' + subtype, false);
    if (summary != NULL)
        summary->AddEvent(chan);

    if(hasStartTag && type != "logic") {
        summary =
            rawev.GetSummary(type + ':' + subtype + ':' + "start", false);
        if (summary != NULL)
            summary->AddEvent(chan);
    }
    return(1);
}

int DetectorDriver::PlotRaw(const ChanEvent *chan) {
    plot(D_RAW_ENERGY + chan->GetID(), chan->GetEnergy());
    return(0);
}

int DetectorDriver::PlotCal(const ChanEvent *chan) {
    plot(D_CAL_ENERGY + chan->GetID(), chan->GetCalEnergy());
    return(0);
}

EventProcessor* DetectorDriver::GetProcessor(const std::string& name) const {
    for (vector<EventProcessor *>::const_iterator it = vecProcess.begin();
	 it != vecProcess.end(); it++) {
	if ( (*it)->GetName() == name )
	    return(*it);
    }
    return(NULL);
}

void DetectorDriver::ReadCalXml() {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(cfg_.c_str());
    if (!result) {
        stringstream ss;
        ss << "DetectorDriver: error parsing file" << cfg_;
        ss << " : " << result.description();
        throw GeneralException(ss.str());
    }

    Messenger m;
    m.start("Loading Calibration");

    pugi::xml_node map = doc.child("Configuration").child("Map");

    /** Note that before this reading in of the xml file, it was already
     * processed for the purpose of creating the channels map.
     * Some sanity checks (module and channel number) were done there
     * so they are not repeated here/
     */
    bool verbose = map.attribute("verbose_calibration").as_bool();
    for (pugi::xml_node module = map.child("Module"); module;
         module = module.next_sibling("Module")) {
        int module_number = module.attribute("number").as_int(-1);
        for (pugi::xml_node channel = module.child("Channel"); channel;
             channel = channel.next_sibling("Channel")) {
            int ch_number = channel.attribute("number").as_int(-1);
            Identifier chanID = DetectorLibrary::get()->at(module_number,
                                                           ch_number);
            bool calibrated = false;
            for (pugi::xml_node cal = channel.child("Calibration");
                cal; cal = cal.next_sibling("Calibration")) {
                string model = cal.attribute("model").as_string("None");
                double min = cal.attribute("min").as_double(0);
                double max =
                  cal.attribute("max").as_double(numeric_limits<double>::max());

                stringstream pars(cal.text().as_string());
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
                    ss << "Module " << module_number << ", channel "
                       << ch_number << ": ";
                    ss << " model-" << model;
                    for (vector<double>::iterator it = parameters.begin();
                         it != parameters.end(); ++it)
                        ss << " " << (*it);
                    m.detail(ss.str(), 1);
                }
                cali.AddChannel(chanID, model, min, max, parameters);
                calibrated = true;
            }
            if (!calibrated && verbose) {
                stringstream ss;
                ss << "Module " << module_number << ", channel "
                   << ch_number << ": ";
                ss << " non-calibrated";
                m.detail(ss.str(), 1);
            }
        }
    }
    m.done();
}

void DetectorDriver::ReadWalkXml() {
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file(cfg_.c_str());
    if (!result) {
        stringstream ss;
        ss << "DetectorDriver: error parsing file " << cfg_;
        ss << " : " << result.description();
        throw GeneralException(ss.str());
    }

    Messenger m;
    m.start("Loading Walk Corrections");

    pugi::xml_node map = doc.child("Configuration").child("Map");
    /** See comment in the similiar place at ReadCalXml() */
    bool verbose = map.attribute("verbose_walk").as_bool();
    for (pugi::xml_node module = map.child("Module"); module;
         module = module.next_sibling("Module")) {
        int module_number = module.attribute("number").as_int(-1);
        for (pugi::xml_node channel = module.child("Channel"); channel;
             channel = channel.next_sibling("Channel")) {
            int ch_number = channel.attribute("number").as_int(-1);
            Identifier chanID = DetectorLibrary::get()->at(module_number,
                                                           ch_number);
            bool corrected = false;
            for (pugi::xml_node walkcorr = channel.child("WalkCorrection");
                walkcorr; walkcorr = walkcorr.next_sibling("WalkCorrection")) {
                string model = walkcorr.attribute("model").as_string("None");
                double min = walkcorr.attribute("min").as_double(0);
                double max =
                  walkcorr.attribute("max").as_double(
                                              numeric_limits<double>::max());

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
                    ss << "Module " << module_number
                       << ", channel " << ch_number << ": ";
                    ss << " model: " << model;
                    for (vector<double>::iterator it = parameters.begin();
                         it != parameters.end(); ++it)
                        ss << " " << (*it);
                    m.detail(ss.str(), 1);
                }
                walk.AddChannel(chanID, model, min, max, parameters);
                corrected = true;
            }
            if (!corrected && verbose) {
                stringstream ss;
                ss << "Module " << module_number << ", channel "
                << ch_number << ": ";
                ss << " not corrected for walk";
                m.detail(ss.str(), 1);
            }
        }
    }
    m.done();
}
