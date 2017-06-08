///@file TraditionalCfd.hpp
///@brief Traditional CFD implemented digitally
///@author S. V. Paulauskas
///@date July 22, 2011

#ifndef PIXIESUITE_TRADITIONALCFD_HPP
#define PIXIESUITE_TRADITIONALCFD_HPP

#include "TimingDriver.hpp"

class TraditionalCfd : public TimingDriver {
public:
    /// Default constructor
    TraditionalCfd() {};

    /// Default destructor
    ~TraditionalCfd() {};

    double CalculatePhase(const std::vector<double> &data,
                          const std::pair<double, double> &pars,
                          const std::pair<unsigned int, double> &max,
                          const std::pair<double, double> baseline);
};

#endif //PIXIESUITE_TRADITIONALCFD_HPP
