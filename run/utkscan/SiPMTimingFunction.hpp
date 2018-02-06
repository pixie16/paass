/// @file VandleTimingFunction.hpp
/// @brief A class to handle the processing of traces
/// @author D. P. Loureiro
/// @date July 26, 2017
#ifndef __SIPMTIMINGFUNCITON__HPP__
#define __SIPMTIMINGFUNCITON__HPP__

class SiPMTimingFunction {
public:
    SiPMTimingFunction() {};

    virtual ~SiPMTimingFunction() {};

    double operator()(double *x, double *p);
};

#endif
