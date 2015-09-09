/** \file E11006Processor.hpp
 * \brief A class specific to the e11006 experiment
 * \author S. V. Paulauskas
 * \date June 18, 2014
 */
#ifndef __E11006PROCESSOR_HPP_
#define __E11006PROCESSOR_HPP_

#include "EventProcessor.hpp"

class ValidProcessor : public EventProcessor {
 public:
    E11006Processor(); // no virtual c'tors
    ~E11006Processor(){};
    bool Process(RawEvent &event);
    void E11006Processor::DeclarePlots(void);
 private:
};
#endif // __E11006PROCESSOR_HPP_
