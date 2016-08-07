/** \file E11006Processor.hpp
 * \brief A class specific to the e11006 experiment
 * \author S. V. Paulauskas
 * \date June 18, 2014
 */
#ifndef __E11006PROCESSOR_HPP_
#define __E11006PROCESSOR_HPP_

#include "EventProcessor.hpp"

///Processor for the e11006 experiment 
class E11006Processor : public EventProcessor {
 public:
    /** Default Constructor */
    E11006Processor(); // no virtual c'tors
    /** Default Destructor */
    ~E11006Processor(){};

    /** Process an event 
     * \param [in] event : the event to process 
     * \return true if successful*/
    bool Process(RawEvent &event);

    /** Declares plots */
    void DeclarePlots(void);
};
#endif // __E11006PROCESSOR_HPP_
