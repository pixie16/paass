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
#include <sstream>
#include <vector>

#include "FittingAnalyzer.hpp"
#include "GslFitter.hpp"

#ifdef USE_ROOT

#include "RootFitter.hpp"

#endif

using namespace std;

FittingAnalyzer::FittingAnalyzer(const std::string &s ,const std::set<std::string> &ignoredTypes) {
    name = "FittingAnalyzer";
    if (s == "GSL" || s == "gsl")
        driver_ = new GslFitter();
#ifdef USE_ROOT
    else if (s == "ROOT" || s == "root")
        driver_ = new RootFitter();
#endif
    else {
        stringstream ss;
        ss << "FittingAnalyzer::FittingAnalyzer - The driver type \"" << s
           << "\" was unknown. Please choose a valid driver.";
        throw GeneralException(ss.str());
    }
    ignoredTypes_ = ignoredTypes; 
}

FittingAnalyzer::~FittingAnalyzer() {
    delete driver_;
}

bool FittingAnalyzer::IsIgnoredDetector(const ChannelConfiguration &id) {
    if (IsIgnored(ignoredTypes_, id)) {
        return true;
    } else {
        return false;
    }
}

void FittingAnalyzer::Analyze(Trace &trace, const ChannelConfiguration &cfg) {
    TraceAnalyzer::Analyze(trace, cfg);

    if (trace.IsSaturated() || trace.empty() || !trace.HasValidWaveformAnalysis() || IsIgnored(ignoredTypes_,cfg)) {
        trace.SetHasValidTimingAnalysis(false);
        EndAnalyze();
        return;
    }

    driver_->SetQdc(trace.GetQdc());

    if (cfg.GetType() == "beta" && cfg.GetSubtype() == "double" && cfg.HasTag("timing"))
        driver_->SetIsFastSiPm(true);

    trace.SetPhase(driver_->CalculatePhase(trace.GetWaveform(), cfg.GetFittingParameters(), trace.GetMaxInfo(),trace.GetBaselineInfo()) + trace.GetMaxInfo().first);
    trace.SetHasValidTimingAnalysis(true);
    EndAnalyze();
}