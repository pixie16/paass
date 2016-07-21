/** \file VandleAtLeribssProcessor.hpp
 * \brief A class to process data from the LeRIBSS 2012 campaign.
 *
 *\author S. V. Paulauskas
 *\date February 5, 2015
 */
#ifndef __VANDLEATLERIBSSPROCESSOR_HPP_
#define __VANDLEATLERIBSSPROCESSOR_HPP_

#include "EventProcessor.hpp"
#include "VandleProcessor.hpp"

/// Class to process VANDLE related events
class VandleAtLeribssProcessor : public VandleProcessor {
public:
    /** Default Constructor */
    VandleAtLeribssProcessor();
    /** Default Destructor */
    ~VandleAtLeribssProcessor() {};
    /** Declare the plots used in the analysis */
    virtual void DeclarePlots(void);

    /** Constructor taking a list of detector types as an argument
     * \param [in] typeList : the list of bar types that are in the analysis
     * \param [in] res : The resolution of the DAMM histograms
     * \param [in] offset : The offset of the DAMM histograms 
     * \param [in] numStarts : the number of starts in the analysis */
    VandleAtLeribssProcessor(const std::vector<std::string> &typeList,
                    const double &res, const double &offset,
                    const double &numStarts);

    /** Process the event
    * \param [in] event : the event to process
    * \return Returns true if the processing was successful */
    virtual bool Process(RawEvent &event);
private:
    std::string fileName_; //!< the name of the his file
    std::vector<std::string> fileNames_; //!< the vector of output file names
};
#endif
