/** \file RootProcessor.hpp 
 *
 * Processor to dump data from events into a root tree
 * This loops over other event processor to fill appropriate branches
 */

#ifndef useroot
#error USEROOT must be defined to use RootProcessor
#endif

#ifndef __ROOTPROCESSOR_HPP_
#define __ROOTPROCESSOR_HPP_

#include <vector>

#include "EventProcessor.hpp"

// forward declaration
class TFile;

using std::vector;

class RootProcessor : public EventProcessor
{
 private:
    TFile *file; //< File where tree is stored
    TTree *tree; //< ROOT tree where event branches are filled

    /// All processors with AddBranch() information
    vector<EventProcessor *> vecProcess;
 public:
    RootProcessor(const char *fileName, const char *treeName);
    virtual bool Init();
    virtual bool Process(RawEvent &event);
    virtual ~RootProcessor();
};

#endif // __ROOTPROCESSOR_HPP_
