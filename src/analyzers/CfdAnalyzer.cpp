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
#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "CfdAnalyzer.hpp"

using namespace std;

CfdAnalyzer::CfdAnalyzer() : TraceAnalyzer() {
    name = "CfdAnalyzer";
}

void CfdAnalyzer::Analyze(Trace &trace, const std::string &detType,
                          const std::string &detSubtype) {
    TraceAnalyzer::Analyze(trace, detType, detSubtype);
    Globals *globals = Globals::get();
    unsigned int saturation = (unsigned int)trace.GetValue("saturation");
    if(saturation > 0) {
            EndAnalyze();
            return;
    }
    double aveBaseline = trace.GetValue("baseline");
    unsigned int maxPos = (unsigned int)trace.GetValue("maxpos");
    unsigned int waveformLow  = (unsigned int)globals->waveformRange().first;
    unsigned int waveformHigh = (unsigned int)globals->waveformRange().second;
    unsigned int delay = 2;
    double fraction = 0.25;
    vector<double> cfd;
    Trace::iterator cfdStart = trace.begin();
    advance(cfdStart, (int)(maxPos - waveformLow - 2));
    Trace::iterator cfdStop  = trace.begin();
    advance(cfdStop, (int)(maxPos + waveformHigh));
    for(Trace::iterator it = cfdStart;  it != cfdStop; it++) {
            Trace::iterator it0 = it;
            advance(it0, delay);
            double origVal = *it;
            double transVal = *it0;
            cfd.insert(cfd.end(), fraction *
                       (origVal - transVal - aveBaseline));
    }
    vector<double>::iterator cfdMax =
        max_element(cfd.begin(), cfd.end());
    vector<double> fitY;
    fitY.insert(fitY.end(), cfd.begin(), cfdMax);
    fitY.insert(fitY.end(), *cfdMax);
    vector<double>fitX;
    for(unsigned int i = 0; i < fitY.size(); i++)
        fitX.insert(fitX.end(), i);
    double num = fitY.size();
    double sumXSq = 0, sumX = 0, sumXY = 0, sumY = 0;
    for(unsigned int i = 0; i < num; i++) {
            sumXSq += fitX.at(i)*fitX.at(i);
            sumX += fitX.at(i);
            sumY += fitY.at(i);
            sumXY += fitX.at(i)*fitY.at(i);
    }
    double deltaPrime = num*sumXSq - sumX*sumX;
    double intercept =
        (1/deltaPrime)*(sumXSq*sumY - sumX*sumXY);
    double slope =
        (1/deltaPrime)*(num*sumXY - sumX*sumY);
    trace.InsertValue("phase", (-intercept/slope)+maxPos);
    EndAnalyze();
}
