/// @file PolynomialCfd.hpp
/// @brief A method that uses the 
/// @author C. R. Thornsberry and S. V. Paulauskas
/// @date December 6, 2016
#ifndef PIXIESUITE_POLYNOMIALCFD_HPP
#define PIXIESUITE_POLYNOMIALCFD_HPP

#include "Exceptions.hpp"
#include "TimingDriver.hpp"

class PolynomialCfd : public TimingDriver {
public:
    /// Default constructor 
    PolynomialCfd();

    ///Constructor taking poly method 1 or 2 as input
    PolynomialCfd(const int &method);

    ~PolynomialCfd() {};

    /// Perform CFD analysis on the waveform using the pol2 algorithm.
    double CalculatePhase(const std::vector<double> &data,
                          const std::pair<double, double> &pars,
                          const std::pair<unsigned int, double> &max,
                          const std::pair<double, double> baseline);

private:
 /// Calculates the phase using a 2nd order polynomial around the threshold
 double CalcPoly2Phase(const double &thresh, const double &traceMax_, const std::vector<double> &data);

 /// Calculates the phase using a 1st order polynomial (linear) around the threshold
 double CalcPoly1Phase(const double &thresh, const double &traceMax_, const std::vector<double> &data);

 /// order of the polynomial to use around the threshold
 int polyMethod_;
};

#endif //PIXIESUITE_POLYNOMIALCFD_HPP
