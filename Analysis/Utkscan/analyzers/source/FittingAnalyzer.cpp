/** \file FittingAnalyzer.cpp
 * \brief Uses a chi^2 minimization to fit waveforms
 *
 * Obtains the phase of a waveform using a Chi^2 fitting algorithm
 * implemented through the GSL libraries. We have now set up two different
 * functions for this processor. One of them handles the fast SiPMT signals,
 * which tend to be more Gaussian in shape than the standard PMT signals.
 *
 * \author S. V. Paulauskas
 * \date 22 July 2011
 *
 * @TODO This currently doesn't actually set the values for the GSL fitter
 * since we have it set as a TimingDriver type. We'll have to figure out how
 * to address that.
 */
#include <algorithm>
#include <iostream>
#include <vector>

#include "FittingAnalyzer.hpp"
#include "GslFitter.hpp"
#include "RootFitter.hpp"

using namespace std;

FittingAnalyzer::FittingAnalyzer(const std::string &s) {
    name = "FittingAnalyzer";
    type_=s;
    if (s == "GSL" || s == "gsl") {
        driver_ = new GslFitter();
    } 
    else if (s == "ROOT" || s == "root") {
        driver_ = new RootFitter();
std::cout<<"ROOT fitter selected"<<std::endl;
    }
}

FittingAnalyzer::~FittingAnalyzer() {
    delete driver_;
}

void FittingAnalyzer::Analyze(Trace &trace, const std::string &detType,
                              const std::string &detSubtype,
                              const std::map<std::string, int> &tagMap) {
    TraceAnalyzer::Analyze(trace, detType, detSubtype, tagMap);

    if (!driver_) {
        EndAnalyze();
        return;
    }

    if (trace.IsSaturated() || trace.empty() ||
    //if (trace.empty() ||
        trace.GetWaveform().size() == 0) {
        EndAnalyze();
        return;
    }

    Globals *globals = Globals::get();

    //We need to check and make sure that we don't need to use the timing
    // functions for the SiPM fast signals
   bool isFastSiPm = detType == "beta" && detSubtype == "double"
                      && tagMap.find("timing") != tagMap.end();
   
    //bool isFastSiPm = detType == "pulser"
    //                  && tagMap.find("timing") != tagMap.end();


    bool isSlowSiPm = detType == "pulser"
                      && tagMap.find("slow") != tagMap.end();

    bool isArraySiPm = detType == "pulser"
                      && tagMap.find("array") != tagMap.end();

    //std::cout<<"isFastSiPm "<<isFastSiPm <<std::endl;


    if (!isFastSiPm || !isSlowSiPm) {
        if (trace.GetBaselineInfo().second > globals->sigmaBaselineThresh()) {
            EndAnalyze();
            return;
        }
    } else {
        if (trace.GetBaselineInfo().second > globals->siPmtSigmaBaselineThresh()) {
            EndAnalyze();
            return;
        }
    }

    pair<double, double> pars = globals->fitPars(detType + ":" + detSubtype);
    
   if(type_=="GSL" || type_=="gsl"){
    GslFitter *theFitter=static_cast<GslFitter*>(driver_);

    if (isFastSiPm){
        pars = globals->fitPars(detType + ":" + detSubtype + ":fasttiming");
        //DPL: we tell the driver to use the corresponding fitting function
        theFitter->SetIsFastSiPm(true); 
    }
    else if(isSlowSiPm){
     pars = globals->fitPars(detType + ":" + detSubtype + ":timing");
        //DPL: we tell the driver to use the corresponding fitting function
        //std::cout<<pars.first<<" "<<pars.second<<std::endl;
        theFitter->SetIsSlowSiPm(true);  
    }
    else if(isArraySiPm){
     pars = globals->fitPars(detType + ":" + detSubtype + ":timing");
        //DPL: we tell the driver to use the corresponding fitting function
        //std::cout<<pars.first<<" "<<pars.second<<std::endl;
        theFitter->SetIsArraySiPm(true);  
    }

    }
else if(type_=="ROOT"|| type_=="root"){
  RootFitter *theFitter=static_cast<RootFitter*>(driver_);
}
    driver_->SetQdc(trace.GetQdc());
    double phase = driver_->CalculatePhase(trace.GetWaveform(),
                                           pars, trace.GetMaxInfo(),
                                           trace.GetBaselineInfo());
    //DPL Print out of phase
    //std::cout<<"phase "<<phase<<" "<<trace.GetMaxInfo().first <<std::endl;
    //std::cout<<"pars "<<pars.first<<" "<<pars.second <<std::endl;
    trace.SetPhase(phase + trace.GetMaxInfo().first);
    GslFitter *myFitter=static_cast<GslFitter*>(driver_);
    trace.SetChiSquareDof(myFitter->GetChiSqPerDof());

    EndAnalyze();
}
