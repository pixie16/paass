//
// Created by vincent on 12/6/16.
//

#ifndef PIXIESUITE_POLYNOMIALCFDCALCULATOR_HPP
#define PIXIESUITE_POLYNOMIALCFDCALCULATOR_HPP

#include <vector>

class PolynomialCfdCalculator : public CfdDriver {
public:
    PolynomialCfdCalculator(){};
    ~PolynomialCfdCalculator(){};

    /// Perform CFD analysis on the waveform using the pol2 algorithm.
    double AnalyzeCFD(const double &f = 0.5,
                      vector<unsigned int> &trc, const
                      double &maximum,
                      const double &baseline);
};

#endif //PIXIESUITE_POLYNOMIALCFD_HPP
