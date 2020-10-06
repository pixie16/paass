///\file GeProcessor.hpp
///@brief Processor for germanium detectors
///@author S. V. Paulauskas
#ifndef __GEPROCESSOR_HPP_
#define __GEPROCESSOR_HPP_

#include <map>
#include <vector>
#include <utility>
#include <cmath>

#include "EventProcessor.hpp"
#include "RawEvent.hpp"

//! Processor to handle Ge events
class GeProcessor : public EventProcessor {
public:
    ///@brief Default constructor
    GeProcessor();

    ///Pre-process the event
    ///@param [in] event : the event to Pre-process
    ///@return true if successful
    virtual bool PreProcess(RawEvent &event);

    ///Declare the plots for the processor
    virtual void DeclarePlots(void);

private:
    processor_struct::CLOVER GEstruct;
};

#endif // __GEPROCESSOR_HPP_
