/** \file WalkVandleBetaProcessor.hpp
 * \brief A class to determine the walk between Vandle and the LeRIBSS betas
 *\author S. V. Paulauskas
 *\date February 24, 2015
 */
#ifndef __WALKVANDLEBETAPROCESSOR_HPP_
#define __WALKVANDLEBETAPROCESSOR_HPP_

#include "EventProcessor.hpp"
#include "VandleProcessor.hpp"

/// Class to process VANDLE related events
class WalkVandleBetaProcessor : public VandleProcessor {
public:
    /** Default Constructor */
    WalkVandleBetaProcessor();
    /** Default Destructor */
    ~WalkVandleBetaProcessor() {};
    /** Declare the plots used in the analysis */
    virtual void DeclarePlots(void);

    /** Constructor taking a list of detector types as an argument
     * \param [in] typeList : the list of bar types that are in the analysis
     * \param [in] res : The resolution of the DAMM histograms
     * \param [in] offset : The offset of the DAMM histograms 
     * \param [in] numStarts : the number of starts in the analysis */
    WalkVandleBetaProcessor(const std::vector<std::string> &typeList,
                    const double &res, const double &offset,
                    const double &numStarts);

    /** Process the event
    * \param [in] event : the event to process
    * \return Returns true if the processing was successful */
    virtual bool Process(RawEvent &event);
};
#endif
