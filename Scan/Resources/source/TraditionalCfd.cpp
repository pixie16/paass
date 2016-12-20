///@file TraditionalCfd.cpp
///@brief Traditional CFD implemented digitally
///@author S. V. Paulauskas
///@date July 22, 2011

#include "HelperFunctions.hpp"
#include "TraditionalCfd.hpp"

using namespace std;

double TraditionalCfd::CalculatePhase(const std::vector<unsigned int> &data,
                                      const std::pair<double, double> &pars,
                                      const std::pair<unsigned int, double> &max,
                                      const std::pair<double, double> baseline) {
    if (data.size() == 0)
        throw range_error("PolynomialCfd::CalculatePhase - The data vector "
                                  "was empty!");
    if (data.size() < max.first)
        throw range_error("PolynomialCfd::CalculatePhase - The maximum "
                                  "position is larger than the size of the "
                                  "data vector.");

    unsigned int delay = (unsigned int) pars.second;
    double fraction = pars.first;
    vector<double> cfd;

    //We are going to calculate the CFD here.
    for (unsigned int i = 0; i < data.size() - delay; i++)
        cfd.push_back(fraction * ((double) data[i] - (double) data[i + delay] -
                                  baseline.first));

    //Now we find the maximum and minimum position to locate the zero crossing.
    vector<double>::iterator cfdMin = min_element(cfd.begin(), cfd.end());
    vector<double>::iterator cfdMax = max_element(cfd.begin(), cfd.end());

    vector<double> fitY(cfdMin, cfdMax);
    vector<double> fitX;

    for (int i = int(cfdMin - cfd.begin()); i < int(cfdMax - cfd.begin()); i++)
        fitX.push_back((double) i);

    double num = fitY.size();

    double sumXSq = 0, sumX = 0, sumXY = 0, sumY = 0;

    for (unsigned int i = 0; i < num; i++) {
        sumXSq += fitX.at(i) * fitX.at(i);
        sumX += fitX.at(i);
        sumY += fitY.at(i);
        sumXY += fitX.at(i) * fitY.at(i);
    }

    double deltaPrime = num * sumXSq - sumX * sumX;

    //Rerun the negative of the intercept / slope
    return -((1 / deltaPrime) * (sumXSq * sumY - sumX * sumXY)) /
           ((1 / deltaPrime) * (num * sumXY - sumX * sumY));
}