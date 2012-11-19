/** \file MtcProcessor.hpp
 * \brief class to handle mtc signals
 */

#ifndef __MTCPROCESSOR_HPP_
#define __MTCPROCESSOR_HPP_

#include "EventProcessor.hpp"

class MtcProcessor : public EventProcessor {
 public:
    MtcProcessor();
    virtual void DeclarePlots(void);
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
 private:
    double lastStartTime; //< time of last leading edge
    double lastStopTime;  //< time of last trailing edge
};

#endif // __MTCPROCESSOR_HPP_
