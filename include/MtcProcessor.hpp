/** \file MtcProcessor.hpp
 *
 * class to handle mtc signals
 */

#ifndef __MTCPROCESSOR_HPP_
#define __MTCPROCESSOR_HPP_

#include "EventProcessor.hpp"

class MtcProcessor : public EventProcessor {
 public:
    /** Enable/disable double stop / start flag */
    MtcProcessor(bool double_stop, bool double_start);

    virtual void DeclarePlots(void);
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
 private:
    /** In some experiments the MTC stop signal was doubled
     * this flags enable removal of such an events */
    bool double_stop_;

    /** In some experiments the MTC start signal was doubled
     * this flags enable removal of such an events */
    bool double_start_;

    /** Upper limit in seconds for bad (double) start/stop event */
    static const double doubleTimeLimit_ = 100e-6;
};

#endif // __MTCPROCESSOR_HPP_
