///@file GammaScintProcessor.hpp
///@brief Processor for Scintilation Gamma-ray detectors
///@authors T.T. King

#ifndef PAASS_GAMMASCINTPROCESSOR_HPP
#define PAASS_GAMMASCINTPROCESSOR_HPP

#include <ctime>
#include <fstream>
#include <set>
#include <sstream>
#include <string>

#include "DammPlotIds.hpp"
#include "EventProcessor.hpp"
#include "Messenger.hpp"
#include "pugixml.hpp"
#include "StringManipulationFunctions.hpp"


#ifdef useroot

#include "TROOT.h"
#include "TSystem.h"
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TH1.h>
#include <TH2.h>
#include "ProcessorStruc.hpp"
#include "GSaddback.hpp"

#endif

class GammaScintProcessor : public EventProcessor {
public:
    ///Default Constructor
    //GammaScintProcessor();

    ///Default Destructor
    ~GammaScintProcessor();

    ///Constructor taking a list of detector types as an argument
    ///@param [in] GSArgs : Map of "Arguments" for the processor, HRBWin, FacilityType, etc (Check GammaScintProcessor.hpp for more)
    ///@param [in] typeList : the list of bar types that are in the analysis
    ///@param [in] timeScales : list of time scales used in the energy vs time plots (units are Secs)
    GammaScintProcessor(const std::map<std::string,std::string> &GSArgs,
                        const std::vector<std::string> &typeList,
                        const std::vector<std::string> &timeScales);

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


    /**Returns the vector of GScint singles events. used for coincidence plotting in root
     *
     * @return vector of GS singles
     */
    std::vector<GAMMASCINT> GetGSVector(){ return PEsing;}

    std::map<std::string,std::string> GetTHeader(){return Theader;}

protected:
    /** Declares a histogram with a range of time scales
     * \param [in] dammId : the dammID to plot
     * \param [in] xsize : the size in the x range
     * \param [in] ysize : the size in the y range
     * \param [in] title : the title of the histogram
     * \param [in] halfWordsPerChan : the half words per channel (default is 1, cloverProcessor is 2)
     * this is more or less the bit size of the bins (See Cory Thornsberry for more)
     * \param [in] timeScale : the list of time scales to plot
     * \param [in] units : the units for the plot */
    void DeclareHistogramTimeY(int dammId, int xsize, int ysize,
                               const char *title, int halfWordsPerChan,
                               const std::vector<float> &timeScale,
                               const char *units);

    /** Declares a histogram with a single time scale
     * \param [in] dammId : the dammID to plot
     * \param [in] xsize : the size in the x range
     * \param [in] ysize : the size in the y range
     * \param [in] title : the title of the histogram
     * \param [in] halfWordsPerChan : the half words per channel (default is 1, cloverProcessor is 2)
     * this is more or less the bit size of the bins (See Cory Thornsberry for more)
     * \param [in] timeScale : Time scale to plot
     * \param [in] units : the units for the plot */
    void DeclareHistogramTimeY(int dammId, int xsize, int ysize,
                               const char *title, int halfWordsPerChan,
                               const float &timeScale,
                               const char *units);

    /** Plotting function to plot in a specific granularity
     * \param [in] dammId : the damm id to plot into
     * \param [in] x : the x value to plot
     * \param [in] y : the y value to plot
     * \param [in] timeScale : the list of time scales to plot into */
    void timePloty(int dammId, double x, double y,
                   const std::vector<float> &timeScale);

private:
    /** Returns Damm Histo ID offsets for known types
     * \param [in] subtype : The known subtype to lookup
     * @return Returns the damm offset
     */
    unsigned int ReturnOffset(const std::string &subtype);

    /** Returns the addback parameters for known types
     * \param [in] subtype : The known subtype to lookup
     * \param [in] option : Parameter to lookup
     * @return Returns (by reference ) parameter
     */
    double GetAddbackPara(const std::string &subtype,const std::string &option);

    /** Updates the Addback Reference time for known types
     * \param [in] subtype : The subtype's whose RefTime needs updating
     * \param [in] newRefTime : New Reference time
     */
    void SetAddbackRefTime(const std::string &subtype, const double &newRefTime);

    /** Returns the addback struct for a specific type
     * @param subtype
     * @return Pointer to the GSAddback struct for the known subtypes
     */
    GSAddback* GetAddbackStruct(const std::string &subtype);

    // Variable list
    std::set<std::string> typeList_ ;//!< list of requested types
    std::vector<ChanEvent *> GSEvents_; //!< Vector of GammaScint Events

    /** The nested map structure is <subtype , < parameter , value > >.
     *    Parameters are "thresh" , "subEvtWin", "refTime".
     *    RefTimes and SubEvtWin need to be in seconds in the cfg but we convert to ticks for the running
     */ std::map<std::string,std::map<std::string,double> > ParameterMap; //!<Map of addback parameters

    bool hasTrigBeta_;//!<  has a Trigger (the GLobal Pixie Trigger; i.e. VANDLE's) Beta
    bool hasLowResBeta_; //!< has Event (Low Res) Beta
    bool hasMedResBeta_; //!< has Medium Res Beta
    bool firstGSEvent_;//!< only True for Very First GammaScintEvent

    double firstEventTime_; //!<Time for first event in the file
    unsigned long evtNum_=0; //!< Event Number as received from Detector Driver (Master Evt Number)
    std::pair<double, std::string> MRBetaWindow_ ;//!< Window size in nanoseconds for the Medium Res beta gate (mainly for LaBr3)
    double bunchLast_; //!<Time of Last Long Time Scale Bunch ( in ns) (i.e. tape cycle or 30 sec bunch
    double bunchingTime_;//!< Time bin size for energy vs Long time plot (frag type ONLY)
    double MAXTIMEPLOTS=3; //!< Max number of extra Time scales to plot (excluding 10ms which is always made if EvsT == True).
    //!< These Plots are add significant size to the output .his file. So we dont want to make too many of them

    bool SysRoot_; //!<will the detectorDirver ask for root data
    int bunchNum_ =0 ; //!<bunch number (bunch defined in bunchLast comment)

    const int binDepth = 1; //!<With binDep 1 they dont add so much; (cloverProcessor uses 2, but the normal DeclareHis uses 1


    //Vectors of Events for addback
    GSAddback* LHaddBack_;//!<structure for Small HAGRiD addback 
    GSAddback* NaddBack_;//!<structure for NaI addback
    GSAddback* BHaddBack_;//!<structure for Big HAGRiD addback
    GSAddback* FAILEDaddback_;//!< Numeric Limits vector for Failed GetAddbackStruct

    /** Most Beta (actual beta type) events are Multi 1
        but this allow for multiple beta decays in 1 pixie event (requires extreme rate)
        This is a vector of a pair of doubles The data is stored as <<Time,Energy>,...>
    */std::vector<std::pair<double,double>> BetaList;

    std::map<unsigned int, std::pair<double, double> > lrtBetas; //!<Low Res Time beta bar map (ISOL style). Order is <Double Beta det #, < Time, Energy > >
   
    //input arguments
    bool BetaGammGamm = false; //!< beta gamma gamma plots which are LARGE and not implamented yet. (might just dump to root rather than here)
    bool ISOL_; //!<How to plot the energy vs Long Time scale; for tracking Drift
    bool EvsT_; //!<Make Energy Vs Time plots?
    std::string FacilType_ ; //!< String of the Facility Type (frag or ISOL)
    std::vector<float> timeScales_; //!< list of time scales to plot (10 ms always present depending on EvsT)
    std::string BunchingTimestr_;//<!String containing the bunching time as parsed. (For rootfiles TNamed's)

    GAMMASCINT Gsing,DefaultStruct; //!< structure of det event info for PEsing, as well as a default version for resetting
    std::vector<GAMMASCINT> PEsing; //!< vector of detector events per pixie event

    std::map<std::string,std::string> Theader; // map of header info for the root file from this processor
};


#endif
