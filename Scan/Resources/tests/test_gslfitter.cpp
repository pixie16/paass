///\file test_gslfitter.cpp
///\brief A small code to test the functionality of the FitDriver
///\author S. V. Paulauskas
///\date August 8, 2016
#include <iostream>

#include "GslFitter.hpp"

using namespace std;

int main(int argc, char *argv[]) {
    cout << "Testing functionality of FitDriver and GslFitter" << endl;

    //Baseline for the trace we're going to fit
    pair<double,double> baseline(436.742857142857, 1.9761847389475);
    //Set the <beta, gamma> for the fitting
    pair<double, double> pars(0.2659404170, 0.208054799179688);
    //Qdc of the trace is necessary to initialization of the fit
    double area = 21329.85714285;

    //Raw data that we want to fit - This is a VANDLE trace
    vector<unsigned int> data{
            437, 501, 1122, 2358, 3509, 3816, 3467, 2921, 2376,
            1914, 1538, 1252, 1043, 877, 750, 667
    };

    //Instance the fitter and pass in the flag for the SiPm
    GslFitter fitter;

    fitter.SetBaseline(baseline);
    fitter.SetQdc(area);

    double phase;
    //Actually perform the fitting
    ///@TODO : We should never catch ALL throws using this syntax. It's bad
    /// practice. Until we can figure out what to throw here then we'll leave
    /// it.
    try {
        phase = fitter.CalculatePhase(data, pars);
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