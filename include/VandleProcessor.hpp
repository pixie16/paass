/** \file VandleProcessor.hpp
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

#include "EventProcessor.hpp"
#include "TimingInformation.hpp"

/// Class to process VANDLE related events
class VandleProcessor : public EventProcessor,
    public TimingInformation {
public:
    /** Default Constructor */
    VandleProcessor();

    /** Default Destructor */
    ~VandleProcessor() {};

    /** Declare the plots used in the analysis */
    virtual void DeclarePlots(void);

    /** Process the event for VANDLE stuff
    * \param [in] event : the event to process
    * \return Returns true if the processing was successful */
    virtual bool Process(RawEvent &event);

protected:
    BarMap  barMap; //!< A map to hold all the bars
    TimingDataMap bigMap;//!< A map to hold all the big ends
    TimingDataMap smallMap;//!< A map to hold all the small ends
    TimingDataMap startMap;//!< A map to to hold all the starts
    TimingDataMap tvandleMap;//!< A map to hold all the Teeny Vandles
private:
    /** \brief Retrieve the Data and build the maps
    * \param [in] event : the even to get the data from
    * \return True if the retrieval was successful. */
    virtual bool RetrieveData(RawEvent &event);

    /** \brief Correct the time of flight based on the geometry of the setup
    * \param [in] TOF : The time of flight to correct
    * \param [in] corRadius : the corrected radius for the flight path
    * \param [in] z0 : perpendicular distance from the source to bar
    * \return True if the retrieval was successful. */
    virtual double CorrectTOF(const double &TOF,
                              const double &corRadius,
                              const double &z0) {
        return((z0/corRadius)*TOF);
    };

    /** Analyze the data retreived from the event
    * \param [in] rawev : the raw event to obtain gamma information */
    virtual void AnalyzeData(RawEvent& rawev);

    /** Builds the VANDLE bars from the individual channel maps
    * \param [in] endMap : Map containing the individual ends
    * \param [in] type : the type of VANDLE bars in the map
    * \param [in] barMap : The bar map that will be filled */
    virtual void BuildBars(const TimingDataMap &endMap,
                           const std::string &type, BarMap &barMap);

    /** Clear the maps in anticipation for the next event */
    virtual void ClearMaps(void);

    /** Calculate the crosstalk between bars (near useless) */
    virtual void CrossTalk(void);

    /** Fills up the map of VANDLE ends, and plots some useful stuff
    * \param [in] eventList : the list of VANDLE events to fill
    * \param [in] type : the type of VANDLE bar we're filling
    * \param [in] eventMap : The map to fill up */
    virtual void FillMap(const std::vector<ChanEvent*> &eventList,
                         const std::string type, TimingDataMap &eventMap);

    /** Process Teeny VANDLE events, will be moved to dedicated processor */
    virtual void Tvandle(void);

    bool hasDecay; //!< True if there was a correlated beta decay
    double decayTime; //!< the time of the decay
    int counter;//!< a counter for various uses

    typedef std::pair<unsigned int, unsigned int> CrossTalkKey; //!< Typedef for crosstalk
    typedef std::map<CrossTalkKey, double> CrossTalkMap;//!< typedef for map for crosstalk handling
    std::map<CrossTalkKey, double> crossTalk;//!< map to put in cross talk info
};
#endif
