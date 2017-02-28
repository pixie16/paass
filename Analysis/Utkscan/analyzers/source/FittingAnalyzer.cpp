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

#ifdef USE_ROOT
#include "RootFitter.hpp"
#endif

using namespace std;

FittingAnalyzer::FittingAnalyzer(const std::string &s) {
    name = "FittingAnalyzer";
    if (s == "GSL" || s == "gsl")
        driver_ = new GslFitter();
#ifdef USE_ROOT
    else if (s == "ROOT" || s == "root")
        driver_ = new RootFitter();
#endif
    else
        driver_ = NULL;
}

FittingAnalyzer::~FittingAnalyzer() {
    delete driver_;
}

void FittingAnalyzer::Analyze(Trace &trace, const std::string &detType,
                              const std::string &detSubtype,
                              const std::map<std::string, int> &tagMap) {
    TraceAnalyzer::Analyze(trace, detType, detSubtype, tagMap);

    if (!driver_)
        throw invalid_argument("FittingAnalyzer::Analyze : A fitting driver "
                                       "was not provided. This is a fatal "
                                       "error.");

    if (trace.IsSaturated() || trace.empty() || trace.GetWaveform().empty()) {
        EndAnalyze();
        return;
    }

    Globals *globals = Globals::get();

    //We need to check and make sure that we don't need to use the timing
    // functions for the SiPM fast signals
    bool isFastSiPm = detType == "beta" && detSubtype == "double"
                      && tagMap.find("timing") != tagMap.end();

    if (!isFastSiPm) {
        if (trace.GetBaselineInfo().second >
            globals->GetSigmaBaselineThresh()) {
            EndAnalyze();
            return;
        }
    } else {
        if (trace.GetBaselineInfo().second >
            globals->GetSiPmSigmaBaselineThresh()) {
            EndAnalyze();
            return;
        }
    }

    pair<double, double> pars = globals->GetFitPars(detType + ":" + detSubtype);
    if (isFastSiPm)
        pars = globals->GetFitPars(detType + ":" + detSubtype + ":timing");

    driver_->SetQdc(trace.GetQdc());
    double phase = driver_->CalculatePhase(trace.GetWaveform(), pars,
                                           trace.GetMaxInfo(),
                                           trace.GetBaselineInfo());
    trace.SetPhase(phase + trace.GetMaxInfo().first);

    EndAnalyze();
}