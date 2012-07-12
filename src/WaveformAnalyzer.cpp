
/*************************************
This code will call the functions to 
calculate various high resolution timing
parameters of the traces.

S.V. Paulauskas 16 July 2009

*************************************/
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
	
        if(trace.HasValue("saturation")) {
	    EndAnalyze();
	    return;
	}
	
	unsigned int waveformLow = GetConstant("waveformLow");
	unsigned int waveformHigh = GetConstant("waveformHigh");
	unsigned int startDiscrimination = 
	    GetConstant("startDiscrimination");
	unsigned int maxPos = trace.FindMaxInfo();

	trace.DoQDC(maxPos-waveformLow, 
		    waveformHigh+waveformLow);
	//Temporarly removed due to SegFault Issues
	// if(detSubtype == "liquid")
	//     trace.DoDiscrimination(startDiscrimination, 
	// 			   waveformHigh - startDiscrimination);
    } //if(detType
    EndAnalyze();
}
