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

void WaveformAnalyzer::CalculateSums() {
    if (trc_->HasValue("baseline"))
        return;

    double sum = 0, qdc = 0;
    vector<double> w;
    double numBins = (double) (bhi_ - trc_->begin());
    mean_ = 0;
    for (Trace::iterator it = trc_->begin(); it != trc_->end(); it++) {
        sum += (*it);
        if (it < bhi_)
            mean_ += (*it) / numBins;

        if (it > waverng_.first && it < waverng_.second) {
            qdc += (*it) - mean_;
            w.push_back((*it) - mean_);
        }
    }

    //We need to perform one separate loop in order to calculate the standard
    // deviation of the baseline
    double accum = 0.0;
    for (Trace::iterator it = trc_->begin(); it != bhi_; it++)
        accum += (*it - mean_) * (*it - mean_);
    double stdev = sqrt(accum / (numBins));

    //Subtract the baseline from the full trace qdc
    sum -= mean_ * trc_->size();

    trc_->SetWaveform(w);
    trc_->InsertValue("tqdc", sum);
    trc_->InsertValue("qdc", qdc);
    trc_->SetValue("baseline", mean_);
    trc_->SetValue("sigmaBaseline", stdev);
    trc_->SetValue("maxval", mval_ - mean_);
}

void WaveformAnalyzer::CalculateDiscrimination(const unsigned int &lo) {
    int discrim = 0;
    for (Trace::iterator i = waverng_.first + lo; i <= waverng_.second; i++)
        discrim += (*i) - mean_;
    trc_->InsertValue("discrim", discrim);
}

bool WaveformAnalyzer::FindWaveform(const unsigned int &lo,
                                    const unsigned int &hi) {
    //high bound will be the trace delay
    Trace::iterator high = trc_->begin() + g_->traceDelay() /
                                           (g_->adcClockInSeconds() * 1e9);

    //if high bound is outside the trace then stop at the end of the trace
    if (trc_->end() < high)
        high = trc_->end();

    //low bound will be 15 + lo, we need at least 15 bins to calculate the
    //baseline, plus the requested risetime of the signal.
    Trace::iterator low = trc_->begin() + 15 + lo;

    //If low is less than 0 then we have some serious issues
    if (low < trc_->begin())
        throw(TOO_LOW);

    //Check that the low bound is less than the high bound. Not sure how this
    // could happen but it has in the past.
    if(high < low)
        throw(LOW_GREATER_HIGH);

    //Find the maximum value of the waveform in the range of low to high
    Trace::iterator tmp = max_element(low, high);

    //Calculate the position of the maximum of the waveform in trace
    int mpos = (int) (tmp - trc_->begin());

    //Set the value of the maximum of the waveform and insert the value into
    // the trace.
    mval_ = *tmp;
    trc_->InsertValue("maxpos", mpos);

    //Comparisons will be < to handle .end(), +1 here makes comparison <=
    //when we do not have the end().
    waverng_ = make_pair(tmp - lo, tmp + hi + 1);

    //Set the high range for the baseline calculation here.
    bhi_ = tmp - lo;

    //Tell the user that the requested waveform size was actually too long.
    // Set the end of the waveform to be the end of the trace.
    if (mpos + hi > trc_->size())
        throw(MAX_END);

    //If the maximum value was greater than the bit resolution of the ADC then
    // we had a saturation and we need to set the saturation flag.
    if (mval_ >= g_->bitResolution())
        trc_->InsertValue("saturation", 1);

    return (true);
}
