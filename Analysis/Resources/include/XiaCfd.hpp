//
// Created by vincent on 12/6/16.
//

#ifndef PIXIESUITE_XIACFD_HPP
#define PIXIESUITE_XIACFD_HPP


class XiaCfd : public TimingDriver {
public:
    XiaCfd() {};

    ~XiaCfd() {};

    /// Perform CFD analysis on the waveform using the XIA algorithm.
    double CalculatePhase(const double &F_ = 0.5, const size_t &D_ = 1,
                          const size_t &L_ = 1);
};


#endif //PIXIESUITE_XIACFD_HPP
