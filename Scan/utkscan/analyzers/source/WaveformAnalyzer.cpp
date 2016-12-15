/** \file WaveformAnalyzer.cpp
 *\brief Preliminary waveform analysis
 *
 *Does preliminary waveform analysis on traces. The parameters set here
 *will be used for the high resolution timing algorithms to do their thing.
 *
 *\author S. V. Paulauskas
 *\date July 16, 2009
*/
#include <numeric>
#include <string>

#include <cmath>

#include "WaveformAnalyzer.hpp"

using namespace std;

enum WAVEFORMANALYZER_ERROR_CODES{
    TOO_LOW,
    MAX_END,
    LOW_GREATER_HIGH
};

WaveformAnalyzer::WaveformAnalyzer() : TraceAnalyzer() {
    name = "WaveformAnalyzer";
    messenger_ = new Messenger();
}

void WaveformAnalyzer::Analyze(Trace &trace, const std::string &type,
                               const std::string &subtype,
                               const std::map<std::string, int> &tags) {
    TraceAnalyzer::Analyze(trace, type, subtype, tags);
    trc_ = &trace;

    if (trace.HasValue("saturation") || trace.size() == 0) {
        EndAnalyze();
        return;
    }

    mean_ = mval_ = 0;

    g_ = Globals::get();

    pair<unsigned int, unsigned int> range =
            g_->waveformRange(type + ":" + subtype);

    if (type == "beta" && subtype == "double" &&
        tags.find("timing") != tags.end())
        range = g_->waveformRange(type + ":" + subtype + ":timing");

    try {
        //First we find the waveform in the trace
        FindWaveform(range.first, range.second);
        //Calculate the baseline, need to know where waveform is before this point
        CalculateSums();
        //If we had something tagged for additional trace analysis.
        if (tags.find("psd") != tags.end())
            CalculateDiscrimination(g_->discriminationStart());
    } catch(WAVEFORMANALYZER_ERROR_CODES errorCode) {
        switch(errorCode) {
            case TOO_LOW:
                messenger_->warning("The low bound for the search was before "
                                            "the beginning of the trace. This"
                                            " is a bad thing, no trace "
                                            "analysis possible.", 0);
                break;
            case MAX_END:
                messenger_->warning("The maximum value of the trace was found"
                                            " at a point where your current "
                                            "waveform range will be outside "
                                            "of the trace. you should "
                                            "reevaluate the waveform range to"
                                            " make sure that you have set "
                                            "something reasonable. I suggest "
                                            "taking a look at the scope "
                                            "program to view the traces. ", 0);
                break;
            case LOW_GREATER_HIGH:
                messenger_->warning("The high bound for the waveform search "
                                            "was lower than the low bound. "
                                            "This should never have happened "
                                            "and I have no idea why it did.",
                                    0);
                break;
            default:
                stringstream ss;
                ss << "There was an unidentified error with an error code of "
                   << errorCode << ". Please review your settings for the "
                        "trace analysis.";
                messenger_->warning(ss.str(),0);
                break;
        }
        EndAnalyze();
    }
    EndAnalyze();
}


