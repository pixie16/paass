/************************************
This code will obtain the phase of a trace
using either a Chi^2 fitting routine in GSL, 
or a Single Point Analysis. 

S.V.P. 16 July 2009

*************************************/
#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>

#include <cstdlib>
#include <cmath>

#include <unistd.h> 

#include "DammPlotIds.hpp"
#include "StatsAccumulator.hpp"
#include "WaveformAnalyzer.hpp"

#ifdef pulsefit
#include <gsl/gsl_blas.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_vector.h>
#endif

using namespace dammIds::waveformanalyzer;
using namespace std;

//A FEW MAGIC NUMBERS
#define WAVEFORMLOW  2  //The starting position of the Waveform (referenced from max)
#define WAVEFORMHIGH 12 //The point when the Waveform returns to baseline (referenced from max)

#ifdef pulsefit
double FitRoutine(const Trace &trace, const string &detSubtype);
int FitFunction(const gsl_vector *x, void *FitData, gsl_vector *f);
int FitFunctionDerivative(const gsl_vector *x, void *FitData, gsl_matrix *J);
int FitFunctionDf(const gsl_vector *x, void *FitData, gsl_vector *f, gsl_matrix *J);
#else
double SptAnalysis(Trace &trace, const string &detType, const string &detSubtype);
#endif



WaveformAnalyzer::WaveformAnalyzer() : TraceAnalyzer()
{
    name = "Waveform";
}

void WaveformAnalyzer::DeclarePlots(void)
{
}

void WaveformAnalyzer::Analyze(Trace &trace,
			       const string &detType, const string &detSubtype)
{
    TraceAnalyzer::Analyze(trace, detType, detSubtype);
    
/*   ! Does nothing for now wait for new functionality
     int satTraceCount = count(trace.begin(),trace.end(),4095);
     
     if (satTraceCount > 0) //SKIP IF SATURATION
     return;
*/
    unsigned int maxPos = trace.FindMaxInfo();
    unsigned int maxVal = (unsigned int)trace.GetValue("maxval");
    double aveBaseline = trace.DoBaseline();  
    double sigmaBaseline = trace.GetValue("sigmaBaseline");

/**** Calculate the QDC value of the waveform ****/
    double traceQDC=0;
    for(unsigned int j = (maxPos - WAVEFORMLOW); (j < (maxPos + WAVEFORMHIGH)) && (j < trace.size()); j++)
	traceQDC += (trace.at(j)-aveBaseline);
    trace.InsertValue("tqdc", traceQDC);
    
    // Reject noise, and set the phase for the waveform
    if((sigmaBaseline <= 3) && ((maxVal-aveBaseline) >= 5)) { 
#ifdef pulsefit
	trace.InsertValue("phase", FitRoutine(trace, detSubtype));
#else
	trace.InsertValue("phase", SptAnalysis(trace, detType, detSubtype));
#endif
    } 
    
    EndAnalyze();
}

#ifdef pulsefit
double FitRoutine(const Trace &trace, const string &detSubtype)
{
    vector<double> trace_array_fit;
    double WID, DKAY;
    
    double aveBaseline = trace.GetValue("baseline");
    int maxX = (int)trace.GetValue("maxpos");

    for(int ll = (maxX-WAVEFORMLOW); ll <= (maxX+WAVEFORMHIGH); ll++)
	trace_array_fit.push_back(trace.at(ll) - aveBaseline);
    
    const int size_fit = trace_array_fit.size();
    
    // Set the gaussian width (WID) and decay constant (DKAY) for the Fitting Routine
    if (detSubtype == "liquid") {
	WID = 4.52672;
	DKAY = 1.73376;
    }
    else {
	WID = 3.9626;
	DKAY = 3.2334;
    }
    
    const gsl_multifit_fdfsolver_type *T;
    gsl_multifit_fdfsolver *s;
    int status;
    unsigned int iter = 0;
    const size_t numParams = 2;
        
    gsl_matrix *covar = gsl_matrix_alloc (numParams, numParams);
    double y[size_fit], sigma[size_fit];
    struct WaveformAnalyzer::FitData d = {size_fit, y, sigma, WID, DKAY};
    gsl_multifit_function_fdf f;
    double x_init[numParams] = {WAVEFORMLOW, trace.at(maxX)-aveBaseline};
    gsl_vector_view x = gsl_vector_view_array (x_init, numParams);
    
    f.f = &FitFunction;
    f.df = &FitFunctionDerivative;
    f.fdf = &FitFunctionDf;
    f.n = size_fit;
    f.p = numParams;
    f.params = &d;
    
    for(int b = 0; b < size_fit; b++) {
	y[b] = trace_array_fit.at(b);
	sigma[b] = trace.GetValue("sigmaBaseline");
    }
    
    T = gsl_multifit_fdfsolver_lmsder;
    s = gsl_multifit_fdfsolver_alloc (T, size_fit, numParams);
    gsl_multifit_fdfsolver_set (s, &f, &x.vector);
    
    do {
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

int FitFunction (const gsl_vector * x, void *FitData, gsl_vector * f)
{
    size_t n = ((struct WaveformAnalyzer::FitData *)FitData)->n;
    double *y = ((struct WaveformAnalyzer::FitData *)FitData)->y;
    double *sigma = ((struct WaveformAnalyzer::FitData *)FitData)->sigma;
    double WID = ((struct WaveformAnalyzer::FitData *)FitData)->WID;
    double DKAY = ((struct WaveformAnalyzer::FitData *)FitData)->DKAY;

    double alpha = gsl_vector_get (x,0);
    double beta = gsl_vector_get (x, 1);

    for(size_t i = 0; i < n; i++) {
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

int FitFunctionDerivative (const gsl_vector * x, void *FitData, gsl_matrix * J)
{
    size_t n = ((struct WaveformAnalyzer::FitData *)FitData)->n;
    double *sigma = ((struct WaveformAnalyzer::FitData *) FitData)->sigma;
    double WID = ((struct WaveformAnalyzer::FitData *)FitData)->WID;
    double DKAY = ((struct WaveformAnalyzer::FitData *)FitData)->DKAY;
  
    double alpha = gsl_vector_get (x, 0);
    double beta = gsl_vector_get (x, 1);

    double dalf, dbet;
    
    for (size_t i = 0; i < n; i++) {
	/* Jacobian matrix J(i,j) = dfi / dxj, */
	/* where fi = (Yi - yi)/sigma[i],      */
	/*       Yi = A * exp(-beta * i) + b  */
	/* and the xj are the parameters (A,beta,b) */
 	double t = i;
 	double s = sigma[i];
	
	if(t < alpha) {
	    dbet = 0;
	    dalf = 0;
	}
	else {
	    dbet = (1-exp(-(t-alpha)*(t-alpha)/WID))*exp(-(t-alpha)/DKAY);
	    dalf = beta*exp(-(t-alpha)/DKAY)*((1-exp(-(t-alpha)*(t-alpha)/WID))/DKAY - (2*(t-alpha)/WID)*exp(-(t-alpha)*(t-alpha)/WID));
	}

	gsl_matrix_set (J,i,0, dalf/s);
	gsl_matrix_set (J,i,1, dbet/s);
    }
    return GSL_SUCCESS;
}

int FitFunctionDf (const gsl_vector * x, void *FitData,
	  gsl_vector * f, gsl_matrix * J)
{
    FitFunction (x, FitData, f);
    FitFunctionDerivative (x, FitData, J);
    
    return GSL_SUCCESS;
}
#else //pulsefit undefined
double SptAnalysis(Trace &trace, const string &detType, const string &detSubtype)
{
    // this has been removed for the present
    return 0.;
}
#endif //#ifdef pulsefit
