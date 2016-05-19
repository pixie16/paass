/** \file LogicProcessor.hpp
 * \brief Class to handle logic signals
 * derived originally from MTC processor
 */
#ifndef __LOGICPROCESSOR_HPP_
#define __LOGICPROCESSOR_HPP_

#include <vector>

#include "EventProcessor.hpp"

//! Class to handle logic signals
class LogicProcessor : public EventProcessor {
public:
    /** Default Constructor */
    LogicProcessor();

    /** Constructor taking histogram offset and range as arguments
    * \param [in] offset : the offset of the histograms
    * \param [in] range : the maximum number of histograms */
    LogicProcessor(int offset, int range, bool doubleStop = false,
		   bool doubleStart = false);

    /** Declare plots used in the analysis */
    virtual void DeclarePlots(void);

    /** Preprocess the event
    * \param [in] event : the event to process
    * \return true if the preprocess was successful */
    virtual bool PreProcess(RawEvent &event);

    /** Process the event
    * \param [in] event : the event to process
    * \return true if processing was successful */
    virtual bool Process(RawEvent &event);

    /** \return The logic status for a given location
     * \param [in] loc : the location to get the status from */
    virtual bool LogicStatus(size_t loc) const { return logicStatus.at(loc); };

    void SetDoubleStart(const bool &a) {doubleStart_ = a;};
    void SetDoubleStop(const bool &a) {doubleStop_ = a;};

    /** \return The stop count for a given location
     * \param [in] loc : the location to get the count from */
    unsigned long StopCount(size_t loc) const { return stopCount.at(loc);};

    /** \return The start count for a given location
     * \param [in] loc : the location to get the status from */
    unsigned long StartCount(size_t loc) const { return startCount.at(loc); };

    /** \return The time since the last off
     * \param [in] loc : the location to get the status from
     * \param [in] t : the current time to compare with the last one */
    double TimeOff(size_t loc, double t) const {
        return (!LogicStatus(loc) ? (t-lastStopTime.at(loc)) : 0.);
    }

    /** \return The time since the last on
     * \param [in] loc : the location to get the status from
     * \param [in] t : the current time to compare with the last one */
    double TimeOn(size_t loc, double t) const {
        return (LogicStatus(loc) ? (t-lastStartTime.at(loc)) : 0.);
    }

protected:
    std::vector<double> lastStartTime; //!< time of last leading edge
    std::vector<double> lastStopTime;  //!< time of last trailing edge
    std::vector<bool>   logicStatus;   //!< current level of the logic signal

    std::vector<unsigned long> stopCount;  //!< number of stops received
    std::vector<unsigned long> startCount; //!< number of starts received

private:
    /** Basic Processing of the event
    * \param [in] event : the even to process */
    void BasicProcessing(RawEvent &event);

    /** Trigger Processing of the event
    * \param [in] event : the even to process */
    void TriggerProcessing(RawEvent &event);

    int plotSize; //!< Size of the plots to make

    /** In some experiments the MTC stop signal was doubled
     * this flags enable removal of such an events */
    bool doubleStop_;

    /** In some experiments the MTC start signal was doubled
     * this flags enable removal of such an events */
    bool doubleStart_;

    bool NiftyGraph(RawEvent &event);
};

#endif // __LOGICPROCESSOR_HPP_
