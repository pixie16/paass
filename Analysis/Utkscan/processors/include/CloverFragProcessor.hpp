///@file CloverFragProcessor.hpp
///@brief Processor for Clovers at Fragmentation Facilities (Based HEAVILY on CloverProcessor.cpp)
///@authors T.T. King

#ifndef PAASS_CLOVERFRAGPROCESSOR_H
#define PAASS_CLOVERFRAGPROCESSOR_H

#include <map>
#include <vector>
#include <utility>
#include <cmath>

#include "EventProcessor.hpp"
#include "PaassRootStruct.hpp"
#include "RawEvent.hpp"

#include "DammPlotIds.hpp"
#include "Messenger.hpp"
#include "pugixml.hpp"
#include "StringManipulationFunctions.hpp"

class CloverFragProcessor : public EventProcessor {
public:
    /**Constructor
     * \param [in] gammaThreshold : Set threshold for gammarays
     * \param [in] vetoThresh : Set threshold for lightIon veto
     * \param [in] ionTrigThresh : Set threshold for ion Trigger
     */
    CloverFragProcessor(double gammaThreshold, double vetoThresh, double ionTrigThresh, double betaThresh);

    /** Deconstructor */
    ~CloverFragProcessor() = default;

    /** Preprocess the event
    * \param [in] event : the event to preprocess
    * \return true if successful
    */
    virtual bool PreProcess(RawEvent &event);

    /** Process the event
     * \param [in] event : the event to process
     * \return true if successful
     */
    virtual bool Process(RawEvent &event);

    /** Declare the plots for the processor */
    virtual void DeclarePlots(void);

    /** Build Clovers */
    void CloverBuilder(const std::set<int> &cloverLocs);

private:
    static const unsigned int chansPerClover = 4; /*!< number of channels per clover */

    std::map<int, int> leafToClover;   /*!< Translate a leaf location to a clover number */
    unsigned int numClovers;           /*!< number of clovers in map */

    double gammaThresh_,vetoThresh_,ionTrigThresh_,betaThresh_; //!<Thresholds

    processor_struct::CLOVERS Cstruct; //!<Root Struct
};

#endif //PAASS_CLOVERFRAGPROCESSOR_H
