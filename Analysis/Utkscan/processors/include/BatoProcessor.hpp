///@file BatoProcessor.hpp
///@brief Processor for the BATOs (Clover beta vetos)
///@authors T.T. King

#ifndef PAASS_BATOPROCESSOR_H
#define PAASS_BATOPROCESSOR_H

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

class BatoProcessor : public EventProcessor {
public:
    /**Constructor
     * \param [in] gammaThreshold : Set threshold for gammarays
     * \param [in] vetoThresh : Set threshold for lightIon veto
     * \param [in] ionTrigThresh : Set threshold for ion Trigger
     */
    BatoProcessor();

    /** Deconstructor */
    ~BatoProcessor() = default;

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
    void BatoBuilder(const std::set<int> &cloverLocs);

    /** onBoardQDC Math */
    double OnBoardQDC_to_WaveFormQDC(std::vector<unsigned> onBoardQdc);

private:
    std::map<int, int> leafToBato;   /*!< Translate a chan location to a bato number */
    unsigned int numBatos;           /*!< number of batos in map */
    static const unsigned int chansPerBato = 2; /*!< number of channels per bato */
    std::map<int, std::vector<double>> BATO_En; // Build the bato "bars" for Pad energy
    std::map<int, std::pair<ChanEvent *, ChanEvent *>> padEvents; // L/R pad events keeping on the largest

    processor_struct::BATO BatoStruct; //!<Root Struct

    int batoevtcounter;
};

#endif //PAASS_CLOVERFRAGPROCESSOR_H
