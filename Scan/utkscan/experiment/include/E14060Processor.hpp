/** \file E14060Processor.hpp
 * \brief Experiment specific processor to handle the e14060 experiment at
 * the NSCL.
 *\author S. V. Paulauskas
 *\date September 15, 2016
 */
#ifndef __E14060PROCESSOR_HPP_
#define __E14060PROCESSOR_HPP_
#include <fstream>

#include "EventProcessor.hpp"

#ifdef useroot
#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>
#include <TH1D.h>
#endif

/// Class to analyze data from e14060
class E14060Processor : public EventProcessor {
public:
    /** Default Constructor */
    E14060Processor();
    /** Default Destructor */
    ~E14060Processor();
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
