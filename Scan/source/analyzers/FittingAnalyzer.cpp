/** \file FittingAnalyzer.cpp
 * \brief Uses a chi^2 minimization to fit waveforms
 *
 * Obtains the phase of a waveform using a Chi^2 fitting algorithm
 * implemented through the GSL libraries. We have now set up two different
 * functions for this processor. One of them handles the fast SiPMT signals,
 * which tend to be more Gaussian in shape than the standard PMT signals.
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
#include <gsl/gsl_vector.h>

/** Defines the GSL fitting function for standard PMTs
 * \param [in] x : the vector of gsl starting parameters
 * \param [in] FitData : The data to use for the fit
 * \param [in] f : pointer to the function
 * \return an integer that GSL does something magical with */
int PmtFunction(const gsl_vector *x, void *FitData, gsl_vector *f);
/** Defines the GSL fitting function for standard PMTs
 * \param [in] x : the vector of gsl starting parameters
 * \param [in] FitData : The data to use for the fit
 * \param [in] J : pointer to the Jacobian of the function
 * \return an integer that GSL does something magical with */
int CalcPmtJacobian(const gsl_vector *x, void *FitData, gsl_matrix *J);
/** Defines the GSL fitting function for the fast output of SiPMTs
 * \param [in] x : the vector of gsl starting parameters
 * \param [in] FitData : The data to use for the fit
 * \param [in] f : pointer to the function
 * \return an integer that GSL does something magical with */
int SiPmtFunction(const gsl_vector *x, void *FitData, gsl_vector *f);
/** Defines the GSL fitting function for the fast output SiPMTs
 * \param [in] x : the vector of gsl starting parameters
 * \param [in] FitData : The data to use for the fit
 * \param [in] J : pointer to the Jacobian of the function
 * \return an integer that GSL does something magical with */
int CalcSiPmtJacobian(const gsl_vector *x, void *FitData, gsl_matrix *J);

using namespace std;
using namespace dammIds::trace::waveformanalyzer;

void FittingAnalyzer::DeclarePlots(void) {
    Trace sample_trace = Trace();
    sample_trace.DeclareHistogram2D(DD_TRACES, S7, S5, "traces data FitAnalyzer");
    sample_trace.DeclareHistogram2D(DD_AMP, SE, SC, "Fit Amplitude");
    sample_trace.DeclareHistogram1D(D_PHASE, SE, "Fit X0");
    sample_trace.DeclareHistogram1D(D_CHISQPERDOF, SE, "Chi^2/dof");
    sample_trace.DeclareHistogram1D(D_SIGMA, SE, "Std Dev Baseline");
}

FittingAnalyzer::FittingAnalyzer() {
    name = "FittingAnalyzer";
}

void FittingAnalyzer::Analyze(Trace &trace, const std::string &detType,
                              const std::string &detSubtype,
                              const std::map<std::string, int> & tagMap) {
    TraceAnalyzer::Analyze(trace, detType, detSubtype, tagMap);

    if(trace.HasValue("saturation") || trace.empty() ||
       trace.GetWaveform().size() == 0) {
     	EndAnalyze();
     	return;
    }

    Globals *globals = Globals::get();

    const double sigmaBaseline = trace.GetValue("sigmaBaseline");
    const double maxVal = trace.GetValue("maxval");
    const double qdc = trace.GetValue("qdc");
    const double maxPos = (double)trace.GetValue("maxpos");
    const vector<double> waveform = trace.GetWaveform();
    bool isDblBeta = detType == "beta" && detSubtype == "double";
    bool isDblBetaT = isDblBeta && tagMap.find("timing") != tagMap.end();

    trace.plot(D_SIGMA, sigmaBaseline*100);

    if(!isDblBetaT) {
	if(sigmaBaseline > globals->sigmaBaselineThresh()) {
	    EndAnalyze();
	    return;
	}
    } else {
	if(sigmaBaseline > globals->siPmtSigmaBaselineThresh()) {
	    EndAnalyze();
	    return;
	}
    }

    pair<double,double> pars =  globals->fitPars(detType+":"+detSubtype);
    if(isDblBetaT)
	pars = globals->fitPars(detType+":"+detSubtype+":timing");

    gsl_multifit_function_fdf f;
    int status, info;
    double phase, fitAmp;
    const size_t n = waveform.size();
    size_t p;
    double xInit[3];

    if(!isDblBetaT) {
        p = 2;
        xInit[0] = 0.0;
        xInit[1]=2.5;

        f.f = &PmtFunction;
        f.df = &CalcPmtJacobian;
    } else {
        p = 1;
        xInit[0] = (double)waveform.size()*0.5;

        f.f = &SiPmtFunction;
        f.df = &CalcSiPmtJacobian;
    }

    const gsl_multifit_fdfsolver_type *T = gsl_multifit_fdfsolver_lmsder;
    gsl_multifit_fdfsolver *s;
    gsl_matrix *jac=gsl_matrix_alloc(n,p);
    gsl_matrix *covar = gsl_matrix_alloc (p,p);
    double y[n], weights[n];
    struct FittingAnalyzer::FitData data =
        {n, y, pars.first, pars.second, qdc};
    gsl_vector_view x = gsl_vector_view_array (xInit,p);
    gsl_vector_view w = gsl_vector_view_array (weights,n);

    double xtol = 1e-8;
    double gtol = 1e-8;
    double ftol = 0.0;

    f.n = n;
    f.p = p;
    f.params = &data;

    for(unsigned int i = 0; i < n; i++) {
        weights[i] = sigmaBaseline;
        y[i] = waveform[i];
    }
    
    s = gsl_multifit_fdfsolver_alloc (T, n, p);
    gsl_multifit_fdfsolver_wset (s, &f, &x.vector, &w.vector);

    //Calculate the intial residual norm
    //gsl_vector *res_f = gsl_multifit_fdfsolver_residual(s);
    //double chi0 = gsl_blas_dnrm2(res_f);
    
    status = gsl_multifit_fdfsolver_driver(s, 1000, xtol, gtol, ftol, &info);
    gsl_multifit_fdfsolver_jac(s,jac);
    gsl_multifit_covar (jac, 0.0, covar);

    if(!isDblBetaT) {
        phase = gsl_vector_get(s->x,0);
        fitAmp = gsl_vector_get(s->x,1);
    } else {
        phase = gsl_vector_get(s->x,0);
        fitAmp = 0.0;
    }

    trace.InsertValue("phase", phase+maxPos);
    
    trace.plot(DD_AMP, fitAmp, maxVal);
    trace.plot(D_PHASE, phase*1000+100);
    trace.plot(D_CHISQPERDOF,
               pow(gsl_blas_dnrm2(s->f),2.0)/(n - p));

    gsl_multifit_fdfsolver_free (s);
    gsl_matrix_free (covar);
    gsl_matrix_free(jac);
    EndAnalyze();
}

int PmtFunction (const gsl_vector * x, void *FitData, gsl_vector * f) {
    size_t n       = ((struct FittingAnalyzer::FitData *)FitData)->n;
    double *y      = ((struct FittingAnalyzer::FitData *)FitData)->y;
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

        gsl_vector_set (f, i, Yi - y[i]);
    }
    return(GSL_SUCCESS);
}

int CalcPmtJacobian (const gsl_vector * x, void *FitData, gsl_matrix * J) {
    size_t n = ((struct FittingAnalyzer::FitData *)FitData)->n;
    double beta    = ((struct FittingAnalyzer::FitData *)FitData)->beta;
    double gamma   = ((struct FittingAnalyzer::FitData *)FitData)->gamma;
    double qdc    = ((struct FittingAnalyzer::FitData *)FitData)->qdc;

    double phi     = gsl_vector_get (x, 0);
    double alpha   = gsl_vector_get (x, 1);

    double dphi, dalpha;

    for(size_t i = 0; i < n; i++) {
        double t = i;
        double diff = t-phi;
        double gaussSq = exp(-pow(gamma*diff,4.));
        if(t < phi) {
            dphi   = 0;
            dalpha = 0;
        } else {
            dphi = alpha*beta*qdc*exp(-beta*diff)*(1-gaussSq) -
                    4*alpha*qdc*pow(diff,3.)*exp(-beta*diff)*pow(gamma,4.)*gaussSq;
            dalpha = qdc * exp(-beta*diff) * (1-gaussSq);
        }
        gsl_matrix_set(J,i,0, dphi);
        gsl_matrix_set(J,i,1, dalpha);
    }
    return(GSL_SUCCESS);
}

int SiPmtFunction (const gsl_vector * x, void *FitData, gsl_vector * f) {
    size_t n       = ((struct FittingAnalyzer::FitData *)FitData)->n;
    double *y      = ((struct FittingAnalyzer::FitData *)FitData)->y;
    double gamma   = ((struct FittingAnalyzer::FitData *)FitData)->gamma;
    double qdc     = ((struct FittingAnalyzer::FitData *)FitData)->qdc;

    double phi = gsl_vector_get (x, 0);

    for(size_t i = 0; i < n; i++) {
        double t = i;
        double diff = t-phi;
        double Yi = (qdc/(gamma*sqrt(2*M_PI)))*exp(-diff*diff/(2*gamma*gamma));
        gsl_vector_set (f, i, Yi - y[i]);
    }
    return(GSL_SUCCESS);
}

int CalcSiPmtJacobian (const gsl_vector * x, void *FitData, gsl_matrix * J) {
    size_t n       = ((struct FittingAnalyzer::FitData *)FitData)->n;
    double gamma   = ((struct FittingAnalyzer::FitData *)FitData)->gamma;
    double qdc     = ((struct FittingAnalyzer::FitData *)FitData)->qdc;

    double phi = gsl_vector_get (x, 0);
    double dphi;

    for (size_t i = 0; i < n; i++) {
        double t = i;
        double diff = t-phi;

        dphi = (qdc*diff/(pow(gamma,3)*sqrt(2*M_PI))) *
            exp(-diff*diff/(2*gamma*gamma));

        gsl_matrix_set (J,i,0, dphi);
    }
    return(GSL_SUCCESS);
}
