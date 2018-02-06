///@file Anl1471Processor.hpp
///@brief A class to process data from ANL1471 experiment using VANDLE.
///@author S. Z. Taylor and S. V. Paulauskas
///@date July 14, 2015
#ifndef __ANL1471PROCESSOR_HPP_
#define __ANL1471PROCESSOR_HPP_
#include <fstream>

#include "EventProcessor.hpp"

/// Class to process ANL experiment related events
class Anl1471Processor : public EventProcessor {
public:
    /** Default Constructor */
    Anl1471Processor();
    /** Default Destructor */
    ~Anl1471Processor();
    /** Declare the plots used in the analysis */
    virtual void DeclarePlots(void);

    /** Constructor taking a list of detector types as an argument
     * \param [in] typeList : the list of bar types that are in the analysis
     * \param [in] res : The resolution of the DAMM histograms

     * \param [in] offset : The offset of the DAMM histograms
     * \param [in] numStarts : number of starts in the analysis */
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