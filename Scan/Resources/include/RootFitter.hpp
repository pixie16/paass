/// @file RootFitter.hpp
/// @brief Class to handle fitting traces using ROOT
/// @author S. V. Paulauskas
/// @date December 18, 2016
#ifndef PIXIESUITE_ROOTFITTER_HPP
#define PIXIESUITE_ROOTFITTER_HPP

#include "TimingDriver.hpp"

class RootFitter : public TimingDriver {
public:
    RootFitter() {};

    ~RootFitter() {};

    /// Perform CFD analysis on the waveform using the pol2 algorithm.
    double CalculatePhase(const std::vector<unsigned int> &data,
                          const std::pair<double, double> &pars,
                          const std::pair<unsigned int, double> &maxInfo,
                          std::pair<double, double> baseline);
};


#endif //PIXIESUITE_ROOTFITTER_HPP
