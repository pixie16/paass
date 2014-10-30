/** \file FittingAnalyzer.cpp
 * \brief Uses a chi^2 minimization to fit waveforms
 *
 * Obtains the phase of a waveform using a Chi^2 fitting algorithm
 * implemented through the GSL libraries.
 *
 * \author S. V. Paulauskas
 * \date 22 July 2011
 */
#include <algorithm>
#include <iostream>
#include <vector>

#include <ctime>

#include "Globals.hpp"
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
int CalcJacobian(const gsl_vector *x, void *FitData,
		      gsl_matrix *J);
int FitFunctionDerivative(const gsl_vector *x, void *FitData,
			  gsl_vector *f, gsl_matrix *J);

using namespace std;
using namespace dammIds::trace::waveformanalyzer;

//********** DeclarePlots **********
void FittingAnalyzer::DeclarePlots(void) {
    Trace sample_trace = Trace();
    sample_trace.DeclareHistogram2D(DD_TRACES, S7, S5, "traces data FitAnalyzer");
    sample_trace.DeclareHistogram2D(DD_AMP, SE, SC, "Fit Amplitude");
    sample_trace.DeclareHistogram1D(D_PHASE, SE, "Fit X0");
    sample_trace.DeclareHistogram1D(D_CHISQPERDOF, SE, "Chi^2/dof");
    sample_trace.DeclareHistogram1D(D_SIGMA, SE, "Std Dev Baseline");
}


//********** FittingAnalyzer **********
FittingAnalyzer::FittingAnalyzer() {
    name = "FittingAnalyzer";
}


//********** Analyze **********
void FittingAnalyzer::Analyze(Trace &trace, const std::string &detType,
			      const std::string &detSubtype) {
    TraceAnalyzer::Analyze(trace, detType, detSubtype);

    Globals *globals = Globals::get();

    if(trace.HasValue("saturation") || trace.empty()) {
     	EndAnalyze();
     	return;
    }

    const double sigmaBaseline = trace.GetValue("sigmaBaseline");
    const double maxVal = trace.GetValue("maxval");
    const double qdc = trace.GetValue("tqdc");
    const unsigned int maxPos = (unsigned int)trace.GetValue("maxpos");
    const vector<double> waveform = trace.GetWaveform();

    if(waveform.size() == 0) {
        EndAnalyze();
        return;
    }

    trace.plot(D_SIGMA, sigmaBaseline*100);

    if(sigmaBaseline > globals->sigmaBaselineThresh()) {
        EndAnalyze();
        return;
    }

    pair<double,double> pars;
    if (detType == "vandleSmall")
        pars = globals->vandlePars();
    else if (detSubtype == "beta")
        pars = globals->startPars();
    else if(detType == "tvandle")
        pars = globals->tvandlePars();
    else if(detType == "pulser")
        pars = globals->pulserPars();
    else if (detType == "sipmt")
        pars = globals->siPmtPars();
    else
        pars = globals->vandlePars();

    const gsl_multifit_fdfsolver_type *T;
    gsl_multifit_fdfsolver *s;
    int status;
    const size_t numParams = 2;
    const size_t sizeFit = waveform.size();

    gsl_matrix *covar = gsl_matrix_alloc (numParams, numParams);
    double y[sizeFit], sigma[sizeFit];
    struct FittingAnalyzer::FitData data =
        {sizeFit, y, sigma, pars.first, pars.second, qdc};
    gsl_multifit_function_fdf f;

    double xInit[numParams] = {0.0,2.5};
    gsl_vector_view x = gsl_vector_view_array (xInit, numParams);

    f.f = &FitFunction;
    f.df = &CalcJacobian;
    f.fdf = &FitFunctionDerivative;
    f.n = sizeFit;
    f.p = numParams;
    f.params = &data;

    for(unsigned int i = 0; i < sizeFit; i++) {
        y[i] = waveform.at(i);
        sigma[i] = sigmaBaseline;
    }

    T = gsl_multifit_fdfsolver_lmsder;
    s = gsl_multifit_fdfsolver_alloc (T, sizeFit, numParams);
    gsl_multifit_fdfsolver_set (s, &f, &x.vector);

    for(unsigned int iter = 0; iter < 1e8; iter++) {
    	status = gsl_multifit_fdfsolver_iterate(s);
        if(status)
    	    break;
        status = gsl_multifit_test_delta (s->dx, s->x, 1e-4, 1e-4);
        if(status != GSL_CONTINUE)
    	    break;
    }

    gsl_multifit_covar (s->J, 0.0, covar);

    double chi = gsl_blas_dnrm2(s->f);
    double dof = sizeFit - numParams;
    double chisqPerDof = pow(chi, 2.0)/dof;
    vector<double> fitPars;

    for(unsigned int i=0; i < numParams; i++)
	fitPars.push_back(gsl_vector_get(s->x,i));
    fitPars.push_back(pars.first);
    fitPars.push_back(pars.second);

    gsl_multifit_fdfsolver_free (s);
    gsl_matrix_free (covar);

    trace.InsertValue("phase", fitPars.front()+maxPos);
    //trace.InsertValue("walk", CalcWalk(maxVal, detType, detSubtype));

    trace.plot(DD_AMP, fitPars.at(1), maxVal);
    trace.plot(D_PHASE, fitPars.at(0)*1000+100);
    trace.plot(D_CHISQPERDOF, chisqPerDof);

    EndAnalyze();
}

//*********** FitFunction **********
int FitFunction (const gsl_vector * x, void *FitData, gsl_vector * f) {
    size_t n       = ((struct FittingAnalyzer::FitData *)FitData)->n;
    double *y      = ((struct FittingAnalyzer::FitData *)FitData)->y;
    double *sigma  = ((struct FittingAnalyzer::FitData *)FitData)->sigma;
    double beta    = ((struct FittingAnalyzer::FitData *)FitData)->beta;
    double gamma   = ((struct FittingAnalyzer::FitData *)FitData)->gamma;
    double qdc     = ((struct FittingAnalyzer::FitData *)FitData)->qdc;

    double phi     = gsl_vector_get (x, 0);
    double alpha   = gsl_vector_get (x, 1);

    for(size_t i = 0; i < n; i++) {
	double t = i;
	double diff = t-phi;
	double Yi = 0;

	if(t < phi)
	    Yi = 0;
	else
	    Yi = qdc * alpha * exp(-beta*diff) * (1-exp(-pow(gamma*diff,4.)));

	gsl_vector_set (f, i, (Yi - y[i])/sigma[i]);
     }
    return(GSL_SUCCESS);
}


//********** CalcJacobian **********
int CalcJacobian (const gsl_vector * x, void *FitData, gsl_matrix * J) {
    size_t n = ((struct FittingAnalyzer::FitData *)FitData)->n;
    double *sigma = ((struct FittingAnalyzer::FitData *) FitData)->sigma;
    double beta    = ((struct FittingAnalyzer::FitData *)FitData)->beta;
    double gamma   = ((struct FittingAnalyzer::FitData *)FitData)->gamma;
    double qdc    = ((struct FittingAnalyzer::FitData *)FitData)->qdc;

    double phi     = gsl_vector_get (x, 0);
    double alpha   = gsl_vector_get (x, 1);

    double dphi, dalpha;

    for (size_t i = 0; i < n; i++) {
	//Compute the Jacobian
 	double t = i;
	double diff = t-phi;
	double gaussSq = exp(-pow(gamma*diff,4.));
 	double s = sigma[i];

	if(t < phi) {
	    dphi   = 0;
	    dalpha = 0;
	}
	else {
	    dphi = alpha*beta*qdc*exp(-beta*diff)*(1-gaussSq) -
		4*alpha*qdc*pow(diff,3.)*exp(-beta*diff)*pow(gamma,4.)*gaussSq;
	    dalpha = qdc * exp(-beta*diff) * (1-gaussSq);
	}

	gsl_matrix_set (J,i,0, dphi/s);
	gsl_matrix_set (J,i,1, dalpha/s);
    }
    return(GSL_SUCCESS);
}


//********** FitFunctionDerivative **********
int FitFunctionDerivative (const gsl_vector * x, void *FitData, gsl_vector * f,
                            gsl_matrix * J) {
    FitFunction (x, FitData, f);
    CalcJacobian (x, FitData, J);
    return(GSL_SUCCESS);
}
