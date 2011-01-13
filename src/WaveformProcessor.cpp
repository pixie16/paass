/************************************
This code will obtain the phase of a trace
using either a Chi^2 fitting routine in GSL, 
or a Single Point Analysis. 

The data is passed into the raw event so that other
subroutines are able to access it. 
  
   S.V.P. 16 July 2009

*************************************/
#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <unistd.h> 

#include "damm_plotids.h"
#include "DetectorDriver.h"
#include "RawEvent.h"
#include "Trace.h"
#include "WaveformProcessor.h"
#include "StatsAccumulator.h"

#ifdef pulsefit
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_vector.h>
#endif

//A FEW MAGIC NUMBERS
#define WID 3.9626     //define gaussian width for fit 3.9626
#define DKAY 3.2334    //define decay constant for fit    3.2334
#define WAVEFORMLOW  2  //The starting position of the Waveform (referenced from max)
#define WAVEFORMHIGH 12 //The point when the Waveform returns to baseline (referenced from max)

extern "C" void count1cc_(const int &, const int &, const int &);
extern "C" void set2cc_(const int &, const int &, const int &, const int &);

void ngdiscrim(const Trace &trace, const double &traceQDC, const double &ave_baseline, const unsigned int &maxX);

#ifdef pulsefit
double fitroutine(const Trace &trace, const int &maxX, const double &ave_baseline, const double &s_dev_baseline);
int my_f(const gsl_vector *x, void *FitData, gsl_vector *f);
int expb_df(const gsl_vector *x, void *FitData, gsl_matrix *J);
int expb_fdf(const gsl_vector *x, void *FitData, gsl_vector *f, gsl_matrix *J);
#else
double spt_analysis(const Trace &trace, const unsigned int &maxX, const double &ave_baseline, const double &trcQDC, const int &counter);
#endif

using namespace dammIds::waveformprocessor;
using namespace std;

WaveformProcessor::WaveformProcessor(): EventProcessor()
{
    name = "FittingRoutine";
    associatedTypes.insert("scint"); 
    associatedTypes.insert("vandle");
    associatedTypes.insert("pulser");

    counter = 0;
}

void WaveformProcessor::DeclarePlots(void) const
{
//    DeclareHistogram1D(D_DISCRIM, SD, "N-Gamma Discrimination");
//    DeclareHistogram2D(DD_NGVSE, SE, SE,"N-G Discrim vs Energy");
}

//process trace 
bool WaveformProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event)) 
	return(false);
    
    static const DetectorSummary* vandleEvents = event.GetSummary("vandle", false);
    static const DetectorSummary* scintEvents = event.GetSummary("scint", false);
    static const DetectorSummary* pulserEvents = event.GetSummary("pulser", false);

    if(vandleEvents && scintEvents)
	if(vandleEvents->GetList().empty() || scintEvents->GetList().empty())
	    return(false);
    
    vector<ChanEvent*> allEvents;
    if(vandleEvents)
	allEvents.insert(allEvents.end(), vandleEvents->GetList().begin(), vandleEvents->GetList().end());
    if(scintEvents)
	allEvents.insert(allEvents.end(), scintEvents->GetList().begin(), scintEvents->GetList().end());
    if(pulserEvents)
	allEvents.insert(allEvents.end(), pulserEvents->GetList().begin(), pulserEvents->GetList().end());
    
    for(vector<ChanEvent*>::const_iterator it = allEvents.begin(); it != allEvents.end(); it++)
    {
	ChanEvent *chan = *it;
	// const unsigned int location = chan->GetChanID().GetLocation();
	const string subType = chan->GetChanID().GetSubtype();

	// const vector<int> &trace = chan->GetTraceRef();
	const Trace &trace = chan->GetTrace();

       	//initalize the variables to be passed to RawEvent
	chan->SetTrcQDC(-9999);
	chan->SetPhase(-9999);
	chan->SetMaxValue(-9999);
	chan->SetStdDevBaseline(-9999);
	chan->SetAveBaseline(-9999);
	chan->SetMaxPos(-9999);
	
	int satTraceCount = count(trace.begin(),trace.end(),4095);

	if(trace.empty()|| (satTraceCount > 0)) //SKIP IF NO TRACE OR SATURATION
	    continue;
	
/**** LOCATE THE WAVEFORM PEAK ****/
	Trace::const_iterator itTrace = max_element(trace.begin()+WAVEFORMLOW, trace.end()-WAVEFORMHIGH);
	double max_value = *itTrace;
	unsigned int max_x = int(itTrace-trace.begin());

/**** CALCULATE THE AVERAGE AND S_DEV OF THE BASELINE (REPLACE WITH STATS ACCUMULATOR) ****/	    
	double sumOfBaseline = 0, baselineOffset = 0;
        const unsigned int numBinsBaseline = 15;
	
	for (unsigned int v=0; v < numBinsBaseline; v++) 
	{
	    unsigned int baselineValue = trace.at(int(v + baselineOffset));
	    sumOfBaseline += baselineValue;
	}
	
	double aveBaseline = sumOfBaseline / numBinsBaseline;
	double stdDevSum = 0;
	
	for (unsigned int z=0; z < numBinsBaseline; z++) 
	{
	    unsigned int baselineValue = trace.at(z); 
	    stdDevSum  += (baselineValue - aveBaseline) * (baselineValue - aveBaseline);
	}
	double stdDevBaseline = sqrt((1/double(numBinsBaseline))*stdDevSum);
	
/**** GET THE QDC VALUE OF THE WAVEFORM ****/
	double traceQDC=0;
	for(unsigned int j = (max_x - WAVEFORMLOW); (j < (max_x + WAVEFORMHIGH)) && (j < trace.size()); j++)
	    traceQDC += (trace.at(j)-aveBaseline);
	
/**** N-GAMMA DISCRIMINATION ****/
	if(subType == "liquid")
	    ngdiscrim(trace, traceQDC, aveBaseline, max_x);
	
	if((stdDevBaseline <= 3) && ((max_value-aveBaseline) >= 5)) //reject noise condition
	{
	    chan->SetStdDevBaseline(stdDevBaseline);
	    chan->SetAveBaseline(aveBaseline);
#ifdef pulsefit
	    chan->SetPhase(fitroutine(trace, max_x, aveBaseline, stdDevBaseline));
#else
	    chan->SetPhase(spt_analysis(trace, max_x, aveBaseline, traceQDC, counter));
#endif
	    chan->SetTrcQDC(traceQDC);
	    chan->SetMaxValue(max_value-aveBaseline);
	    chan->SetMaxPos(max_x);
	} 
    }  //END LOOP OVER WHOLE EVENT
    
    EndProcess();
    return(true);
}

void ngdiscrim(const Trace &trace, const double &traceQDC, const double &aveBaseline, const unsigned int &maxX)
{
    double discrim = 0, discrim_norm = 0;
    
    for(size_t j = maxX+5; (j < maxX+WAVEFORMHIGH) && (j < trace.size()); j++)  
	discrim += (trace.at(j)-aveBaseline);		    
	       
    discrim_norm = (discrim/traceQDC)*10000+150;
    
    plot(D_DISCRIM,int(discrim_norm),1);
    plot(DD_NGVSE, int(discrim), int(traceQDC));
}

#ifdef pulsefit
double fitroutine(const Trace &trace, const int &maxX, const double &aveBaseline, const double &stdDevBaseline)
{
    vector<double> trace_array_fit;
        
    for(int ll = (maxX-WAVEFORMLOW); ll <= (maxX+WAVEFORMHIGH); ll++)
	trace_array_fit.push_back(trace.at(ll) - aveBaseline);
    
    const int size_fit = trace_array_fit.size();
    
    const gsl_multifit_fdfsolver_type *T;
    gsl_multifit_fdfsolver *s;
    int status;
    unsigned int iter = 0;
    const size_t numParams = 2;
        
    gsl_matrix *covar = gsl_matrix_alloc (numParams, numParams);
    double y[size_fit], sigma[size_fit];
    struct WaveformProcessor::FitData d = {size_fit, y, sigma};
    gsl_multifit_function_fdf f;
    double x_init[numParams] = {2, trace.at(maxX)-aveBaseline};
    gsl_vector_view x = gsl_vector_view_array (x_init, numParams);
    
    f.f = &my_f;
    f.df = &expb_df;
    f.fdf = &expb_fdf;
    f.n = size_fit;
    f.p = numParams;
    f.params = &d;
    
    for(int b = 0; b < size_fit; b++)
    {
	y[b] = trace_array_fit.at(b);
	sigma[b] = stdDevBaseline;
    }
    
    T = gsl_multifit_fdfsolver_lmsder;
    s = gsl_multifit_fdfsolver_alloc (T, size_fit, numParams);
    gsl_multifit_fdfsolver_set (s, &f, &x.vector);
    
    do
    {
	iter++;
	
	status = gsl_multifit_fdfsolver_iterate(s);
	
	if (status)
	    break;
	
	status = gsl_multifit_test_delta (s->dx, s->x,
					  0.001, 0.001);
    }
    while ((status == GSL_CONTINUE) && (iter < 1000000));
    
    gsl_multifit_covar (s->J, 0.0, covar);
    
    double fit_results[2];
    for(int i=0; i < 2; i++)
	fit_results[i] = gsl_vector_get(s->x,i);

    gsl_multifit_fdfsolver_free (s);
    gsl_matrix_free (covar);

    return (fit_results[0]+maxX);
}

int my_f (const gsl_vector * x, void *FitData, gsl_vector * f)
{
    size_t n = ((struct WaveformProcessor::FitData *)FitData)->n;
    double *y = ((struct WaveformProcessor::FitData *)FitData)->y;
    double *sigma = ((struct WaveformProcessor::FitData *)FitData)->sigma;

    double alpha = gsl_vector_get (x,0);
    double beta = gsl_vector_get (x, 1);

    for(size_t i = 0; i < n; i++)
    {
	double t = i;
	double Yi = 0;

	if(t < alpha)
	    Yi = 0;
	else
	    Yi = beta*(1-exp(-((t-alpha)*(t-alpha))/WID))*exp(-(t-alpha)/DKAY);

	gsl_vector_set (f, i, (Yi - y[i])/sigma[i]);
     }
    return GSL_SUCCESS;
}

int expb_df (const gsl_vector * x, void *FitData, gsl_matrix * J)
{
    size_t n = ((struct WaveformProcessor::FitData *)FitData)->n;
    double *sigma = ((struct WaveformProcessor::FitData *) FitData)->sigma;
  
    double alpha = gsl_vector_get (x, 0);
    double beta = gsl_vector_get (x, 1);

    double dalf, dbet;
    
    for (size_t i = 0; i < n; i++)
    {
	/* Jacobian matrix J(i,j) = dfi / dxj, */
	/* where fi = (Yi - yi)/sigma[i],      */
	/*       Yi = A * exp(-beta * i) + b  */
	/* and the xj are the parameters (A,beta,b) */
 	double t = i;
 	double s = sigma[i];
	
	if(t < alpha)
	{
	    dbet = 0;
	    dalf = 0;
	}
	else
	{
	    dbet = (1-exp(-(t-alpha)*(t-alpha)/WID))*exp(-(t-alpha)/DKAY);
	    dalf = beta*exp(-(t-alpha)/DKAY)*((1-exp(-(t-alpha)*(t-alpha)/WID))/DKAY - (2*(t-alpha)/WID)*exp(-(t-alpha)*(t-alpha)/WID));
	}

	gsl_matrix_set (J,i,0, dalf/s);
	gsl_matrix_set (J,i,1, dbet/s);
    }
    return GSL_SUCCESS;
}

int expb_fdf (const gsl_vector * x, void *FitData,
	  gsl_vector * f, gsl_matrix * J)
{
    my_f (x, FitData, f);
    expb_df (x, FitData, J);
    
    return GSL_SUCCESS;
}
#else //pulsefit undefined
double spt_analysis(const Trace &trace, const unsigned int &maxX, const double &ave_baseline, const double &trcQDC, const int &counter)
{
    //! Lots of magic numbers here
    //Normalize the trace
    vector<double> normtrc(trace.size());
    for(unsigned int j = 0; j < trace.size(); j++)
	normtrc.at(j) = ((trace.at(j)-ave_baseline)/trcQDC)*6226.55; 
//    	normtrc.at(j) = ((trace.at(j)-ave_baseline)); 

    // not currently used
    /*
    double bassum=0;
    for (unsigned int v=0; v < 15; v++) 
    {
	//! unsigned int baselineValue = normtrc.at(v);
	// bassum += baselineValue;
	bassum += normtrc.at(v);
    }
    double ave_bass = bassum / 15;
    */

    double normE=0;
    for(unsigned int j = (maxX - WAVEFORMLOW); j < (maxX + WAVEFORMHIGH); j++)
	normE += (normtrc.at(j));
    
    double delta1=fabs(trace.at(maxX)-trace.at(maxX-1));
    double inv_fcn = 0;
    
    //pulser settings
//    const double sigma1 = 0.102; //solid parameters
//    const double amp1 = 1904.293314;

    const double sigma1 = 0.166597; //test parameters
    const double amp1 = 1816.27;

    //short bar settings
//    const double sigma1 = 0.039185;
//    const double amp1 = 1997.157;

    //dev settings
    //const double sigma1 = 0.102; //0.097481  0.096199429
    //const double amp1 = 0.3192159*normE-1.361152;     //1986.2
    //const double amp1 = 0.3192159*trcQDC-1.361152;     //1986.2
    
     if(delta1 > ((trace.at(maxX)-ave_baseline)*0.4))  
 	inv_fcn = (pow(-log((normtrc.at(maxX-1))/(amp1))*pow(sigma1,3),0.25)/sigma1)+(maxX-1);
     else if ((delta1 < (trace.at(maxX)-ave_baseline)*0.4) &&  (normtrc.at(maxX-2) > 0))
 	inv_fcn = (pow(-log((normtrc.at(maxX-2))/(amp1))*pow(sigma1,3),0.25)/sigma1)+(maxX-2);
     else
 	inv_fcn = (pow(-log((normtrc.at(maxX-1))/(amp1))*pow(sigma1,3),0.25)/sigma1)+(maxX-1);

    return inv_fcn;
}
#endif //#ifdef pulsefit
