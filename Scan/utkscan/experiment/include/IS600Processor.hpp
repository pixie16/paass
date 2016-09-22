/** \file IS600Processor.hpp
 * \brief A class to process data from ISOLDE 599 and 600 experiments using
 * VANDLE
 *
 *\author S. V. Paulauskas
 *\date July 14, 2015
 */
#ifndef __IS600PROCESSOR_HPP_
#define __IS600PROCESSOR_HPP_
#include <fstream>

#include "EventProcessor.hpp"

#ifdef useroot
#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>
#include <TH1D.h>
#endif

/// Class Analyze for ISOLDE experiments 599 and 600
class IS600Processor : public EventProcessor {
public:
    /** Default Constructor */
    IS600Processor();
    /** Default Destructor */
    ~IS600Processor();
    /** Declare the plots used in the analysis */
    virtual void DeclarePlots(void);

    /** PreProcess does nothing since this is solely dependent on results
     from other Processors*/
    virtual bool PreProcess(RawEvent &event);

    /** Process the event
    * \param [in] event : the event to process
    * \return Returns true if the processing was successful */
    virtual bool Process(RawEvent &event);
private:
#ifdef useroot
    TFile *rootfile_; //!< the root file to be used for output
    TTree *roottree_; //!< the root tree to be used for storing data
    TH2D *qdctof_; //!< a 2D histogram in ROOT
    TH1D *vsize_; //!< a 1D histogram in root
#endif
    std::ofstream *outstream; //!< filestream to output to text file
};
#endif
