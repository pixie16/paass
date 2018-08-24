/** \file TraceDump_PSMPT.hpp
 * \brief Class to Dump the PSPMT traces into a ROOT file for further analysis
 * \author D. P.Loureiro
 * \date July 24, 2018
 */

#ifndef __TRACEDUMP_PSPMT_HPP_
#define __TRACEDUMP_PSPMT_HPP_

#include "EventProcessor.hpp"

#ifdef useroot
#include <TFile.h>
#include <TTree.h>
#include <TH2I.h>
#include <TH1I.h>
#endif


//! A class that handles a simple timing experiment
class TraceDump_PSPMT : public EventProcessor {
 public:
    /** Default Constructor */
    TraceDump_PSPMT();

    /** Default Destructor */
    ~TraceDump_PSPMT();


    /** Performs the main processsing, which may depend on other processors
    * \param [in] event : the event to process
    * \return true if processing was successful */
    bool Process(RawEvent &event);

private: 


      Double_t leftTimeStamp;
      Double_t rightTimeStamp;

#ifdef useroot
    /** Method to setup the ROOT output, tree and histograms */
  void SetupRootOutput(void);
  TFile *prootfile_; //! pointer to root file
  TTree *proottree_; //! pointer to root tree
  TH1I *codes;
#endif

};
#endif // __TRACEDUMP_PSPMT_HPP_
