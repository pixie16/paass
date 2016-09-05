/** \file WaveformAnalyzer.cpp
 *\brief Preliminary waveform analysis
 *
 *Does preliminary waveform analysis on traces. The parameters set here
 *will be used for the high resolution timing algorithms to do their thing.
 *
 *\author S. V. Paulauskas
 *\date 16 July 2009
*/
#include <numeric>
#include <string>

#include <cmath>

#include "WaveformAnalyzer.hpp"

using namespace std;

WaveformAnalyzer::WaveformAnalyzer() : TraceAnalyzer() {
    name = "WaveformAnalyzer";
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

    //First we find the waveform in the trace
    if (FindWaveform(range.first, range.second)) {
        //Calculate the baseline, need to know where waveform is before this point
        CalculateSums();
        //Now calculate the QDC since we need to subtract the baseline
        //CalculateQdc(range.first, range.second);
        //If we had a liquid
        if (tags.find("psd") != tags.end())
            CalculateDiscrimination(g_->discriminationStart());
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

        if (it > wrng_.first && it < wrng_.second) {
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
    for (Trace::iterator i = wrng_.first + lo; i <= wrng_.second; i++)
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
    if (low < trc_->begin()) {
        cerr << "The low bound to search was before the beginning of the "
                "trace. This is a bad thing, no trace analysis possible."
             << endl;
        return (false);
    }

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
    wrng_ = make_pair(tmp - lo, tmp + hi + 1);

    //Set the high range for the baseline calculation here.
    bhi_ = tmp - lo;

    //Tell the user that the requested waveform size was actually too long.
    // Set the end of the waveform to be the end of the trace.
    if (mpos + hi > trc_->size()) {
        cerr << "The max is too close to the end of the trace."
             << endl;
        wrng_.second = trc_->end();
    }

    //If the maximum value was greater than the bit resolution of the ADC then
    // we had a saturation and we need to set the saturation flag.
    if (mval_ >= g_->bitResolution())
        trc_->InsertValue("saturation", 1);
    return (true);
}
