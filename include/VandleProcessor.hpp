/** \file VandleProcessor.hpp
 * \brief A class to handle VANDLE bars
 *
 *Processes information from the VANDLE Bars, allows for
 *beta-gamma-neutron correlations. The prototype for this
 *code was written by M. Madurga.
 *
 *\author S. V. Paulauskas
 *\date 26 July 2010
 */
#ifndef __VANDLEPROCESSOR_HPP_
#define __VANDLEPROCESSOR_HPP_

#include "BarDetector.hpp"
#include "EventProcessor.hpp"
#include "HighResTimingData.hpp"

/// Class to process VANDLE related events
class VandleProcessor : public EventProcessor {
public:
    /** Default Constructor */
    VandleProcessor();
    /** Default Destructor */
    ~VandleProcessor() {};
    /** Declare the plots used in the analysis */
    virtual void DeclarePlots(void);

    /** Preprocess the VANDLE data
     * \param [in] event : the event to preprocess
     * \return true if successful */
    virtual bool PreProcess(RawEvent &event);

    /** Process the event for VANDLE stuff
    * \param [in] event : the event to process
    * \return Returns true if the processing was successful */
    virtual bool Process(RawEvent &event);
protected:
    BarMap bars_;//!< A map to hold all the bars
    TimingMap starts_;//!< A map to to hold all the starts
    DetectorSummary *geSummary_;
private:
    /** Analyze the data.
    * \param [in] bartype : the type of bar that we want to analyze. */
    void AnalyzeData(void);
    /** Clear the maps in anticipation for the next event */
    void ClearMaps(void);

    /** \brief Correct the time of flight based on the geometry of the setup
    * \param [in] TOF : The time of flight to correct
    * \param [in] corRadius : the corrected radius for the flight path
    * \param [in] z0 : perpendicular distance from the source to bar
    * \return True if the retrieval was successful. */
    virtual double CorrectTOF(const double &TOF, const double &corRadius,
                              const double &z0) {
        return((z0/corRadius)*TOF);
    };

    /** Fill up the basic histograms */
    void FillBasicHists();

    /** Calculate the crosstalk between bars (near useless) */
    //virtual void CrossTalk(void);

    bool hasDecay_; //!< True if there was a correlated beta decay
    double decayTime_; //!< the time of the decay

    typedef std::pair<unsigned int, unsigned int> CrossTalkKey; //!< Typedef for crosstalk
    typedef std::map<CrossTalkKey, double> CrossTalkMap;//!< typedef for map for crosstalk handling
    std::map<CrossTalkKey, double> crossTalk;//!< map to put in cross talk info
};
#endif
