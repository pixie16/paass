/// @file PolynomialCfd.hpp
/// @brief A method that uses the 
/// @author C. R. Thornsberry and S. V. Paulauskas
/// @date December 6, 2016
#ifndef PIXIESUITE_POLYNOMIALCFD_HPP
#define PIXIESUITE_POLYNOMIALCFD_HPP

#include "TimingDriver.hpp"

class PolynomialCfd : public TimingDriver {
public:
    PolynomialCfd() {};

    ~PolynomialCfd() {};

    /// Perform CFD analysis on the waveform using the pol2 algorithm.
    double CalculatePhase(const std::vector<unsigned int> &data,
                          const std::pair<double, double> &pars,
                          const std::pair<unsigned int, double> &maxInfo);
};

#endif //PIXIESUITE_POLYNOMIALCFD_HPP
