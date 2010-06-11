/** \file SplineProcessor.cpp
 * 
 * Pulseanalysis class
 *
 * Based on vandle version, 04-18-2010
 * Modifed by D. Miller, April 2010
 */

#include <iostream>
#include <string>

#include <cmath>

#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_vector.h>

#include "damm_plotids.h"
#include "DetectorDriver.h"
#include "RawEvent.h"
#include "SplineProcessor.h"

using namespace dammIds::spline;
using namespace std;

int my_f(const gsl_vector *x, void *data, gsl_vector *f);
int expb_df(const gsl_vector *x, void *data, gsl_matrix *J);
int expb_fdf(const gsl_vector *x, void *data, gsl_vector *f, gsl_matrix *J);

SplineProcessor::SplineProcessor() : EventProcessor()
{
    name = "spline";
    associatedTypes.insert("scint");
    associatedTypes.insert("vandle");
}

SplineProcessor::~SplineProcessor()
{
    // recorded, triggered, and invalid trace output?
}

bool SplineProcessor::Init(DetectorDriver &driver)
{
    if (!EventProcessor::Init(driver))
	return false;

    counter = 0;
    
    return true;
}

void SplineProcessor::DeclarePlots(void) const
{
    const int numTraces = S7;
    const int traceBins = S9;

    DeclareHistogram2D(DD_TRACE_ALL, traceBins, numTraces, "All traces");
    DeclareHistogram2D(DD_TRACE_0, traceBins, numTraces, "big bar traces");
    DeclareHistogram2D(DD_TRACE_1, traceBins, numTraces, "little bar traces");
    DeclareHistogram2D(DD_TRACE_2, traceBins, numTraces, "?? traces");
    // DeclareHistogram2D(DD_TRACE_3, traceBins, numTraces, "?? traces");
}

bool SplineProcessor::Process(RawEvent &event)
{
    ChanEvent *chan;
    DetectorSummary *summary;
    const vector<ChanEvent*> &eventList = event.GetEventList();

    // Start processing for a single event
    for (unsigned int i=0; i < eventList.size(); i++) {
	chan = eventList.at(i);
	Identifier chanID = eventList.at(i)->GetChanID();

	// initialize rawevent variables
	double traceTime   = -9999;
	double hrTime      = -9999;
	double traceEnergy = 0;
	unsigned int maxX = 0;
	int maxValue = 0;

	string type = chanID.GetType();
	string subtype = chanID.GetSubtype();
	summary = event.GetSummary(type);
	int id = chan->GetID();
	double discrim = 0;


	// cout << id << endl;
	
	vector<int> myFullTrace = chan->GetTraceRef();
	for (unsigned int j=0; j < myFullTrace.size(); j++) {
	    Identifier chanID = eventList.at(i)->GetChanID();
	    
	    if (maxValue < myFullTrace[j]) {
		maxValue = myFullTrace[j];
		maxX = j;
	    }

	    // make 2D spectrum of traces
	    set2cc_(DD_TRACE_ALL, j, counter, myFullTrace[j]);	    
	}
	
	double sumOfBaseline = 0;

	for (unsigned int v=0; v < numBinsBaseline; v++) {
	    // traceOffset needed for experiment small traces
	    unsigned int baselineValue = myFullTrace[int(v + baselineOffset)];
	    sumOfBaseline += baselineValue;
	}
	
	double ave = sumOfBaseline / numBinsBaseline;

	double stdDevSum = 0;
	for (unsigned int z=0; z < numBinsBaseline; z++) {
	    unsigned int baselineValue = myFullTrace[z]; //!
	    stdDevSum     += (baselineValue - ave) * (baselineValue - ave);
	}
	double stdDevBaseline = sqrt((1/double(numBinsBaseline))*stdDevSum);

	if (maxValue > (ave + stdDevBaseline) && (maxValue - ave) > 1) {
	    // CREATE THE CUT (different cuts for LG and HG)

	    int lowEnd, highEnd, validTraces = 0;
	    vector<double> traceArrayFit;
	    unsigned int sizeFit;

	    lowEnd = 2;
	    highEnd = 12;
	    
	    for (unsigned int ll =  maxX - lowEnd; ll <= maxX + highEnd; ll++) {
		traceArrayFit.push_back(myFullTrace[ll] - ave);

		if (id == 0)
		    set2cc_(DD_TRACE_0, validTraces, counter, 
			    int(myFullTrace[ll] - ave));
		else if (id == 1)
		    set2cc_(DD_TRACE_1, validTraces, counter, 
			    int(myFullTrace[ll] - ave));
		// not defined
		else if (id == 2 && maxValue - ave > 700) 
		    set2cc_(DD_TRACE_2, validTraces, counter, 
			    int(myFullTrace[ll] - ave));
		// not defined
		// else if (id == 3) 
		//   set2cc_(DD_TRACE_3, validTraces, counter, myFullTrace[ll] - ave);
		validTraces++;
	    }

	    sizeFit = traceArrayFit.size();
	    // calculate energy (integrate pulse)
	    for (unsigned int qq = 0; qq < sizeFit; qq++) {
		traceEnergy += traceArrayFit[qq];
	    }
	    if (subtype == "liquid") {
		for (unsigned int qq = maxX - 5; qq < maxX + 10; qq++) {
		    traceEnergy += myFullTrace[qq] - ave;
		}
	    }

	    // calculate the n-gamma discrimination (SVP)
	    if (subtype == "liquid") {
		for (unsigned int qq = maxX + 5; qq < maxX + 10; qq++) {
		    discrim += myFullTrace[qq] - ave;
		}
	    }

	    // fitting routine
	    const gsl_multifit_fdfsolver_type *T;
	    gsl_multifit_fdfsolver *s;
	    const size_t n = sizeFit;
	    const size_t p = 2; // 3 before mm

	    gsl_matrix *covar = gsl_matrix_alloc(p,p);
	    double y[sizeFit], sigma[sizeFit];
	    struct data d = {n, y, sigma};
	    gsl_multifit_function_fdf f;
	    double xInit[2] = {2, myFullTrace[maxX] - ave}; //[3] before mm
	    gsl_vector_view x = gsl_vector_view_array(xInit, p);

	    f.f      = &my_f;
	    f.df     = &expb_df;
	    f.fdf    = &expb_fdf;
	    f.n      = n;
	    f.p      = p;
	    f.params = &d;

	    for (unsigned int b = 0; b < n; b++) {
		y[b]     = traceArrayFit[b];
		sigma[b] = stdDevBaseline;
	    }

	    T = gsl_multifit_fdfsolver_lmsder;
	    s = gsl_multifit_fdfsolver_alloc(T, n, p);
	    gsl_multifit_fdfsolver_set(s, &f, &x.vector);
	    

	    for (unsigned int iter = 0; iter < 10000000; iter++) {
		int status;

		status = gsl_multifit_fdfsolver_iterate(s);
		if (status) 
		    break;
		
		status = gsl_multifit_test_delta(s->dx, s->x, 0.01, 0.01);
		// previous 1e-7
		if (status != GSL_CONTINUE)
		    break;
	    }
	    // double chi = gsl_blas_dnrm2(s->f);
	    // double dof = n - p;
	    // double chisqPerDof = chi / dof;
	    // cout << chisqPerDof << endl;

	    gsl_multifit_covar(s->J, 0.0, covar);

//!!!!
#define FIT(i) gsl_vector_get(s->x, i)
#define ERR(i) sqrt(gsl_matrix_get(covar, i, i))
//!!!!

	    // set the x-intercept variable
	    traceTime = FIT(0) + maxX;
	    hrTime = traceTime + chan->GetTime();

	    // clean up the GSL fitting routine pointers
	    gsl_multifit_fdfsolver_free(s);
	    gsl_matrix_free(covar);
// END FITTING ROUTINE
	} else {
	    traceEnergy = -9999;
	}

	/*
	chan->SetTraceEnergy(traceEnergy);
	chan->SetTraceTime(traceTime);
	chan->SetHighResTime(hrTime);
	chan->SetMaximumValue(maxValue);
	chan->SetDiscrim(discrim); // SVP set the n-gamma discrimination
	*/

	counter++;
    }

    EndProcess(); // update the processing time
    return true;
}

