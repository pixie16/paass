/** \file ValidProcessor.hpp
 * \brief Class for handling Valid Signals
 * For the VANDLE-MoNA experiment at MSU
 * \author S. V. Paulauskas
 * \date 20 June 2012
 */
#ifndef __VALIDPROCESSOR_HPP_
#define __VALIDPROCESSOR_HPP_

#include <sstream>

#include "EventProcessor.hpp"
#include "GetArguments.hpp"

//! Class to handle the Valid signal from MoNA DAQ
class ValidProcessor : public EventProcessor {
 public:
    /** Default Constructor */
    ValidProcessor();

    /** Default Destructor */
    ~ValidProcessor(){};

    /** Process events with a Valid in them
    * \param [in] event : the event to process
    * \return True if processing was successful */
    virtual bool Process(RawEvent &event);
 private:
    /** Output the data to a text file
    * \param [in] event : the event to output */
    virtual void OutputData(RawEvent &event);

    static std::stringstream fileName; //!< Stream to create the file name for output
};
#endif
