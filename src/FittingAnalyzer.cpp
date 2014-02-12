/*********************************************
 This code will obtain the phase of a trace
using a Chi^2 fitting routine in GSL, 

S.V. Paulauskas 22 July 2011

********************************************/
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <vector>

#include <time.h>

#include "DammPlotIds.hpp"
#include "FittingAnalyzer.hpp"

#include <gsl/gsl_blas.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_vector.h>

int FitFunction(const gsl_vector *x, void *FitData, 
		gsl_vector *f);
int CalculateJacobian(const gsl_vector *x, void *FitData, 
		      gsl_matrix *J);
int FitFunctionDerivative(const gsl_vector *x, void *FitData, 
			  gsl_vector *f, gsl_matrix *J);

using namespace std;
using namespace dammIds::trace::waveformanalyzer;

//********** DeclarePlots **********
void FittingAnalyzer::DeclarePlots(void)
{
    Trace sample_trace = Trace(); 
    sample_trace.DeclareHistogram2D(DD_TRACES, S7, S5, "traces data FitAnalyzer");
    sample_trace.DeclareHistogram2D(DD_AMP, SE, SC, "Fit Amplitude");
    sample_trace.DeclareHistogram1D(D_PHASE, SE, "Fit X0");
    sample_trace.DeclareHistogram1D(D_CHISQPERDOF, SE, "Chi^2/dof");
}


//********** FittingAnalyzer **********
FittingAnalyzer::FittingAnalyzer()
{
    name = "FittingAnalyzer";
    counter = 0;
}


//********** Analyze **********
void FittingAnalyzer::Analyze(Trace &trace, const string &detType, 
			      const string &detSubtype)
{
    TraceAnalyzer::Analyze(trace, detType, detSubtype);
    if(trace.HasValue("saturation")) {
       EndAnalyze();
       return;
    }

    const double aveBaseline = trace.GetValue("baseline");
    const double sigmaBaseline = trace.GetValue("sigmaBaseline");
    const double maxVal = trace.GetValue("maxval");

    const unsigned int maxPos = (unsigned int)trace.GetValue("maxpos");
    const unsigned int waveformLow = 
	(unsigned int)TimingInformation::GetConstant("waveformLow");
    const unsigned int waveformHigh = 
	(unsigned int)TimingInformation::GetConstant("waveformHigh");

    if((maxPos < waveformLow) || (maxPos + waveformHigh >= trace.size()) ||
       (sigmaBaseline > 3)) {
       EndAnalyze();
       return;
    }

    for(unsigned int i = (maxPos-waveformLow); 
	i <= (maxPos+waveformHigh); i++) {
	fittedTrace.push_back(trace.at(i) - aveBaseline);
    }
    
    const size_t sizeFit = fittedTrace.size();
    
    //Set the gaussian width (width) and decay constant (decay) 
    //for the Fitting Routine.
    double width, decay;
    if(detType == "vandleSmall") {
	width = TimingInformation::GetConstant("widthVandle");
	decay = TimingInformation::GetConstant("decayVandle");
    }else if (detSubtype == "liquid") {
	width = TimingInformation::GetConstant("widthLiquid");
	decay = TimingInformation::GetConstant("decayLiquid");
    } else {
	width = TimingInformation::GetConstant("widthDefault");
	decay = TimingInformation::GetConstant("decayDefault");
    }
    
    const gsl_multifit_fdfsolver_type *T;
    gsl_multifit_fdfsolver *s;
    int status;
    const size_t numParams = 2;
        
    gsl_matrix *covar = gsl_matrix_alloc (numParams, numParams);
    double y[sizeFit], sigma[sizeFit];
    struct FittingAnalyzer::FitData data = 
	{sizeFit, y, sigma, width, decay};
    gsl_multifit_function_fdf f;
    double xInit[numParams];
    if (Globals::get()->revision() == "D" ||
        Globals::get()->revision() == "F") {
        xInit[0] = 0.0;
        xInit[1] = maxVal * 10.0;
    } else {
        xInit[0] = 0.0;
        xInit[1] = maxVal * 3.0;
    }
    gsl_vector_view x = 
	gsl_vector_view_array (xInit, numParams);
    
    f.f = &FitFunction;
    f.df = &CalculateJacobian;
    f.fdf = &FitFunctionDerivative;
    f.n = sizeFit;
    f.p = numParams;
    f.params = &data;
    
    for(unsigned int b = 0; b < sizeFit; b++) {
	y[b] = fittedTrace.at(b);
	sigma[b] = sigmaBaseline;
    }
    
    T = gsl_multifit_fdfsolver_lmsder;
    s = gsl_multifit_fdfsolver_alloc (T, sizeFit, numParams);
    gsl_multifit_fdfsolver_set (s, &f, &x.vector);
    
    for(unsigned int iter = 0; iter < 1000000; iter++) {
	status = gsl_multifit_fdfsolver_iterate(s);
	
	if(status)
	    break;
	
	status = gsl_multifit_test_delta (s->dx, s->x,
					  0.001, 0.001);
	
	if(status != GSL_CONTINUE)
	    break;
    }

    gsl_multifit_covar (s->J, 0.0, covar);
    
    double chi = gsl_blas_dnrm2(s->f);
    double dof = sizeFit - numParams;
    double chisqPerDof = pow(chi, 2.0)/dof;

    for(unsigned int i=0; i < numParams; i++)
	fittedParameters.push_back(gsl_vector_get(s->x,i));
    fittedParameters.push_back(width);
    fittedParameters.push_back(decay);

    gsl_multifit_fdfsolver_free (s);
    gsl_matrix_free (covar);

    trace.InsertValue("phase", fittedParameters.front()+maxPos);
    trace.InsertValue("walk", CalculateWalk(maxVal));

    trace.plot(DD_AMP, fittedParameters.at(1), trace.at(maxPos)-aveBaseline);
    trace.plot(D_PHASE, fittedParameters.at(0)*1000+1000);    
    trace.plot(D_CHISQPERDOF, chisqPerDof);
    
    if(detType == "vandleSmall"){
        unsigned sz = trace.size();
        for(double i = 0; i < sz; ++i) {
            trace.plot(DD_TRACES, i, counter, trace.at(int(i))-aveBaseline);
            trace.plot(DD_TRACES, i, counter+1,
                    CalculateFittedFunction(i) + 100);
        }
        counter++;
    }
    
    FreeMemory();
    EndAnalyze();
} //void FittingAnalyzer::Analyze


//********** CalculateReducedChiSquared **********
double FittingAnalyzer::CalculateReducedChiSquared(const double &dof, 
						   const double &sigmaBaseline)
{
    double chisq = 0;
    for(double  i = 0; i < fittedTrace.size(); i++) {
	chisq += 
	    pow((fittedTrace.at(int(i))-
		 CalculateFittedFunction(i))/sigmaBaseline,2.0);
    } 
    return(chisq/dof);
}


//********** CalculateFittedFunction **********
double FittingAnalyzer::CalculateFittedFunction(double &x)
{
    double alpha = fittedParameters.at(0);
    double beta  = fittedParameters.at(1);
    double width = fittedParameters.at(2);
    double decay = fittedParameters.at(3);

    double function = beta*((1-exp(-((x-alpha)*(x-alpha))/width))*
			    exp(-(x-alpha)/decay));
    
    return(function);
}


//********** WalkCorrection **********
double FittingAnalyzer::CalculateWalk(const double &maxVal)
{
    double f = 92.7907602830327 * exp(-maxVal/186091.225414275) +
	0.59140785215161 * exp(maxVal/2068.14618331387) - 
	95.5388835298589;
    return (f); //calculated in ns
}


//********** Free Memory **********
void FittingAnalyzer::FreeMemory(void)
{
    fittedParameters.clear();
    fittedTrace.clear();
}


//********** OutputFittedFunction **********
void FittingAnalyzer::OutputFittedInformation()
{
     for(vector<double>::iterator it = fittedParameters.begin();
 	it != fittedParameters.end(); it++)
	 cout << *it << endl;

    cout << endl;

    for(vector<double>::const_iterator it = fittedTrace.begin();
 	it != fittedTrace.end(); it++) 
 	cout << int(it-fittedTrace.begin()) << " " << *it << endl;
}


//*********** FitFunction **********
int FitFunction (const gsl_vector * x, void *FitData, 
				  gsl_vector * f)
{
    size_t n = ((struct FittingAnalyzer::FitData *)FitData)->n;
    double *y = ((struct FittingAnalyzer::FitData *)FitData)->y;
    double *sigma = ((struct FittingAnalyzer::FitData *)FitData)->sigma;
    double width = ((struct FittingAnalyzer::FitData *)FitData)->width;
    double decay = ((struct FittingAnalyzer::FitData *)FitData)->decay;

    double alpha = gsl_vector_get (x, 0);
    double beta  = gsl_vector_get (x, 1);

    for(size_t i = 0; i < n; i++) {
	double t = i;
	double Yi = 0;

	if(t < alpha)
	    Yi = 0;
	else
	    Yi = beta*((1-exp(-((t-alpha)*(t-alpha))/width))*
		       exp(-(t-alpha)/decay));

	gsl_vector_set (f, i, (Yi - y[i])/sigma[i]);
     }
    return GSL_SUCCESS;
}


//********** CalculateJacobian **********
int CalculateJacobian (const gsl_vector * x, void *FitData, gsl_matrix * J)
{
    size_t n = ((struct FittingAnalyzer::FitData *)FitData)->n;
    double *sigma = ((struct FittingAnalyzer::FitData *) FitData)->sigma;
    double width = ((struct FittingAnalyzer::FitData *)FitData)->width;
    double decay = ((struct FittingAnalyzer::FitData *)FitData)->decay;
  
    double alpha = gsl_vector_get (x, 0);
    double beta = gsl_vector_get (x, 1);

    double dalf, dbet;
    
    for (size_t i = 0; i < n; i++) {
	//Compute the Jacobian 
 	double t = i;
 	double s = sigma[i];
	
	if(t < alpha) {
	    dbet = 0;
	    dalf = 0;
	}
	else {
	    dbet = ((1-exp(-(t-alpha)*(t-alpha)/width))*
		    exp(-(t-alpha)/decay));
	    dalf = beta*exp(-(t-alpha)/decay)*
		((1-exp(-(t-alpha)*(t-alpha)/width))/decay - 
		 (2*(t-alpha)/width)*
		 exp(-(t-alpha)*(t-alpha)/width));
	}

	gsl_matrix_set (J,i,0, dalf/s);
	gsl_matrix_set (J,i,1, dbet/s);
    }
    return GSL_SUCCESS;
}


//********** FitFunctionDerivative **********
int FitFunctionDerivative (const gsl_vector * x, void *FitData,
	  gsl_vector * f, gsl_matrix * J)
{
    FitFunction (x, FitData, f);
    CalculateJacobian (x, FitData, J);
    
    return GSL_SUCCESS;
}
