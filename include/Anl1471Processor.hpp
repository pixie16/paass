/** \file Anl1471Processor.hpp
 * \brief A class to process data from the LeRIBSS 2012 campaign.
 *
 *\author S. V. Paulauskas
 *\date September 19, 2015
 */
#ifndef __ANL1471PROCESSOR_HPP_
#define __ANL1471PROCESSOR_HPP_

#include "EventProcessor.hpp"
#include "VandleProcessor.hpp"

/// Class to process VANDLE related events
class Anl1471Processor : public VandleProcessor {
public:
    /** Default Constructor */
    Anl1471Processor();
    /** Default Destructor */
    ~Anl1471Processor() {};
    /** Declare the plots used in the analysis */
    virtual void DeclarePlots(void);

    /** Constructor taking a list of detector types as an argument
     * \param [in] typeList : the list of bar types that are in the analysis
     * \param [in] res : The resolution of the DAMM histograms
     * \param [in] offset : The offset of the DAMM histograms */
    Anl1471Processor(const std::vector<std::string> &typeList,
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
