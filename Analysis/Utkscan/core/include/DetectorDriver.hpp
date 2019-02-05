/*! \file DetectorDriver.hpp
 *  \brief Main Driver for event processing.
 * \author S. N. Liddick, D. Miller, K. Miernik, S. V. Paulauskas
 * \date July 2, 2007
 *
 * The main analysis program.  A complete event is create in PixieStd
 * passed into this class. In this file are the details for experimental
 * processing of a raw event created by ScanList() in PixieStd.cpp.  Event
 * processing includes things that do not change from experiment to experiment
 * (such as energy calibration and raw parameter plotting) and things that do
 * (differences between MTC and RMS experiment, for example).
 * See manual for further details.
 *
 * SNL - 7-2-07
 * SNL - 7-12-07
 *       Add root analysis. If the ROOT program has been
 *       detected on the computer system the and the
 *       makefile has the useroot flag declared, ROOT
 *       analysis will be included.
 * DTM - Oct. '09
 *      Significant structural/cosmetic changes. Event processing is
 *      now primarily handled by individual event processors which
 *     handle their own DetectorDrivers
 *
 * SVP - Oct. '10
 *       Added the VandleProcessor for use with VANDLE.
 *       Added the PulserProcessor for use with Pulsers.
 *       Added the WaveformProcessor to determine ps time resolutions.
 * KM  - Dec. '12, Jan. '13
 *       Huge changes due to switching to XML configuration file.
 *       See git commits comments.
 *
 * TTK - June 2018
 *       Added System-wide root output
 *
*/
#ifndef __DETECTORDRIVER_HPP_
#define __DETECTORDRIVER_HPP_

#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Calibrator.hpp"
#include "ChanEvent.hpp"
#include "Globals.hpp"
#include "Messenger.hpp"
#include "Plots.hpp"
#include "WalkCorrector.hpp"

#ifdef useroot
#include "TROOT.h"
#include "TSystem.h"
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TH1.h>
#include <TH2.h>
#include "CloverProcessor.hpp"
#include "GammaScintProcessor.hpp"
#include "PaassRootStruct.hpp"
#include "PspmtProcessor.hpp"
#include "SingleBetaProcessor.hpp"
#include "VandleProcessor.hpp"
#include "NsheProcessor.hpp"

#endif

class Calibration;

class RawEvent;

class EventProcessor;

class TraceAnalyzer;

/*! \brief DetectorDriver controls event processing

  This class controls the processing of each event and includes the
  calibration of all raw energies, the plotting of raw and calibrated
  energies, and any experiment specific processing requirements
*/
class DetectorDriver {
public:
    /*! \return Instance is created upon first call */
    static DetectorDriver *get();

    const WalkCorrector *walk_; //!< Instance of the walk correction
    const Calibrator *cali_;//!< Instance of the calibrator
    Plots histo;//!< Instance of the histogram class

    /*! \brief Plots into histogram defined by dammId
    * \param [in] dammId : The histogram number to define
    * \param [in] val1 : the x value
    * \param [in] val2 : the y value
    * \param [in] val3 : the z value
    * \param [in] name : the name of the histogram */
    virtual void plot(int dammId, double val1, double val2 = -1, 
    double val3 = -1, const char *name = "h") {
        //if we want to skip the DAMM filling 
        if (Globals::get()->GetDammPlots()) {
            histo.Plot(dammId, val1, val2, val3, name);
        }
    }

    /*! \brief Control of the event processing
    *
    * The ProcessEvent() function is called from ScanList() in PixieStd.cpp
    * after an event has been constructed. This function is passed the mode
    * the analysis is currently in (the options are either "scan" or
    * "standaloneroot").  The function checks the thresholds for the individual
    * channels in the event and calibrates their energies.
    * The raw and calibrated energies are plotted if the appropriate DAMM spectra
    * have been created.  Then experiment specific processing is performed.
    * Currently, both RMS and MTC processing is available.  After all processing
    * has occured, appropriate plotting routines are called.
    * \param [in] rawev : the raw event to process */
    void ProcessEvent(RawEvent &rawev);

    /*! \brief Check threshold and calibrate each channel.
     * Check the thresholds and calibrate the energy for each channel using the
     * calibrations contained in the calibration vector filled during ReadCal()
     * \param [in] chan : the channel to do the calibration on
     * \param [in] rawev : the raw event to write the information into
     * \return an unused integer (maybe change to void) */
    int ThreshAndCal(ChanEvent *chan, RawEvent &rawev);

    /*! Called from PixieStd.cpp during initialization.
     * The calibration file Config.xml is read using the function ReadCal() and
     * checked to make sure that all channels have a calibration.
     * \param [in] rawev : the raw event to initialize with */
    void Init(RawEvent &rawev);

    /*! Plot the raw energies of each channel into the damm spectrum number
     * assigned to it in the map file with an offset as defined in
     * DammPlotIds.hpp
     * \param [in] chan : the channel to plot
     * \return an unused integer maybe use void*/
    int PlotRaw(const ChanEvent *chan);

    /*! Plot the calibrated energies of each channel into the damm spectrum
     * number assigned to it in the map file with an offset as defined in
     * DammPlotIds.hpp
     * \return an unused integer maybe use void*/
    int PlotCal(const ChanEvent *);

    /** Declares the raw plots that are used, and loops over all the declare
     * plots methods in all of the analyzers and processors. */
    void DeclarePlots();

    /** Use Exceptions to throw an exception here if sanity check was
     * not succesful */
    void SanityCheck(void) const {};

    /** Correlates the pixie clock to the wall clock
     * \param [in] d : the pixie time to correlate
     * \param [in] t : the wall time to correlate */
    void CorrelateClock(double d, time_t t) {
        pixieToWallClock = std::make_pair(d, t);
    }

    /** \return The wall time
     * \param [in] d : the pixie time to convert to wall time */
    time_t GetWallTime(double d) const {
        return (time_t) ((d - pixieToWallClock.first) *
                         Globals::get()->GetClockInSeconds() +
                         pixieToWallClock.second);
    }

    /** \return the list of the Event Processors in the analysis */
    const std::vector<EventProcessor *> &GetProcessors(void) const {
        return vecProcess;
    }

    /** \return the current pixie event number */
    unsigned long GetEventNumber(){return eventNumber_;};

    /** \return Detector Driver's First Event Time (units= ticks)*/
    double GetFirstEventTime(){return firstEventTime_;}

    /** \return Detector Driver's First Event Time (units= ns)*/
    double GetFirstEventTimeinNs(){return firstEventTimeinNs_;}

    /** \return the first detector event in the current pixie event (units= ticks)*/
    double GetEventFirstTime(){return eventFirstTime_;}

    /** \return The requested event processor
     * \param [in] name : the name of the processor to return */
    EventProcessor *GetProcessor(const std::string &name) const;

    /**\return The list of processors used the the analysis, from vecProcess
     * */
    std::set<std::string> GetProcessorList();

    /**\return System ROOT Output Status. True if ROOT output is requested */
    bool GetSysRootOutput(){ return sysrootbool_; }

    /** \return the set of detectors used in the analysis */
    const std::set<std::string> &GetUsedDetectors(void) const;

    ///Sets the processor list
    ///@param[in] a : The vector containing the pointer to the event processors
    void SetEventProcessors(const std::vector<EventProcessor *> &a) {
        vecProcess = a;
    }

    ///Sets the analyzer list
    ///@param[in] a : The vector containing the pointer to the Trace Analyzers
    void SetTraceAnalyzers(const std::vector<TraceAnalyzer *> &a) {
        vecAnalyzer = a;
    }

    /** Default Destructor */
    virtual ~DetectorDriver();


private:
    /** Constructor that initializes the various processors and analyzers. */
    DetectorDriver();

    DetectorDriver(const DetectorDriver &); //!< Overloaded constructor
    DetectorDriver &operator=(DetectorDriver const &);//!< Equality constructor
    static DetectorDriver *instance;//!< The only instance of DetectorDriver

    std::vector<EventProcessor *> vecProcess; /**< vector of processors to handle each event */

    std::vector<TraceAnalyzer *> vecAnalyzer; /**< object which analyzes traces of channels to extract
                   energy and time information */
    std::set<std::string> knownDetectors; /**< list of valid detectors that can
                   be used as detector types */
    std::string cfg_; //!< The configuration file to read
    std::pair<double, time_t> pixieToWallClock; /**< rough estimate of pixie to wall clock */
    unsigned long eventNumber_; //!< "Global" Event Number.
    double firstEventTime_; //!< The time of the first event that passes through the DetectorDriver
    double firstEventTimeinNs_; //!< The time of the first event that passes through the DetectorDriver in ns
    double eventFirstTime_; //!<The Time of the first detector event in the current pixie event
    /*! Declares a 1D histogram calls the C++ wrapper for DAMM
    * \param [in] dammId : The histogram number to define
    * \param [in] xSize : The range of the x-axis
    * \param [in] title : The title for the histogram */
    virtual void DeclareHistogram1D(int dammId, int xSize, const char *title) {
        if(Globals::get()->GetDammPlots()){
            histo.DeclareHistogram1D(dammId, xSize, title);
        }
    }
    /*! \brief Declares a 2D histogram calls the C++ wrapper for DAMM
    * \param [in] dammId : The histogram number to define
    * \param [in] xSize : The range of the x-axis
    * \param [in] ySize : The range of the y-axis
    * \param [in] title : The title of the histogram */
    virtual void DeclareHistogram2D(int dammId, int xSize, int ySize,
                                    const char *title) {
        if(Globals::get()->GetDammPlots()){
            histo.DeclareHistogram2D(dammId, xSize, ySize, title);
        }
    }
    /*! \brief Fills the Logic structure for ROOT output.
     * Because the logic spans pixie events if we fill in the processor it does not get filled into each ROOT entry.
     * So we will fill here in the DetectorDriver utilizing the work that was put into the TreeCorrelator  */
    void FillLogicStruc();

    std::set<std::string> setProcess; /**< list of processors used in the analysis.
    * This should be identical to vecProcess, but in string form */

    TFile *PixieFile;
    TTree *PTree;
    TBranch *PBr;

    PixTreeEvent pixie_tree_event_; /** tree event container class **/

    bool sysrootbool_; ///Bool for ROOT ouput
    bool fillLogic_; /// Should we fill the logic struct
    processor_struct::LOGIC LogStruc; //Logic root struc
    int tapeCycleNum_; //counts the number of tape cycles
    double lastCycleTime_; // last cycle start time (for cycle num incrementing)
    double rFileSizeGB_;/// Max size in GB for the ROOT file before starting a new one
};

#endif // __DETECTORDRIVER_HPP_
