/// @file VandleTimingFunction.hpp
/// @brief A class to handle the processing of traces
/// @author S. V. Paulauskas
/// @date October 3, 2014
#ifndef __VANDLETIMINGFUNCITON__HPP__
#define __VANDLETIMINGFUNCITON__HPP__

class VandleTimingFunction {
public:
    VandleTimingFunction() {};

    virtual ~VandleTimingFunction() {};

    double operator()(double *x, double *p);
};

#endif
