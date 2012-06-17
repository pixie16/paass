/*********************************************
 This code will obtain the phase of a trace
using a CFD coupled with a polynomial fit to
a spline. 

S.V. Paulauskas 22 July 2011

********************************************/
#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "CfdAnalyzer.hpp"

#include <gsl/gsl_blas.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_vector.h>

int FitFunction(const gsl_vector *x, void *FitData, 
		gsl_vector *f);
int FitFunctionDerivative(const gsl_vector *x, void *FitData, 
			  gsl_matrix *J);
int FitFunctionDf(const gsl_vector *x, void *FitData, 
		  gsl_vector *f, gsl_matrix *J);

using namespace std;

//********** CfdAnalyzer **********
CfdAnalyzer::CfdAnalyzer() : TraceAnalyzer()
{
    name = "CfdAnalyzer";
}


//********** Analyze **********
void CfdAnalyzer::Analyze(Trace &trace, const string &detType, 
			  const string &detSubtype)
{
    TraceAnalyzer::Analyze(trace, detType, detSubtype);
    
    unsigned int saturation = (unsigned int)trace.GetValue("saturation");
    if(saturation > 0) {
	EndAnalyze();
	return;
    }
    
    double aveBaseline = trace.GetValue("baseline");
    unsigned int maxPos = (unsigned int)trace.GetValue("maxpos");
    
    unsigned int waveformLow = 
	(unsigned int)TimingInformation::GetConstant("waveformLow");
    unsigned int waveformHigh = 
	(unsigned int)TimingInformation::GetConstant("waveformHigh");
    
    unsigned int delay = 2;
    double fraction = 0.25;
    vector<double> cfd;

    Trace::iterator cfdStart = trace.begin();
    advance(cfdStart, (int)(maxPos - waveformLow - 2));
    Trace::iterator cfdStop  = trace.begin();
    advance(cfdStop, (int)(maxPos + waveformHigh));

    for(Trace::iterator it = cfdStart; 	it != cfdStop; it++) {
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

//********************************************************
//********************************************************
//******  DO SOME FITTING WITH THE CFD TEMPORARY !!!!*****
//********************************************************
//********************************************************
    unsigned int size = fitY.size();
    double splineRes = 0.1;
    double splineInputX[size];
    double splineInputY[size];
    
    for(unsigned int i = 0; i < size; i++) {
	splineInputX[i] = fitX.at(i);
	splineInputY[i] = fitY.at(i);
    }

    gsl_interp_accel *trace_acc = 
	gsl_interp_accel_alloc (); //initialize the accelerator 
    gsl_spline *traceSpline = 
	gsl_spline_alloc (gsl_interp_akima, size); //init the spline
    
    gsl_spline_init (traceSpline, splineInputX, splineInputY, size); //spline data

    //Retrieve and store the Splined values. 
    vector<double> splineX, splineY;
    for (double kk = 0; kk < size; kk+=splineRes) {
	splineX.push_back(kk);
	splineY.push_back(gsl_spline_eval(traceSpline, kk, trace_acc));
    }

    vector<double> fittingY;
    vector<double>::iterator itMin = 
	min_element(splineY.begin(), splineY.end());
    itMin++;
    fittingY.insert(fittingY.begin(), 
		    max_element(splineY.begin(), splineY.end()),
		    itMin);

    gsl_spline_free(traceSpline);
    gsl_interp_accel_free(trace_acc);

    const int sizeFit = fittingY.size();
    const gsl_multifit_fdfsolver_type *T;
    gsl_multifit_fdfsolver *s;
    int status;
    unsigned int iter = 0;
    const size_t numParams = 9;
       
    gsl_matrix *covar = gsl_matrix_alloc (numParams, numParams);
    double y[sizeFit], sigma[sizeFit];
    struct CfdAnalyzer::FitData d = 
	{sizeFit, y, sigma};
    gsl_multifit_function_fdf f;
    double x_init[numParams] = 
	{-0.071949, 0.892179, 1.99406, -77.87958, 427.0451, -883.61707, 618.7639,
	 -207.703, -75.575};
    gsl_vector_view x = 
	gsl_vector_view_array (x_init, numParams);
    
    f.f = &FitFunction;
    f.df = &FitFunctionDerivative;
    f.fdf = &FitFunctionDf;
    f.n = sizeFit;
    f.p = numParams;
    f.params = &d;
    
    for(int b = 0; b < sizeFit; b++) {
	y[b] = fittingY.at(b);
	//sigma[b] = trace.GetValue("sigmaBaseline");
	sigma[b] = 1;
    }
    
    T = gsl_multifit_fdfsolver_lmsder;
    s = gsl_multifit_fdfsolver_alloc (T, sizeFit, numParams);
    gsl_multifit_fdfsolver_set (s, &f, &x.vector);
    
   for(unsigned int iter = 0; iter < 1000000; iter++) {
	status = gsl_multifit_fdfsolver_iterate(s);
	
	if(status != GSL_CONTINUE)
	    break;

	if(status)
	    break;
	
	status = gsl_multifit_test_delta (s->dx, s->x,
					  0.001, 0.001);
    }
    
    gsl_multifit_covar (s->J, 0.0, covar);
    
    double chi = gsl_blas_dnrm2(s->f);
    double dof = sizeFit - numParams;
    double chisqPerDof = pow(chi, 2.0)/dof;
    
    double fitResults[8];
    for(int i=0; i < 8; i++)
	fitResults[i] = gsl_vector_get(s->x,i);

    gsl_multifit_fdfsolver_free (s);
    gsl_matrix_free (covar);
    
    cout << fitResults[0] << " " 
	 << fitResults[1] << " " 
	 << fitResults[2] << " " 
	 << fitResults[3] << " "
	 << fitResults[4] << " "
	 << fitResults[5] << " "
	 << fitResults[6] << " "
	 << fitResults[7] << " "
	 << fitResults[8] << endl << endl;
    
    fitY.clear();
    fitX.clear();
    splineY.clear();
    fittingY.clear();
    cfd.clear();

    trace.InsertValue("phase", (-intercept/slope)+maxPos);
    EndAnalyze();
}


//********** Declare Plots **********
void CfdAnalyzer::DeclarePlots (void) const 
{
}

//*********** FitFunction **********
int FitFunction (const gsl_vector * x, void *FitData, 
		 gsl_vector * f)
{
    size_t n = ((struct CfdAnalyzer::FitData *)FitData)->n;
    double *y = ((struct CfdAnalyzer::FitData *)FitData)->y;
    double *sigma = 
	((struct CfdAnalyzer::FitData *) FitData)->sigma;
    
    double alpha   = gsl_vector_get (x, 0);
    double beta    = gsl_vector_get (x, 1);
    double gamma   = gsl_vector_get (x, 2);
    double delta   = gsl_vector_get (x, 3);
    double epsilon = gsl_vector_get (x, 4);
    double zeta    = gsl_vector_get (x, 5);
    double eta     = gsl_vector_get (x, 6);
    double theta   = gsl_vector_get (x, 7);
    double iota    = gsl_vector_get (x, 8);
        
    for(size_t i = 0; i < n; i++) {
	double t = i;
	
	double Yi = alpha*pow(t,8) + beta*pow(t,7) + gamma*pow(t,6) + 
	    delta*pow(t,5) + epsilon*pow(t,4) + zeta*pow(t,3) + eta*pow(t,2) + 
	    theta*pow(t,1) + iota;
	
	gsl_vector_set (f, i, (Yi - y[i])/sigma[i]);
    }
    return GSL_SUCCESS;
}


//********** FitFunctionDerivative **********
int FitFunctionDerivative (const gsl_vector * x, void *FitData, 
			   gsl_matrix * J)
{
    size_t n = ((struct CfdAnalyzer::FitData *)FitData)->n;
    double *sigma = 
	((struct CfdAnalyzer::FitData *) FitData)->sigma;
    
    double dalpha, dbeta, dgamma, ddelta, depsilon, dzeta, deta, dtheta, diota;
    
    for (size_t i = 0; i < n; i++) {
	//Compute the Jacobian
 	double t = i;

	dalpha   = pow(t,8);
	dbeta    = pow(t,7);
	dgamma   = pow(t,6);
	ddelta   = pow(t,5);
	depsilon = pow(t,4);
	dzeta    = pow(t,3);
	deta     = pow(t,2);
	dtheta   = pow(t,1);
	diota    = 1;
        
	gsl_matrix_set (J,i,0, dalpha/sigma[i]);
	gsl_matrix_set (J,i,1, dbeta/sigma[i]);
	gsl_matrix_set (J,i,2, dgamma/sigma[i]);
	gsl_matrix_set (J,i,3, ddelta/sigma[i]);
	gsl_matrix_set (J,i,4, depsilon/sigma[i]);
	gsl_matrix_set (J,i,5, dzeta/sigma[i]);
	gsl_matrix_set (J,i,6, deta/sigma[i]);
	gsl_matrix_set (J,i,7, dtheta/sigma[i]);
	gsl_matrix_set (J,i,8, diota/sigma[i]);
    }
    
    return GSL_SUCCESS;
}


//********** FitFunctionDf **********
int FitFunctionDf (const gsl_vector * x, void *FitData,
		   gsl_vector * f, gsl_matrix * J)
{
    FitFunction (x, FitData, f);
    FitFunctionDerivative (x, FitData, J);
    
    return GSL_SUCCESS;
}
