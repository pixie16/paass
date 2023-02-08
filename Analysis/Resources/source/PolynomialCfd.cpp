/// @file PolynomialCfd.cpp
/// @brief Timing method that calculates the timing using a Polynomial based
/// CFD.
/// @author C. R. Thornsberry and S. V. Paulauskas
/// @date December 6, 2016
#include <cmath>

#include "HelperFunctions.hpp"
#include "PolynomialCfd.hpp"

using namespace std;

PolynomialCfd::PolynomialCfd(){
    polyMethod_ = 1;
}

PolynomialCfd::PolynomialCfd(const int &method){
    
    switch ( method ) {
        case 1:
	    polyMethod_ = method;
            break;
        case 2:
            polyMethod_ = method;
            break;
        default:
            throw GeneralException("PolynomialCfd::Unsupported Interpolation method!");       
    }
    
}


/// Perform CFD analysis on the waveform.
double PolynomialCfd::CalculatePhase(const std::vector<double> &data, const std::pair<double, double> &pars,const std::pair<unsigned int, double> &max, const std::pair<double, double> baseline) {
    if (data.size() == 0)
        throw range_error("PolynomialCfd::CalculatePhase - The data vector was empty!");
    if (data.size() < max.first)
        throw range_error("PolynomialCfd::CalculatePhase - The maximum position is larger than the size of the data vector.");

    double threshold = pars.first * max.second;
    double phase = -9999;
  
    if (polyMethod_ == 1) {
        phase = CalcPoly1Phase(threshold,max.first,data);
        //  cout<< "PolyCFD_method=: "<<polyMethod_<<"   Returned "<< phase << " as the phase      threshold(%) = "<< threshold << " (" << pars.first << ")" <<endl;
    } else if (polyMethod_ == 2){
        phase = CalcPoly2Phase(threshold,max.first,data);
    }
    
    return phase;
}

double PolynomialCfd::CalcPoly1Phase(const double &thresh, const double &traceMax_, const std::vector<double> &data) {
    double pol1Phase_ = -9999;

    vector<double> result;
    for (unsigned int cfdIndex = traceMax_; cfdIndex > 0; cfdIndex--) {
        if (data[cfdIndex - 1] < thresh && data[cfdIndex] >= thresh) {
            // Fit the rise of the trace to a 2nd order polynomial.
            result = Polynomial::CalculatePoly1(data, cfdIndex - 1).second;

            pol1Phase_ = (thresh - result[0]) / result[1];
            break;
        }
    }
    return pol1Phase_;
}

double PolynomialCfd::CalcPoly2Phase(const double &thresh, const double &traceMax_, const std::vector<double> &data){
    double pol2Phase_=-9999;
    float multiplier = 1.;

    vector<double> result;
    for (unsigned int cfdIndex = traceMax_; cfdIndex > 0; cfdIndex--) {
        if (data[cfdIndex - 1] < thresh && data[cfdIndex] >= thresh) {
            // Fit the rise of the trace to a 2nd order polynomial.
            result = Polynomial::CalculatePoly2(data, cfdIndex - 1).second;

            // Calculate the phase of the trace.
            if (result[2] > 1)
                multiplier = -1.;

            pol2Phase_ = (-result[1] + multiplier * sqrt(result[1] * result[1] - 4 * result[2] * (result[0] - thresh))) / (2 * result[2]);

            break;
        }
    }
    return pol2Phase_;
}
