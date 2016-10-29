///@file Ornl2016Processor.hpp
///@brief A class to process data taken at HRIBF in February-March 2016
///@author S. V. Paulauskas
///@date July 14, 2015
#ifndef __ORNL2016PROCESSOR_HPP_
#define __ORNL2016PROCESSOR_HPP_

#include "EventProcessor.hpp"

///A class to process data taken at HRIBF in February-March 2016
class Ornl2016Processor : public EventProcessor {
public:
    ///Default Constructor
    Ornl2016Processor();

    ///Default Destructor
    ~Ornl2016Processor() {};

    ///Declare the plots used in the analysis
    void DeclarePlots(void);

    ///@Process the event
    ///@param [in] event : the event to process
    ///@return Returns true if the processing was successful
    bool Process(RawEvent &event);
};

#endif