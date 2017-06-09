/** \file RootProcessor.hpp
 * \brief Processor to dump data from events into a root tree
 *
 * This loops over other event processor to fill appropriate branches
 * @authors D. Miller
 */

#ifndef useroot
#error USEROOT must be defined to use RootProcessor
#endif

#ifndef __ROOTPROCESSOR_HPP_
#define __ROOTPROCESSOR_HPP_

#include <vector>

#include "EventProcessor.hpp"

#ifdef useroot

#include <TFile.h>

#endif

using std::vector;

//! A Class to handle outputting things into ROOT
class RootProcessor : public EventProcessor {
private:
    TFile *file; //!< File where tree is stored
    TTree *tree; //!< ROOT tree where event branches are filled

    /// All processors with AddBranch() information
    vector<EventProcessor *> vecProcess;
public:
    /** Constructor taking arguments for file and tree creation
    * \param [in] fileName : the file name for the root file
    * \param [in] treeName : the name of the tree to save to the root file*/
    RootProcessor(const char *fileName, const char *treeName);

    /** Initializes the processor
    * \param [in] rawev : the raw event to analyze
    * \return true if the initialization was successful */
    virtual bool Init(RawEvent &rawev);

    /** Performs the main processsing, which may depend on other processors
    * \param [in] event : the event to process
    * \return true if processing was successful */
    virtual bool Process(RawEvent &event);

    /** Default Destructor */
    virtual ~RootProcessor();
};

#endif // __ROOTPROCESSOR_HPP_
