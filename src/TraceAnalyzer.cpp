/**    \file TraceAnalyzer.cpp
 *     \brief defines the Trace class.
 *
 *     Implements a quick online trapezoidal filtering mechanism
 *     for the identification of double pulses
 *
 *     - SNL - 7-2-07 - created
 *     - SNL - 2-4-08 - Add plotting spectra
 */

#include <iostream>
#include <numeric>
#include <vector>

#include <cstdlib>

#include "damm_plotids.h"
#include "RawEvent.h"
#include "StatsAccumulator.h"
#include "TraceAnalyzer.h"

using std::accumulate;
using std::cout;
using std::endl;

/**
 * Initialize the trace analysis class.  Set the row numbers
 * for spectra 850 and 870 to zero and reserve enough memory
 * for the filter vectors.
 */
int TraceAnalyzer::Init()
{    
    const int maxTraceLength = 3000;

    rownum850 = 0;
    rownum870 = 0;

    // reserves enough space in the vector to contain a reasonable sized trace
    // This improves the performance of the program as a whole since memory
    // does not need to be reallocated every time the size is increased.

    average.reserve(maxTraceLength);
    v40.reserve(maxTraceLength);
    v50.reserve(maxTraceLength);
    v60.reserve(maxTraceLength);
    v70.reserve(maxTraceLength);
    flt.reserve(maxTraceLength);

    return(0);
}

/**
 * Nothing is done in the Trace constructor
 */
TraceAnalyzer::TraceAnalyzer() : 
    userTime(0.), systemTime(0.)
{
    clocksPerSecond = sysconf(_SC_CLK_TCK);
}

/** Output time processing traces */
TraceAnalyzer::~TraceAnalyzer() 
{
    cout << "Trace processor : " 
	 << userTime << " user time, "
	 << systemTime << " system time" << endl;
}

/**
 * Function to quickly analyze a trace online.  Trapezoidal filters
 * are run across the trace with varying parameters to extract E1
 * and E2 values and time differences
 *
 * detType and detSubtype not currently used
 */
int TraceAnalyzer::Analyze(const vector<int> &trace, vector<double> &values, 
		   const string &detType, const string &detSubtype)
{
    tms tmsBegin;

    times(&tmsBegin); // begin timing process
    
    //values.clear();
    average.clear();
    v40.clear();
    v50.clear();
    v60.clear();
    v70.clear();
    // quick trace analysis adapted from previous scan versions in
    // the xia_trace99.f file.
    const int baseLow = 5, baseHigh = 35;
    
    StatsAccumulator stats = accumulate
	(&trace[baseLow], &trace[baseHigh], StatsAccumulator());
    double basel  = stats.GetMean();
    // double sbasel = stats.GetStdDev();
    // sigma not used

    // make a trace of the running average
    const int averageLen = 10;
    // a lot of repeated arithmetic here too
    //? start running average on interval [1,averagelen]
    //    as opposed to [0,averaglen)
    for(int i = averageLen; i< int(trace.size()-20); i++){
	stats = accumulate
	    (&trace[i-averageLen + 1], &trace[i+1], StatsAccumulator());
	average.push_back(stats.GetMean() - basel);
    }
    
    // determine trace filters, these are trapezoidal filters
    // characterized by a risetime and gaptime and a range of
    // the filter from lo to hi.
    // filter(trace, lo, hi, gaptime, risetime);
    int high;
    high = trace.size()-30;
    
    FilterFill(trace, v40, 20, high, 4, 4);
    FilterFill(trace, v50, 40, high, 10, 10);
    FilterFill(trace, v60, 40, high, 20, 10);
    // "magic" numbers follow

    // find the point at which the trace crosses the threshold
    thrtime1 = 0;
    e1 = 0;
    
    const int thre  = 50;
    const int thre2 = 15;
    
    int i = 20;
    /* alternatively
       vector<int>::iterator iThr  = v40.begin();
       vector<int>::iterator iHigh = v40.end() - 30; 
       while (true) {
       iThr = find_if(iThr, iHigh, bind2nd(greater<int>(), thre));
       if (iThr != iHigh) {
       // check that the correlated energy is sufficiently high
       thrtime1 = iThr - v40.begin();
       if (v60[thrtime1 + 20]/10. > thre2) {
       thrtime1 = iThr - v40.begin();
       e1 = v50[thrtime1 + 15] / 10.;
       break;
       }
       } else {
       thrtime1 = -1;
       }
       }
    */
    while(thrtime1 == 0) {
	if(v40[i] > thre && v60[i+20]/10. > thre2){
	    thrtime1 = i;
	    e1 = v50[thrtime1 + 15]/10.;
	}
	
	i++;
	if(i>=high) thrtime1 = -1; //stop loop if nothing found in trace
    }
    
    // find a second crossing point
  thrtime2 = 0;
  e2 = 0;
  
  i = thrtime1 + 20;
  if(/*thrtime1*/i < int(trace.size()) && thrtime1 != -1){
    while(thrtime2 == 0){
      if(v40[i] > thre && (v60[i+20]/10. - average[i-5]) > thre2){
	thrtime2 = i;
	if(thrtime2 < int(trace.size()-10)){
	  e2 = v50[thrtime2+15]/10;
	}
      }
      
      i++;
      if(i>=high/*int(trace.size()-10)*/) thrtime2 = -1;
    }
  }

  values.push_back(e1);
  values.push_back(thrtime1);
  values.push_back(e2);
  values.push_back(thrtime2);

  TracePlot(trace,values);
  
  tms tmsEnd;
  times(&tmsEnd);
  userTime += (tmsEnd.tms_utime - tmsBegin.tms_utime) / clocksPerSecond;
  systemTime += (tmsEnd.tms_stime - tmsBegin.tms_stime) / clocksPerSecond; 

  return(0);
}

/**
 * Defines how to implement a trapezoidal filter characterized by two
 * moving sum windows of width risetime separated by a length gaptime.
 * Filter is calculated from channels lo to hi.
 */
vector<int> TraceAnalyzer::Filter(vector<int> &trace, int lo, int hi, 
			  int gapTime, int riseTime){
  flt.clear();

  if (lo != 0) {
      for(int i = 0; i < lo; i++) {
	  flt.push_back(0); 
      }
  }

  // repeated arithmetic here too
  for(int i = lo; i < hi; i++){
    int leftSum  = 0;
    int rightSum = 0;
    
    for (int j = i - 2*riseTime - gapTime; j < i - riseTime - gapTime; j++) {
	leftSum += trace[j];
    }

    for (int j = i - riseTime; j < i; j++) {
	rightSum += trace[j];
    }

    flt.push_back(rightSum - leftSum);
  }

  return flt;
}

/**
 * same thing as Filter() but fill the storage directly without passing 
 * through the temporary return value
 */
void TraceAnalyzer::FilterFill(const vector<int> &trace, vector<int> &res,
			int lo, int hi, int gapTime, int riseTime){
    res.assign(lo, 0);

    for (int i = lo; i < hi; i++) {
	int leftSum = accumulate(&trace[i - 2*riseTime - gapTime],
				 &trace[i - riseTime - gapTime], 0);
	int rightSum = accumulate(&trace[i - riseTime], &trace[i], 0);

	res.push_back(rightSum - leftSum);
    }
}

/** declare the damm plots */
void TraceAnalyzer::DeclarePlots() const
{
    using namespace dammIds::trace;

    const int traceBins = SA;
    const int numTraces = S5;
    const int energyBins = SC;
    const int energyBins2 = S9;
    const int timeBins = SA;

    DeclareHistogram2D(DD_TRACE, traceBins, numTraces, "traces data");
    DeclareHistogram2D(DD_FILTER1, traceBins, numTraces, "v40");    
    DeclareHistogram2D(DD_FILTER2, traceBins, numTraces, "v50");
    DeclareHistogram2D(DD_FILTER3, traceBins, numTraces, "v60");
    DeclareHistogram2D(DD_AVERAGE_TRACE, traceBins, numTraces, "trace average");

    DeclareHistogram1D(D_ENERGY1, energyBins, "E1 from traces");    
    DeclareHistogram1D(D_ENERGY2, energyBins, "E2 from traces");
    // DeclareHistogram1D(867, SC, "Decays after recoils");
    DeclareHistogram2D(DD_TRACE_COND, traceBins, numTraces, "double traces dT>250 ns");
    DeclareHistogram2D(DD_ENERGY2__TDIFF, energyBins2, timeBins, "E2 vs T2", 2);
    DeclareHistogram2D(DD_ENERGY2__ENERGY1, energyBins2, energyBins2, "E2 vs E1", 2);
    DeclareHistogram2D(DD_ENERGY2__ENERGY1_COND, energyBins2, energyBins2,
		       "E2 vs E1 - dT>250 ns", 2);
    DeclareHistogram2D(DD_ENERGY2__TDIFF_COND, energyBins2, timeBins,
		       "E2 vs T2 - dT>250 ns", 2);
}

/**
 * From the trace analysis plot the damm spectra.
 *   including the first few traces of the analysis
 *   and their associated filters.
 *   as well as  E2 v E1 and E2 v time difference plots with
 *   varying conditions for double pulses.
 */
void TraceAnalyzer::TracePlot(const vector<int> &trace, 
			      const vector<double> &values)
{
    //? shouldn't we have values directly from the analyze function w/o passing as an argument    
    double e1,t1,e2,t2;
    e1 = values[0];
    t1 = values[1];
    e2 = values[2];
    t2 = values[3];
    
    using namespace dammIds::trace;

    // plot trace and associated filters and derived energies
    for(unsigned int i = 0; i < trace.size(); i++){
	if(i < average.size() )
	    plot(DD_AVERAGE_TRACE,i,rownum850,average[i]);
	if(i < v60.size() ) 
	    plot(DD_FILTER3,i,rownum850,abs(v60[i]));
	if(i < v50.size() ) 
	    plot(DD_FILTER2,i,rownum850,abs(v50[i]));
	if( i < v40.size() )
	    plot(DD_FILTER1,i,rownum850,abs(v40[i]));
	plot(DD_TRACE,i,rownum850,trace[i]);
    }
    
    plot(DD_TRACE,trace.size()+10,rownum850,e1);
    plot(DD_TRACE,trace.size()+11,rownum850,t1);
    if(e2>0)
	plot(DD_TRACE,trace.size()+12,rownum850,e2);
    if(t2>0)
	plot(DD_TRACE,trace.size()+13,rownum850,t2);
    rownum850++;
    
    // plot double pulse E and T spectra
    plot(D_ENERGY1,e1);
    if(e2 > 0) {
	plot(D_ENERGY2,e2);
	plot(DD_ENERGY2__TDIFF,e2,t2-t1);
	plot(DD_ENERGY2__ENERGY1,e2,e1);
	// more magic numbers
	// but pulses separated in time with sufficient energy of 2nd pulse
	if((t2 - t1) > 22 && (e2 > 60)  && (t2<700) ){
	    plot(DD_ENERGY2__ENERGY1_COND,e2,e1);
	    plot(DD_ENERGY2__TDIFF_COND,e2,t2-t1);
	    for (size_t i = 0; i < trace.size(); i++) {
		plot(DD_TRACE_COND,i,rownum870,trace[i]);
	    }
	    plot(DD_TRACE_COND,trace.size()+10,rownum870,e1);
	    plot(DD_TRACE_COND,trace.size()+11,rownum870,t1);
	    plot(DD_TRACE_COND,trace.size()+12,rownum870,e2);
	    plot(DD_TRACE_COND,trace.size()+13,rownum870,t2);
	    
	    rownum870++;
	}
    }
}
