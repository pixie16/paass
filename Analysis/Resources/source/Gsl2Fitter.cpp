/// \file Gsl2Fitter.cpp
/// \brief Implementation of the GSL fitting routine for GSL v2+
/// \author S. V. Paulauskas, K. Smith
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

using namespace std;

double GslFitter::CalculatePhase(const std::vector<double> &data,
                                 const std::pair<double, double> &pars,
                                 const std::pair<unsigned int, double> &max,
                                 const std::pair<double, double> baseline) {
    gsl_multifit_function_fdf f;
    int info;
    const size_t n = data.size();
    size_t p;
    double xInit[3];

    if (!isFastSiPm_) {
        p = 2;
        xInit[0] = 0.0;
        xInit[1] = 2.5;

        f.f = &PmtFunction;
        f.df = &CalcPmtJacobian;
    } else {
        p = 1;
        xInit[0] = (double) data.size() * 0.5;

        f.f = &SiPmtFunction;
        f.df = &CalcSiPmtJacobian;
    }

    dof_ = n - p;

    const gsl_multifit_fdfsolver_type *T = gsl_multifit_fdfsolver_lmsder;
    gsl_multifit_fdfsolver *s = gsl_multifit_fdfsolver_alloc(T, n, p);
    gsl_matrix *jac = gsl_matrix_alloc(n, p);
    gsl_matrix *covar = gsl_matrix_alloc(p, p);
    double *y = new double[n];
    double *weights = new double[n];
    struct FitData fitData = {n, y, weights, pars.first,
                              pars.second, qdc_};
    gsl_vector_view x = gsl_vector_view_array(xInit, p);
    gsl_vector_view w = gsl_vector_view_array(weights, n);

    static const unsigned int maxIter = 100;
    static const double xtol = 1e-4;
    static const double gtol = 1e-4;
    static const double ftol = 0.0;

    f.n = n;
    f.p = p;
    f.params = &fitData;

    for (unsigned int i = 0; i < n; i++) {
        weights[i] = baseline.second;
        y[i] = data[i];
    }

    gsl_multifit_fdfsolver_wset(s, &f, &x.vector, &w.vector);
    gsl_multifit_fdfsolver_driver(s, maxIter, xtol, gtol, ftol, &info);
    gsl_multifit_fdfsolver_jac(s, jac);
    gsl_multifit_covar(jac, 0.0, covar);

    gsl_vector *res_f = gsl_multifit_fdfsolver_residual(s);
    chi_ = gsl_blas_dnrm2(res_f);

    double phase = 0.0;
    if (!isFastSiPm_) {
        phase = gsl_vector_get(s->x, 0);
        amp_ = gsl_vector_get(s->x, 1);
    } else {
        phase = gsl_vector_get(s->x, 0);
        amp_ = 0.0;
    }

    gsl_multifit_fdfsolver_free(s);
    gsl_matrix_free(covar);
    gsl_matrix_free(jac);
    delete[] y;
    delete[] weights;

    return phase;
}

int PmtFunction(const gsl_vector *x, void *FitData, gsl_vector *f) {
    size_t n = ((struct GslFitter::FitData *) FitData)->n;
    double *y = ((struct GslFitter::FitData *) FitData)->y;
    double beta = ((struct GslFitter::FitData *) FitData)->beta;
    double gamma = ((struct GslFitter::FitData *) FitData)->gamma;
    double qdc = ((struct GslFitter::FitData *) FitData)->qdc;

    double phi = gsl_vector_get(x, 0);
    double alpha = gsl_vector_get(x, 1);

    for (size_t i = 0; i < n; i++) {
        double t = i;
        double diff = t - phi;
        double Yi = 0;

        if (t < phi)
            Yi = 0;
        else
            Yi = qdc * alpha * exp(-beta * diff) *
                 (1 - exp(-pow(gamma * diff, 4.)));

        gsl_vector_set(f, i, Yi - y[i]);
    }
    return (GSL_SUCCESS);
}

int CalcPmtJacobian(const gsl_vector *x, void *FitData, gsl_matrix *J) {
    size_t n = ((struct GslFitter::FitData *) FitData)->n;
    double beta = ((struct GslFitter::FitData *) FitData)->beta;
    double gamma = ((struct GslFitter::FitData *) FitData)->gamma;
    double qdc = ((struct GslFitter::FitData *) FitData)->qdc;

    double phi = gsl_vector_get(x, 0);
    double alpha = gsl_vector_get(x, 1);

    double dphi, dalpha;

    for (size_t i = 0; i < n; i++) {
        double t = i;
        double diff = t - phi;
        double gaussSq = exp(-pow(gamma * diff, 4.));
        if (t < phi) {
            dphi = 0;
            dalpha = 0;
        } else {
            dphi = alpha * beta * qdc * exp(-beta * diff) * (1 - gaussSq) -
                   4 * alpha * qdc * pow(diff, 3.) * exp(-beta * diff) *
                   pow(gamma, 4.) * gaussSq;
            dalpha = qdc * exp(-beta * diff) * (1 - gaussSq);
        }
        gsl_matrix_set(J, i, 0, dphi);
        gsl_matrix_set(J, i, 1, dalpha);
    }
    return (GSL_SUCCESS);
}

int SiPmtFunction(const gsl_vector *x, void *FitData, gsl_vector *f) {
    size_t n = ((struct GslFitter::FitData *) FitData)->n;
    double *y = ((struct GslFitter::FitData *) FitData)->y;
    double gamma = ((struct GslFitter::FitData *) FitData)->gamma;
    double qdc = ((struct GslFitter::FitData *) FitData)->qdc;

    double phi = gsl_vector_get(x, 0);

    for (size_t i = 0; i < n; i++) {
        double t = i;
        double diff = t - phi;
        double Yi = (qdc / (gamma * sqrt(2 * M_PI))) *
                    exp(-diff * diff / (2 * gamma * gamma));
        gsl_vector_set(f, i, Yi - y[i]);
    }
    return (GSL_SUCCESS);
}

int CalcSiPmtJacobian(const gsl_vector *x, void *FitData, gsl_matrix *J) {
    size_t n = ((struct GslFitter::FitData *) FitData)->n;
    double gamma = ((struct GslFitter::FitData *) FitData)->gamma;
    double qdc = ((struct GslFitter::FitData *) FitData)->qdc;

    double phi = gsl_vector_get(x, 0);
    double dphi;

    for (size_t i = 0; i < n; i++) {
        double t = i;
        double diff = t - phi;

        dphi = (qdc * diff / (pow(gamma, 3) * sqrt(2 * M_PI))) *
               exp(-diff * diff / (2 * gamma * gamma));

        gsl_matrix_set(J, i, 0, dphi);
    }
    return (GSL_SUCCESS);
}

