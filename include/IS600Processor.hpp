/** \file IS600Processor.hpp
 * \brief A class to process data from ISOLDE 599 and 600 experiments using
 * VANDLE
 *
 *\author S. V. Paulauskas
 *\date July 14, 2015
 */
#ifndef __IS600PROCESSOR_HPP_
#define __IS600PROCESSOR_HPP_

#include "EventProcessor.hpp"
#include "VandleProcessor.hpp"

/// Class to process VANDLE analysis for ISOLDE experiments 599 and 600 related events
class IS600Processor : public VandleProcessor {
public:
    /** Default Constructor */
    IS600Processor();
    /** Default Destructor */
    ~IS600Processor() {};
    /** Declare the plots used in the analysis */
    virtual void DeclarePlots(void);

    /** Constructor taking a list of detector types as an argument
     * \param [in] typeList : the list of bar types that are in the analysis
     * \param [in] res : The resolution of the DAMM histograms
     * \param [in] offset : The offset of the DAMM histograms */
    IS600Processor(const std::vector<std::string> &typeList,
                    const double &res, const double &offset,
                    const double &numStarts);

    /** Process the event
    * \param [in] event : the event to process
    * \return Returns true if the processing was successful */
    virtual bool Process(RawEvent &event);
private:
    std::string fileName_;
    std::vector<std::string> fileNames_;
};
#endif
