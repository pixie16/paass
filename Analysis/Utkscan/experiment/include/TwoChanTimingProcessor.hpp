/** \file TwoChanTimingProcessor.hpp
 * \brief Class to perform simple two channel timing experiments
 * \author S. V. Paulauskas
 * \date July 10, 2009
 */

#ifndef __TWOCHANTIMINGPROCESSOR_HPP_
#define __TWOCHANTIMINGPROCESSOR_HPP_

#include "EventProcessor.hpp"

#ifdef useroot
#include <TFile.h>
#include <TTree.h>
#include <TH2I.h>
#include <TH1I.h>
#endif


//! A class that handles a simple timing experiment
class TwoChanTimingProcessor : public EventProcessor {
 public:
    /** Default Constructor */
    TwoChanTimingProcessor();

    /** Default Destructor */
    ~TwoChanTimingProcessor();


    /** Performs the main processsing, which may depend on other processors
    * \param [in] event : the event to process
    * \return true if processing was successful */
    bool Process(RawEvent &event);

private: 
  double StartTimeStamp;
  double StopTimeStamp;
  int StartMaxValue;
  int StopMaxValue;
  double StartChiSq;
  double StopChiSq;



#ifdef useroot
    /** Method to setup the ROOT output, tree and histograms */
  void SetupRootOutput(void);
  TFile *prootfile_; //! pointer to root file
  TTree *proottree_; //! pointer to root tree
  TH1I *codes;
  TH2I *traces;
  TH2I *traces_stop;
  TH2I *superpulse_start;
  TH2I *superpulse_stop;
#endif

};
#endif // __TWOCHANTIMINGPROCESSOR_HPP_
