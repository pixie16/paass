///@file GammaScintFragProcessor.hpp
///@brief Faster/ Simpler Processor for Scintilation Gamma-ray detectors
///@authors T.T. King

#ifndef PAASS_GAMMASCINTFRAGPROCESSOR_HPP
#define PAASS_GAMMASCINTFRAGPROCESSOR_HPP

#include <ctime>
#include <fstream>
#include <set>
#include <sstream>
#include <string>

#include "DammPlotIds.hpp"
#include "EventProcessor.hpp"
#include "Messenger.hpp"
#include "StringManipulationFunctions.hpp"
#include "pugixml.hpp"

#ifdef useroot

#include <TBranch.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TTree.h>
#include "GSaddback.hpp"
#include "PaassRootStruct.hpp"
#include "TROOT.h"
#include "TSystem.h"

#endif

class GammaScintFragProcessor : public EventProcessor {
   public:
    ///Default Constructor
    //GammaScintFragProcessor();

    ///Default Destructor
    ~GammaScintFragProcessor();

    ///Constructor taking a list of detector types as an argument
    ///@param [in] GSArgs : Map of "Arguments" for the processor, HRBWin, FacilityType, etc (Check GammaScintFragProcessor.hpp for more)
    ///@param [in] typeList : the list of bar types that are in the analysis
    ///@param [in] timeScales : list of time scales used in the energy vs time plots (units are Secs)
    GammaScintFragProcessor(const std::map<std::string, std::string> &GSArgs,
                            const std::vector<std::string> &typeList);

    ///Declare the plots for the processor
    virtual void DeclarePlots(void);

    /// Preprocess the Event
    /// \param [in] event : the event to preprocess
    /// \return true if successful
    virtual bool PreProcess(RawEvent &event);

    ///Process the event
    /// \param [in] event : the event to process
    /// \return Returns true if the processing was successful
    virtual bool Process(RawEvent &event);

    std::map<std::string, std::string> GetTHeader() { return Theader; }

    private:
    /** Returns Damm Histo ID offsets for known types
     * \param [in] subtype : The known subtype to lookup
     * @return Returns the damm offset
     */
    unsigned int ReturnOffset(const std::string &subtype);

    // Variable list
    std::set<std::string> typeList_;     //!< list of requested types
    std::vector<ChanEvent *> GSEvents_;  //!< Vector of GammaScint Events

    bool hasLowResBeta_;  //!< has Event (Low Res) Beta
    bool firstGSEvent_;   //!< only True for Very First GammaScintEvent

    double firstEventTime_;                        //!<Time for first event in the file
    unsigned long evtNum_ = 0;                     //!< Event Number as received from Detector Driver (Master Evt Number)
   
    double bunchLast_;                             //!<Time of Last Long Time Scale Bunch ( in ns) (i.e. tape cycle or 30 sec bunch
    double bunchingTime_;                          //!< Time bin size for energy vs Long time plot (frag type ONLY)
   
    bool localRoot_;      //!<will the detectorDirver ask for root data
    int bunchNum_ = 0;  //!<bunch number (bunch defined in bunchLast comment)

    //input arguments
    bool ISOL_;                      //!<How to plot the energy vs Long Time scale; for tracking Drift
    std::string FacilType_;          //!< String of the Facility Type (frag or ISOL)
    std::string BunchingTimestr_;    //<!String containing the bunching time as parsed. (For rootfiles TNamed's)

    processor_struct::GAMMASCINT GSstruct;  //!< structure of det event info for PEsing

    std::map<std::string, std::string> Theader;  // map of header info for the root file from this processor
};

#endif
