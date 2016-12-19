///\file test_gslfitter.cpp
///\brief A small code to test the functionality of the FitDriver
///\author S. V. Paulauskas
///\date August 8, 2016
#include <iostream>

#include "GslFitter.hpp"
#include "UnitTestExampleTrace.hpp"

using namespace std;
using namespace unittest_trace_variables;

int main(int argc, char *argv[]) {
    cout << "Testing functionality of FitDriver and GslFitter" << endl;

    //Qdc of the trace is necessary to initialization of the fit
    double area = 21329.85714285;

    //Instance the fitter and pass in the flag for the SiPm
    GslFitter fitter;

    fitter.SetBaseline(expected_baseline_pair);
    fitter.SetQdc(area);

    double phase;
    //Actually perform the fitting
    ///@TODO : We should never catch ALL throws using this syntax. It's bad
    /// practice. Until we can figure out what to throw here then we'll leave
    /// it.
    try {
        phase = fitter.CalculatePhase(waveform, expected_trace_pars);
    } catch(...) {
        cerr << "Something went wrong with the fit" << endl;
    }

    //Output the fit results and compare to what we get with gnuplot
    cout << "Chi^2 = " << fitter.GetChiSq() << endl
         << "Amplitude from Gnuplot = 0.8565802" << endl
         << "Amplitude = " << fitter.GetAmplitude() << endl
         << "Phase from Gnuplot = -0.0826487" << endl
         << "Phase = " << phase << endl;
}