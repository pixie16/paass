/// @file RootFitter.cpp
/// @brief Class to handle fitting traces using ROOT
/// @author S. V. Paulauskas
/// @date December 18, 2016
#include <stdexcept>

#include <TMinuit.h>

#include "RootFitter.hpp"
#include "VandleTimingFunction.hpp"

using namespace std;

RootFitter::RootFitter() {
    vandleTimingFunction_ = new VandleTimingFunction();
    func_ = new TF1("func", vandleTimingFunction_, 0., 1.e6, 5);
}

RootFitter::~RootFitter() {
    delete vandleTimingFunction_;
    delete func_;
}

double RootFitter::CalculatePhase(const std::vector<double> &data,
                                  const std::pair<double, double> &pars,
                                  const std::pair<unsigned int, double> &maxInfo,
                                  std::pair<double, double> baseline) {
    if (data.size() == 0)
        throw range_error("RootFitter::CalculatePhase - The data was sized zero.");

    vector<double> xvals;
    for (unsigned int i = 0; i < data.size(); i++)
        xvals.push_back(double(i));

    TGraphErrors graph((int)data.size(), &(xvals[0]), &(data[0]));

    func_->SetParameters(0, qdc_ * 0.5, pars.first, pars.second, 0);
    func_->SetParLimits(2, 1, 1);
    func_->SetParLimits(3, 1, 1);
    func_->SetParLimits(4, 1, 1);

    graph.Fit(func_, "NRQ", "", 0, data.size());

    return func_->GetParameter(0);
}