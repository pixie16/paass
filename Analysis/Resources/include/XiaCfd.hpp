///@file TraditionalCfd.hpp
///@brief Same CFD algorithm implemented by Xia LLC but offline.
///@author S. V. Paulauskas
///@date July 22, 2011
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
