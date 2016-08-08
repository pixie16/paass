/// \file GslFitter.hpp
/// \brief Implementation of the GSL fitting routine for GSL v2+
/// \author S. V. Paulauskas
/// \date August 8, 2016

#ifndef PIXIESUITE_GSLFITTER_HPP
#define PIXIESUITE_GSLFITTER_HPP

#include "FitDriver.hpp"

class GslFitter : public FitDriver{
public:
    ///Default Constructor
    GslFitter(const bool &isFastSipm) : FitDriver() {isFastSipm_ = isFastSipm;}
    ///Default Destructor
    virtual ~GslFitter(){};

    ///\return the phase from the GSL fit
    virtual double GetPhase(void){return phase_;}
    ///\return the amplitude from the GSL fit
    virtual double GetAmplitude(void) {return amp_;}
    ///\return the chi^2 from the GSL fit
    virtual double GetChiSq(void) {return chisq_;}
    ///\return the chi^2dof from the GSL fit
    virtual double GetChiSqPerDof(void) {return chisq_/dof_;}
    ///The main driver for the fitting
    /// \param[in] data The data that we would like to try and fit
    /// \param[in] pars The parameters for the fit
    /// \param[in] weight The weight for the fit
    virtual void PerformFit(const std::vector<double> &data,
                            const std::pair<double,double> &pars,
                            const double &weight = 1.,
                            const double &area = 1.);
private:
    bool isFastSipm_;

    double amp_;
    double chisq_;
    double dof_;
    double phase_;
};


#endif //PIXIESUITE_GSLFITTER_HPP
