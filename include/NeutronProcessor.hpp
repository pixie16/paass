/** \file NeutronProcessor.hpp
 *
 * Processor for neutron scintillator detectors
 */

#ifndef __NEUTRONPROCESSOR_HPP_
#define __NEUTRONPROCESSOR_HPP_

#include "EventProcessor.hpp"

class NeutronProcessor : public EventProcessor
{
public:
    NeutronProcessor();
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
};

#endif // __NEUTRONPROCSSEOR_HPP_
