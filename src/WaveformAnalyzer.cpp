/** \file WaveformAnalyzer.cpp 
 *\brief Preliminary waveoform analysis
 *
 *Does preliminary waveform analysis on traces. The parameters set here
 *will be used for the high resolution timing algorithms to do their thing. 
 *
 *\author S. V. Paulauskas 
 *\date 16 July 2009
*/
#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>

#include <cmath>

#include "FittingAnalyzer.hpp"
#include "StatsAccumulator.hpp"
#include "WaveformAnalyzer.hpp"

using namespace std;

//********** WaveformAnalyzer **********
WaveformAnalyzer::WaveformAnalyzer() : TraceAnalyzer() 
{
    name = "Waveform";
}


//********** DeclarePlots **********
void WaveformAnalyzer::DeclarePlots(void) const
{
}


//********** Analyze **********
void WaveformAnalyzer::Analyze(Trace &trace,
			       const string &detType, 
			       const string &detSubtype)
{
    TraceAnalyzer::Analyze(trace, detType, detSubtype);
    
    if(detType == "vandleSmall" || detType == "vandleBig" 
       || detType == "scint" || detType == "pulser" 
       || detType == "tvandle") {

	unsigned int maxPos = trace.FindMaxInfo();

        if(trace.HasValue("saturation")) {
	    EndAnalyze();
	    return;
	}
	
	unsigned int waveformLow = GetConstant("waveformLow");
	unsigned int waveformHigh = GetConstant("waveformHigh");
	unsigned int startDiscrimination = 
	    GetConstant("startDiscrimination");

	trace.DoQDC(maxPos-waveformLow, 
		    waveformHigh+waveformLow);

	if(detSubtype == "liquid")
	    trace.DoDiscrimination(startDiscrimination, 
				   waveformHigh - startDiscrimination);
    } //if(detType
    EndAnalyze();
}
