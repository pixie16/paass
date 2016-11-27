/// \file FitDriver.hpp
/// \brief An abstract class that will provide the base for fitting
/// \author S. V. Paulauskas
/// \date August 8, 2016

#ifndef PIXIESUITE_FITDRIVER_HPP
#define PIXIESUITE_FITDRIVER_HPP
#include <utility>
#include <vector>

/// A base class that will be used to handle fitting. We currently only support
/// the VANDLE fitting function in this version, which has only two free
/// parameters.
class FitDriver {
public:
    ///Default Constructor
    FitDriver() {};
    ///Default destructor
    virtual ~FitDriver(){};

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
    /// Gets the phase found by the fit. Does nothing by default. We assume
    /// that the children will overload this with their specific implementation.
    ///\return The phase found by the fit
    virtual double GetPhase(void) {return 0.;}
    ///The main driver of the fitting program. Does nothing by default. We assume
    /// that the children will overload this with their specific implementation.
    /// \param[in] data The data that we would like to try and fit
    /// \param[in] pars The parameters for the fit
    /// \param[in] weight The weight for the fit
    virtual void PerformFit(const std::vector<double> &data,
                            const std::pair<double,double> &pars,
                            const bool &isFastSipm,
                            const double &weight = 1.,
                            const double &area = 1.) {};
    /// Sets the data that we are going to fit
    /// \param[in] a Vector containing data to fit
    void SetData(const std::vector<double> &a) {data_ = a;}
    /// Sets the parameters for the fitting functions
    /// \param[in] a pair containing the constants for the fit
    void SetParameters(const std::pair<double,double> &a) {pars_ = a;}
    /// Sets the QDC that's used in the fit
    /// \param[in] a the qdc of the waveform for the fit
    void SetQdc(const double &a) {qdc_ = a;}
    /// Sets the weight for the fit. We will assume for now that the
    /// weight is identical for all points in the fit
    /// \param[in] a weight of the points for the fit
    void SetWeight(const double &a) {weight_ = a;}

    //! Structure necessary for the GSL fitting routines
    struct FitData {
        size_t n;//!< size of the fitting parameters
        double *y;//!< ydata to fit
        double *sigma;//!< weights used for the fit
        double beta; //!< the beta parameter for the fit
        double gamma; //!< the gamma parameter for the fit
        double qdc;//!< the QDC for the fit
    };
protected:
    std::vector<double> data_;//!< Vector of data to fit
    std::pair<double,double> pars_;//!< parameters for the fit function
    double weight_;//!< weight for the fit, assumed constant for all pts
    double qdc_;//!< qdc of the waveform being fitted
};
#endif //PIXIESUITE_FITDRIVER_HPP
