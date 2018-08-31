///@file VandleProcessor.hpp
///@brief A class to handle VANDLE bars 
///Processes information from the VANDLE Bars, allows for
///beta-gamma-neutron correlations.
///
/// Currently, the code is set to recognize two types of starts: LeRIBSS style
/// and the new Double Beta starts (SiPMT implementation). I have currently
/// coded things to be mutually exclusive. If there are Double Beta Starts then
/// the code will not analyze the LeRIBSS style starts. This is to alleviate a
/// potential confusion when it comes to the histogramming. This problem will be
/// resolved at a later date. -SVP 12/17/2014
///
///@author S. V. Paulauskas, M. Madurga
///@date 26 July 2010
#ifndef __VANDLEPROCESSOR_HPP_
#define __VANDLEPROCESSOR_HPP_
#include <set>
#include <string>

#include "BarDetector.hpp"
#include "EventProcessor.hpp"
#include "HighResTimingData.hpp"

#include "ProcessorRootStruc.hpp"

/// Class to process VANDLE related events
class VandleProcessor : public EventProcessor {
public:
    ///Default Constructor */
    VandleProcessor();

    ///Default Destructor */
    ~VandleProcessor() {};

    ///Declare the plots used in the analysis */
    virtual void DeclarePlots(void);

    ///Constructor taking a list of detector types as an argument
    ///@param [in] typeList : the list of bar types that are in the analysis
    ///@param [in] res : The resolution of the DAMM histograms
    ///@param [in] offset : The offset of the DAMM histograms 
    ///@param [in] numStarts : number of starts we have to process */
    VandleProcessor(const std::vector<std::string> &typeList, const double &res, const double &offset,
                    const unsigned int &numStarts, const double &compression = 1.0);

    ///Preprocess the VANDLE data
    ///@param [in] event : the event to preprocess
    ///@return true if successful */
    virtual bool PreProcess(RawEvent &event);

    ///Process the event for VANDLE stuff
   ///@param [in] event : the event to process
   ///@return Returns true if the processing was successful */
    virtual bool Process(RawEvent &event);

    ///@brief Correct the time of flight based on the geometry of the setup
    ///@param [in] TOF : The time of flight to correct
    ///@param [in] corRadius : the corrected radius for the flight path
    ///@param [in] z0 : perpendicular distance from the source to bar
    ///@return True if the retrieval was successful. */
    virtual double CorrectTOF(const double &TOF, const double &corRadius, const double &z0) {
        return ((z0 / corRadius) * TOF);
    }

    ///@return the map of the build VANDLE bars */
    BarMap GetBars(void) { return bars_; }

    ///@return true if we requested small bars in the xml */
    bool GetHasSmall(void) { return requestedTypes_.find("small") != requestedTypes_.end(); }

    ///@return true if we requested medium bars in the xml  */
    bool GetHasMed(void) { return requestedTypes_.find("medium") != requestedTypes_.end(); }

    ///@return true if we requsted large bars in the xml */
    bool GetHasBig(void) { return requestedTypes_.find("big") != requestedTypes_.end(); }

private:
    ///Analyze the data for scenarios with Bar Starts; e.g. Double Beta detectors
    void AnalyzeBarStarts(const BarDetector &bar, unsigned int &barLoc);

    ///Analyze the data for scenarios with Single sided Starts; e.g. LeRIBSS beta scintillators.
    void AnalyzeStarts(const BarDetector &bar, unsigned int &barLoc);

    ///Fill up the basic histograms
    void FillVandleOnlyHists();

    void PlotTofHistograms(const double &tof, const double &cortof,const double &NCtof, const double &qdc,
                           const unsigned int &barPlusStartLoc, const std::pair<unsigned int, unsigned int> &offset,
                           bool &calibrated );

    ///@return Returns a pair of the appropriate offsets based off the VANDLE bar type <calibrated, NonCalibrated>
    ///@param [in] type : The type of bar that we are dealing with
    std::pair<unsigned int, unsigned int> ReturnOffset(const std::string &type);

    BarMap bars_;//!< A map to hold all the bars
    TimingMap starts_;//!< A map to to hold all the starts
    BarMap barStarts_;//!< A map that holds all of the bar starts
    DetectorSummary *geSummary_;//!< The Detector Summary for Ge Events

    bool hasDecay_; //!< True if there was a correlated beta decay
    double decayTime_; //!< the time of the decay

    double plotMult_;//!< The resolution multiplier for DAMM histograms
    double plotOffset_;//!< The offset multiplier for DAMM histograms
    double qdcComp_; //!<QDC compression value as read from the config file

    bool hasSmall_; //!< True if small bars were requested in the Config
    bool hasBig_; //!< True if big bars were requested in the Config
    bool hasMed_; //!< True if medium bars were requested in the Config

    unsigned int numStarts_; //!< The number of starts set in the Config File

    std::set<std::string> requestedTypes_;//!< The list of bar types to expect

    processor_struct::VANDLES vandles; //!<Working structure  
};

#endif
