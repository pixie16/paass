///\file test_gslfitter.cpp
///\brief A small code to test the functionality of the FitDriver
///\author S. V. Paulauskas
///\date August 8, 2016
#include <iostream>

#include "GslFitter.hpp"

using namespace std;

int main(int argc, char* argv[]){
    cout << "Testing functionality of FitDriver and GslFitter" << endl;

    //Baseline for the trace we're going to fit
    double baseline = 436.742857142857;

    //Raw data that we want to fit - This is a VANDLE trace
    vector<double> data {
            437, 501, 1122, 2358, 3509, 3816, 3467, 2921, 2376,
            1914, 1538, 1252, 1043, 877, 750, 667
    };

    //Subtract the baseline from the data
    for(vector<double>::iterator it = data.begin(); it != data.end(); it++ )
        (*it) -= baseline;

    //Set the <beta, gamma> for the fitting
    pair<double,double> pars = make_pair(0.2659404170, 0.208054799179688);

    //Standard deviation of the baseline provides weight
    double weight = 1.9761847389475;

    //Qdc of the trace is necessary to initialization of the fit
    double area = 21329.85714285;

    //We are not fitting a SiPm Fast signal (basically a Gaussian)
    bool isSiPmTiming = false;

    //Instance the fitter and pass in the flag for the SiPm
    GslFitter fitter;

    //Actually perform the fitting
    try {
        fitter.PerformFit(data, pars, isSiPmTiming, weight, area);
    } catch(...) {
        cerr << "Something went wrong with the fit" << endl;
    }

    //Output the fit results and compare to what we get with gnuplot
    cout << "Amplitude = " << fitter.GetAmplitude() << endl
         << "Amplitude from Gnuplot = 0.8565802" << endl
         << "Chi^2 = " << fitter.GetChiSq() << endl
         << "Phase = " << fitter.GetPhase() << endl
         << "Phase from Gnuplot = -0.0826487" << endl;
}