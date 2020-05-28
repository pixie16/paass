///@file MtasPspmtProcessor.cpp
///@brief Processes information from a series of SiPMs to determine position.
///@author D.McKinnon, A. Keeler, S. Go, S. V. Paulauskas
///@date February 21, 2019

#ifndef _MTASPSPMTPROCESSOR_HPP_
#define _MTASPSPMTPROCESSOR_HPP_

#include <string>
#include <map>
#include <algorithm>
#include <climits>
#include <cmath>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <fstream>

#include "EventProcessor.hpp"
#include "RawEvent.hpp"
#include "PaassRootStruct.hpp"


/// Class to handle processing of MTAS implant and beam diagnostics detector
class MtasPspmtProcessor : public EventProcessor {
public:

    ///Default Constructor */
    MtasPspmtProcessor(void){};

    ///@brief Constructor that sets the scale and offset for histograms
    ///@param[in] scale: The multiplicative scaling factor
    ///@param[in] offset: The additive offset for the histogram
    MtasPspmtProcessor(const double &impl_scale, const unsigned int &impl_offset,
                        const double &impl_threshold, const double &diag_scale, const unsigned int &diag_offset,
                       const double &diag_threshold);

    ///Default Destructor
    ~MtasPspmtProcessor() {};

    ///Declare the plots used in the analysis
    void DeclarePlots(void);

    ///Preprocess the PSPMT data
    ///@param [in] event: the event to preprocess
    ///@return true if successful */
    bool PreProcess(RawEvent &event);

    private:

    double implPosScale_; ///< The implant detector scale that we need for the DAMM output
    unsigned int implPosOffset_; ///< The implant detector offset that we need for the the DAMM output
    double implThreshold_; ///< The implant detector threshold that the energy calculated by
    ///< the Pixie-16 trapezoidal filter needs to reach
    ///< before we can analyze the signals.
    double diagPosScale_; ///< The diagnostic detector scale that we need for the DAMM output
    unsigned int diagPosOffset_; ///< The diagnostic detector offset that we need for the the DAMM output
    double diagThreshold_; ///< The diagnostic detector threshold that the energy calculated by
    ///< the Pixie-16 trapezoidal filter needs to reach
    ///< before we can analyze the signals.

    processor_struct::MTASPSPMT MTASPSstruct; //!< PSPMT root Struct
    unsigned long evtNum_ = 0.0;
    int bin_num = 0;
    double first_time;
    const int geThreshold = 10;
    double first_time_beta;
    double delta_beta_T;
    double currentTime;
    double currentTimegamma = 0;
    bool beta_ev;

    
    //bool beta_ev,gamma_ev,dual_ev;


};
#endif //_MTASPSPMTPROCESSOR_HPP_
