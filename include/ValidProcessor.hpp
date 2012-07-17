/** \file ValidProcessor.hpp
 * 
 * Class for handling Valid Signals
 * For the VANDLE-MoNA experiment at MSU
 */

#ifndef __VALIDPROCESSOR_HPP_
#define __VALIDPROCESSOR_HPP_

#include <sstream>

#include "EventProcessor.hpp"
#include "GetArguments.hpp"

class ValidProcessor : public EventProcessor
{
 public:
    ValidProcessor(); // no virtual c'tors
    virtual bool Process(RawEvent &event);
 private:
    virtual void OutputData(RawEvent &event);
    
    static std::stringstream fileName;
}; //Class ValidProcessor
#endif // __VALIDPROCESSOR_HPP_
