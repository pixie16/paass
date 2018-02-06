/// @file GslFitter.hpp
/// @brief Implementation of the GSL fitting routine for GSL v2+
/// @author S. V. Paulauskas
/// @date August 8, 2016
#ifndef PIXIESUITE_GSLFITTER_HPP
#define PIXIESUITE_GSLFITTER_HPP

#include <iostream>

#include <cmath>

#include <gsl/gsl_blas.h>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_multifit_nlin.h>

#include "TimingDriver.hpp"

class GslFitter : public TimingDriver {
public:
    ///Default Constructor
    GslFitter() : TimingDriver() { isFastSiPm_ = false;isSlowSiPm_ = false; }

    ///Default Destructor
    ~GslFitter() {}

    /// @return the amplitude from the GSL fit
    double GetAmplitude(void) { return amp_; }

    /// @return the chi^2 from the GSL fit
    double GetChiSq(void) { return chi_ * chi_; }

    /// @return the chi^2dof from the GSL fit
    double GetChiSqPerDof(void) { return GetChiSq() / dof_; }

    ///The ever important phase calculation
    /// @param[in] data The baseline subtracted data for the fitting
    /// @param[in] pars The parameters for the fit
    /// @param[in] max : Information about the maximum position and value
    /// @param[in] baseline : The average and standard deviation of the baseline
    double CalculatePhase(const std::vector<double> &data,
                          const std::pair<double, double> &pars,
                          const std::pair<unsigned int, double> &max,
                          const std::pair<double, double> baseline);

    ///Sets the isFastSiPm_ flag
    ///@param[in] a : The value that we are going to set
    void SetIsFastSiPm(const bool &a) { isFastSiPm_ = a; }
    void SetIsSlowSiPm(const bool &a) { isSlowSiPm_ = a; }

    /// @brief Structure necessary for the GSL fitting routines
    struct FitData {
        size_t n;//!< size of the fitting parameters
        double *y;//!< ydata to fit
        double *sigma;//!< weights used for the fit
        double beta; //!< the beta parameter for the fit
        double gamma; //!< the gamma parameter for the fit
        double qdc;//!< the QDC for the fit
    };
private:
    bool isFastSiPm_;
    bool isSlowSiPm_;

    double amp_;
    double chi_;
    double dof_;
};


#endif //PIXIESUITE_GSLFITTER_HPP
