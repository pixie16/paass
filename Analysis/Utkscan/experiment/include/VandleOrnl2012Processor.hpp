///@file VandleOrnl2012Processor.hpp
///@brief A class to process data from the LeRIBSS 2012 campaign.
///@author S. V. Paulauskas
///@date February 5, 2015
#ifndef __VANDLEORNL2012PROCESSOR_HPP_
#define __VANDLEORNL2012PROCESSOR_HPP_

#include "EventProcessor.hpp"

/// Class to process VANDLE related events
class VandleOrnl2012Processor : public EventProcessor {
public:
    /** Default Constructor */
    VandleOrnl2012Processor();

    /** Default Destructor */
    ~VandleOrnl2012Processor() {};

    /** Declare the plots used in the analysis */
    void DeclarePlots(void);

    /** Process the event
    * \param [in] event : the event to process
    * \return Returns true if the processing was successful */
    bool Process(RawEvent &event);

private:
    std::string fileName_; //!< the name of the his file
    std::vector <std::string> fileNames_; //!< the vector of output file names
};

#endif
