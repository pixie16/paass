/// \file GslFitter.hpp
/// \brief Implementation of the GSL fitting routine for GSL v2+
/// \author S. V. Paulauskas
/// \date August 8, 2016
#ifndef PIXIESUITE_GSLFITTER_HPP
#define PIXIESUITE_GSLFITTER_HPP
#include <iostream>

#include <cmath>

#include <gsl/gsl_blas.h>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_multifit_nlin.h>

#include "FitDriver.hpp"

class GslFitter : public FitDriver{
public:
    ///Default Constructor
    GslFitter() : FitDriver() {};
    ///Default Destructor
    virtual ~GslFitter() {};

    ///\return the phase from the GSL fit
    double GetPhase(void){return phase_;}
    ///\return the amplitude from the GSL fit
    double GetAmplitude(void) {return amp_;}
    ///\return the chi^2 from the GSL fit
    double GetChiSq(void) {return chi_*chi_;}
    ///\return the chi^2dof from the GSL fit
    double GetChiSqPerDof(void) {return GetChiSq()/dof_;}
    ///The main driver for the fitting
    /// \param[in] data The data that we would like to try and fit
    /// \param[in] pars The parameters for the fit
    /// \param[in] weight The weight for the fit
    void PerformFit(const std::vector<double> &data,
                            const std::pair<double,double> &pars,
                            const bool &isSipmFast = false,
                            const double &weight = 1.,
                            const double &area = 1.);
private:
    double amp_;
    double chi_;
    double dof_;
    double phase_;
};


#endif //PIXIESUITE_GSLFITTER_HPP
