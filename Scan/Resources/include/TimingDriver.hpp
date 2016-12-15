/// \file TimingDriver.hpp
/// \brief An abstract class that will provide the base for fitting
/// \author S. V. Paulauskas
/// \date August 8, 2016

#ifndef PIXIESUITE_TIMINGDRIVER_HPP
#define PIXIESUITE_TIMINGDRIVER_HPP
#include <utility>
#include <vector>

/// An abstract class that will be used to handle timing.
class TimingDriver {
public:
    ///Default Constructor
    TimingDriver() {};
    ///Default destructor
    virtual ~TimingDriver(){};

    /// Gets the amplitude found by the fit. Does nothing by default. We assume
    /// that the children will overload this with their specific implementation.
    ///\return The Amplitude found by the fit
    virtual double GetAmplitude(void) {return 0.;}
    /// Gets the Chi^2 of the fit. Does nothing by default. We assume
    /// that the children will overload this with their specific implementation.
    ///\return The Chi^2 from the fit
    virtual double GetChiSq(void){return 0.;}
    /// Gets the Chi^2/dof of the fit. Does nothing by default. We assume
    /// that the children will overload this with their specific implementation.
    ///\return The Chi^2/dof from the fit
    virtual double GetChiSqPerDof(void){return 0.;}

    virtual double CalculatePhase(const std::vector<unsigned int> &data,
                                  const std::pair<double,double> &pars,
                                  const double &maxPosition) {}

    ///The main driver of the fitting program. Does nothing by default. We assume
    /// that the children will overload this with their specific implementation.
    /// \param[in] data The data that we would like to try and fit
    /// \param[in] pars The parameters for the fit
    /// \param[in] weight The weight for the fit
    virtual void PerformFit(const std::vector<double> &data,
                            const std::pair<double,double> &pars,
                            const bool &isFastSipm,
                            const double &weight = 1.,
                            const double &area = 1.) {}

    /// Sets the QDC that's used in the fit
    /// \param[in] a the qdc of the waveform for the fit
    void SetQdc(const double &a) {qdc_ = a;}
    /// Sets the weight for the fit. We will assume for now that the
    /// weight is identical for all points in the fit
    /// \param[in] a weight of the points for the fit
    void SetWeight(const double &a) {weight_ = a;}


protected:
    std::vector<double> data_;//!< Vector of data to fit
    std::pair<double,double> pars_;//!< parameters for the fit function
    double weight_;//!< weight for the fit, assumed constant for all pts
    double qdc_;//!< qdc of the waveform being fitted
};
#endif //PIXIESUITE_TIMINGDRIVER_HPP
