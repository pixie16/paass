/// @file RootFitter.cpp
/// @brief Class to handle fitting traces using ROOT
/// @author S. V. Paulauskas
/// @date December 18, 2016
#include <stdexcept>

#include <TF1.h>
#include <TFitResult.h>
#include <TGraphErrors.h>

#include "RootFitter.hpp"
#include "VandleTimingFunction.hpp"

using namespace std;

double RootFitter::CalculatePhase(const std::vector<unsigned int> &data,
                                  const std::pair<double, double> &pars,
                                  const std::pair<unsigned int, double> &maxInfo,
                                  std::pair<double, double> baseline) {
    if (data.size() == 0)
        throw range_error("RootFitter::CalculatePhase - The data was sized "
                                  "zero.");

    vector<double> xvals, yvals;
    for (unsigned int i = 0; i < data.size(); i++) {
        xvals.push_back(double(i));
        yvals.push_back(double(data[i]));
    }

    TGraphErrors *graph =
            new TGraphErrors(data.size(), &(xvals[0]), &(yvals[0]));
    for (unsigned int i = 0; i < xvals.size(); i++)
        graph->SetPointError(i, 0.0, baseline.second);

    VandleTimingFunction vandleTimingFunction;
    TF1 func("func", vandleTimingFunction, 0., 1.e6, 5);
    func.SetParameters(0, qdc_ * 0.5, 0.5, 0.5, baseline.first);

    ///@TODO We need to get this working, it's suffering from some strange
    /// issues with ROOT and linked libraries.
    //TFitResultPtr fitResults = graph->Fit(&func, "MENRS", "", 0, data.size());
    //int fitStatus = fitResults;

//    cout << "Fit Status : " << fitStatus << endl;
//    cout << "QDC : " << qdc_ << endl;
//    cout << "Phase : " << func->GetParameter(0) << endl;
//    cout << "Amplitude : " << func->GetParameter(1) << endl;
//    cout << "Beta : " << func->GetParameter(2) << endl;
//    cout << "Gamma : " << func->GetParameter(3) << endl;
//    cout << "Baseline : " << func->GetParameter(4) << endl;
    //delete func;
    return 0.0;
}