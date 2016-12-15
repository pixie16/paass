/// \file Gsl1Fitter.cpp
/// \brief Implementation of the GSL fitting routine for GSL v2+
/// \author S. V. Paulauskas
/// \date August 8, 2016
#include "GslFitter.hpp"

/** Defines the GSL fitting function for standard PMTs
 * \param [in] x : the vector of gsl starting parameters
 * \param [in] FitData : The data to use for the fit
 * \param [in] f : pointer to the function
 * \return an integer that GSL does something magical with */
int PmtFunction(const gsl_vector *x, void *FitData, gsl_vector *f);
/** Defines the GSL fitting function for standard PMTs
 * \param [in] x : the vector of gsl starting parameters
 * \param [in] FitData : The data to use for the fit
 * \param [in] J : pointer to the Jacobian of the function
 * \return an integer that GSL does something magical with */
int CalcPmtJacobian(const gsl_vector *x, void *FitData, gsl_matrix *J);
/** Defines the GSL fitting function for the analysis
 * \param [in] x : the vector of gsl starting parameters
 * \param [in] FitData : The data to use for the fit
 * \param [in] f : pointer to the function
 * \param [in] J : pointer to the Jacobian of the function
 * \return an integer that GSL does something magical with */
int PmtFunctionDerivative(const gsl_vector *x, void *FitData, gsl_vector *f,
                          gsl_matrix *J);
/** Defines the GSL fitting function for the fast output of SiPMTs
 * \param [in] x : the vector of gsl starting parameters
 * \param [in] FitData : The data to use for the fit
 * \param [in] f : pointer to the function
 * \return an integer that GSL does something magical with */
int SiPmtFunction(const gsl_vector *x, void *FitData, gsl_vector *f);
/** Defines the GSL fitting function for the fast output SiPMTs
 * \param [in] x : the vector of gsl starting parameters
 * \param [in] FitData : The data to use for the fit
 * \param [in] J : pointer to the Jacobian of the function
 * \return an integer that GSL does something magical with */
int CalcSiPmtJacobian(const gsl_vector *x, void *FitData, gsl_matrix *J);
/** Defines the GSL fitting function for the fast output of SiPMTs
 * \param [in] x : the vector of gsl starting parameters
 * \param [in] FitData : The data to use for the fit
 * \param [in] f : pointer to the function
 * \param [in] J : pointer to the Jacobian of the function
 * \return an integer that GSL does something magical with */
int SiPmtFunctionDerivative(const gsl_vector *x, void *FitData, gsl_vector *f,
                            gsl_matrix *J);

using namespace std;

void GslFitter::PerformFit(const std::vector<double> &data,
                            const std::pair<double, double> &pars,
                           const bool & isSipmFast/*= false */,
                           const double &weight/* = 1.*/,
                            const double &area/* = 1.*/) {
    gsl_multifit_function_fdf f;
    int status;
    const size_t sizeFit = data.size();
    size_t numParams;
    double xInit[3];

    double y[sizeFit], sigma[sizeFit];
    for(unsigned int i = 0; i < sizeFit; i++) {
        y[i] = data.at(i);
        sigma[i] = weight;
    }

    struct FitDriver::FitData fitData =
            {sizeFit, y, sigma, pars.first, pars.second, area};

    f.n = sizeFit;
    f.params = &fitData;

    if(!isSipmFast) {
        numParams = 2;
        xInit[0] = 0.0;
        xInit[1] = 2.5;
        f.f = &PmtFunction;
        f.df = &CalcPmtJacobian;
        f.fdf = &PmtFunctionDerivative;
    } else {
        numParams = 1;
        xInit[0] = (double)data.size()*0.5;
        f.f = &SiPmtFunction;
        f.df = &CalcSiPmtJacobian;
        f.fdf = &SiPmtFunctionDerivative;
    }
    dof_ = sizeFit - numParams;

    const gsl_multifit_fdfsolver_type *T = gsl_multifit_fdfsolver_lmsder;
    gsl_vector_view x = gsl_vector_view_array (xInit, numParams);
    gsl_multifit_fdfsolver *s = gsl_multifit_fdfsolver_alloc (T, sizeFit, numParams);
    gsl_matrix *covar = gsl_matrix_alloc (numParams, numParams);
    f.p = numParams;
    gsl_multifit_fdfsolver_set (s, &f, &x.vector);

    for(unsigned int iter = 0; iter < 1e8; iter++) {
        status = gsl_multifit_fdfsolver_iterate(s);
        if(status)
            break;
        status = gsl_multifit_test_delta (s->dx, s->x, 1e-4, 1e-4);
        if(status != GSL_CONTINUE)
            break;
    }

    gsl_multifit_covar (s->J, 0.0, covar);
    chi_ = gsl_blas_dnrm2(s->f);

    if(!isSipmFast) {
        phase_ = gsl_vector_get(s->x,0);
        amp_ = gsl_vector_get(s->x,1);
    } else {
        phase_ = gsl_vector_get(s->x,0);
        amp_ = 0.0;
    }

    gsl_multifit_fdfsolver_free (s);
    gsl_matrix_free (covar);
}

int PmtFunction (const gsl_vector * x, void *FitData, gsl_vector * f) {
    size_t n       = ((struct FitDriver::FitData *)FitData)->n;
    double *y      = ((struct FitDriver::FitData *)FitData)->y;
    double *sigma  = ((struct FitDriver::FitData *)FitData)->sigma;
    double beta    = ((struct FitDriver::FitData *)FitData)->beta;
    double gamma   = ((struct FitDriver::FitData *)FitData)->gamma;
    double qdc     = ((struct FitDriver::FitData *)FitData)->qdc;

    double phi     = gsl_vector_get (x, 0);
    double alpha   = gsl_vector_get (x, 1);

    for(size_t i = 0; i < n; i++) {
        double t = i;
        double diff = t-phi;
        double Yi = 0;

        if(t < phi)
            Yi = 0;
        else
            Yi = qdc * alpha * exp(-beta*diff) * (1-exp(-pow(gamma*diff,4.)));

        gsl_vector_set (f, i, (Yi - y[i])/sigma[i]);
    }
    return(GSL_SUCCESS);
}

int CalcPmtJacobian (const gsl_vector * x, void *FitData, gsl_matrix * J) {
    size_t n = ((struct FitDriver::FitData *)FitData)->n;
    double *sigma = ((struct FitDriver::FitData *) FitData)->sigma;
    double beta    = ((struct FitDriver::FitData *)FitData)->beta;
    double gamma   = ((struct FitDriver::FitData *)FitData)->gamma;
    double qdc    = ((struct FitDriver::FitData *)FitData)->qdc;

    double phi     = gsl_vector_get (x, 0);
    double alpha   = gsl_vector_get (x, 1);

    double dphi, dalpha;

    for(size_t i = 0; i < n; i++) {
        double t = i;
        double diff = t-phi;
        double gaussSq = exp(-pow(gamma*diff,4.));
        double s = sigma[i];
        if(t < phi) {
            dphi   = 0;
            dalpha = 0;
        } else {
            dphi = alpha*beta*qdc*exp(-beta*diff)*(1-gaussSq) -
                   4*alpha*qdc*pow(diff,3.)*exp(-beta*diff)*pow(gamma,4.)*gaussSq;
            dalpha = qdc * exp(-beta*diff) * (1-gaussSq);
        }
        gsl_matrix_set(J,i,0, dphi/s);
        gsl_matrix_set(J,i,1, dalpha/s);
    }
    return(GSL_SUCCESS);
}

int PmtFunctionDerivative (const gsl_vector * x, void *FitData, gsl_vector * f,
                           gsl_matrix * J) {
    PmtFunction (x, FitData, f);
    CalcPmtJacobian (x, FitData, J);
    return(GSL_SUCCESS);
}

int SiPmtFunction (const gsl_vector * x, void *FitData, gsl_vector * f) {
    size_t n       = ((struct FitDriver::FitData *)FitData)->n;
    double *y      = ((struct FitDriver::FitData *)FitData)->y;
    double *sigma  = ((struct FitDriver::FitData *)FitData)->sigma;
    double gamma   = ((struct FitDriver::FitData *)FitData)->gamma;
    double qdc     = ((struct FitDriver::FitData *)FitData)->qdc;

    double phi = gsl_vector_get (x, 0);

    for(size_t i = 0; i < n; i++) {
        double t = i;
        double diff = t-phi;
        double Yi = (qdc/(gamma*sqrt(2*M_PI)))*exp(-diff*diff/(2*gamma*gamma));
        gsl_vector_set (f, i, (Yi - y[i])/sigma[i]);
    }
    return(GSL_SUCCESS);
}

int CalcSiPmtJacobian (const gsl_vector * x, void *FitData, gsl_matrix * J) {
    size_t n       = ((struct FitDriver::FitData *)FitData)->n;
    double *sigma  = ((struct FitDriver::FitData *)FitData)->sigma;
    double gamma   = ((struct FitDriver::FitData *)FitData)->gamma;
    double qdc     = ((struct FitDriver::FitData *)FitData)->qdc;

    double phi = gsl_vector_get (x, 0);
    double dphi;

    for (size_t i = 0; i < n; i++) {
        double t = i;
        double diff = t-phi;
        double s = sigma[i];

        dphi = (qdc*diff/(pow(gamma,3)*sqrt(2*M_PI))) *
               exp(-diff*diff/(2*gamma*gamma));

        gsl_matrix_set (J,i,0, dphi/s);
    }
    return(GSL_SUCCESS);
}

int SiPmtFunctionDerivative (const gsl_vector * x, void *FitData, gsl_vector * f,
                             gsl_matrix * J) {
    SiPmtFunction (x, FitData, f);
    CalcSiPmtJacobian (x, FitData, J);
    return(GSL_SUCCESS);
}