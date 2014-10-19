/** \file NeutronScintProcessor.hpp
 *
 * Processor for neutron scintillator detectors
 */

#ifndef __NEUTRONSCINTPROCESSOR_HPP_
#define __NEUTRONSCINTPROCESSOR_HPP_

#include "EventProcessor.hpp"

class NeutronScintProcessor : public EventProcessor
{
public:
    NeutronScintProcessor();
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
};

#endif // __NEUTRONSCINTPROCSSEOR_HPP_
