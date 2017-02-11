/** \file CfdAnalyzer.cpp
 * \brief Uses a Digital CFD to obtain waveform phases
 *
 * This code will obtain the phase of a waveform using a digital CFD.
 * Currently the only method is a polynomial fit to the crossing point.
 * For 100-250 MHz systems, this is not going to produce good timing.
 * This code was originally written by S. Padgett.
 *
 * \author S. V. Paulauskas
 * \date 22 July 2011
 */
#include <iostream>
#include <vector>
#include <utility>

#include "CfdAnalyzer.hpp"
#include "PolynomialCfd.hpp"
#include "TraditionalCfd.hpp"

using namespace std;

CfdAnalyzer::CfdAnalyzer(const std::string &s) : TraceAnalyzer() {
    name = "CfdAnalyzer";
    if (s == "polynomial" || s == "poly")
        driver_ = new PolynomialCfd();
    else if (s == "traditional" || s == "trad")
        driver_ = new TraditionalCfd();
    else
        driver_ = NULL;

}

void CfdAnalyzer::Analyze(Trace &trace, const std::string &detType,
                          const std::string &detSubtype,
                          const std::map<std::string, int> &tagMap) {
    TraceAnalyzer::Analyze(trace, detType, detSubtype, tagMap);

    if (!driver_) {
        EndAnalyze();
        return;
    }

    if (trace.IsSaturated() || trace.empty() ||
        trace.GetWaveform().empty()) {
        EndAnalyze();
        return;
    }

    const pair<double, double> pars =
            Globals::get()->GetCfdPars(detType + ":" + detSubtype);

    trace.SetPhase(driver_->CalculatePhase(trace.GetTraceSansBaseline(),
                                           pars,
                                           trace.GetExtrapolatedMaxInfo(),
                                           trace.GetBaselineInfo()));
    EndAnalyze();
}
