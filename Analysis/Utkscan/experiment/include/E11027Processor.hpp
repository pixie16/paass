///@file E11027Processor.hpp
///@brief Class that handles outputting MONA-LISA-SWEEPER timestamps from the E11027 experiment conducted at NSCL.
///@author S. V. Paulauskas
///@date May 26, 2017
#ifndef __E11027PROCESSOR_HPP__
#define __E11027PROCESSOR_HPP__
#include <fstream>

#include "EventProcessor.hpp"

/// Working template class for experiment processors
class E11027Processor : public EventProcessor {
public:
    ///Default Constructor 
    E11027Processor();

    ///Default Destructor 
    ~E11027Processor();

    ///Declare the plots used in the analysis 
    void DeclarePlots(void);

    ///Process the event
    ///@param [in] event : the event to process
    ///@return Returns true if the processing was successful
    bool Process(RawEvent &event);

private:
    std::ofstream *poutstream_; //!< Pointer to ouptut ASCII file stream.
};

#endif
