///@file HelperFunctions.hpp
///@brief A file containing stand-alone functions (only depend on standard
/// C++ headers) that are used in the software.
///@author S. V. Paulauskas
///@date December 6, 2016
#ifndef PIXIESUITE_HELPERFUNCTIONS_HPP
#define PIXIESUITE_HELPERFUNCTIONS_HPP

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <cmath>

using namespace std;

namespace Polynomial {
    template<class T>
    static const pair<double, vector<double> > CalculatePoly2(
            const vector<T> &data, const unsigned int &startBin) {
        if (data.size() < 3)
            throw range_error("Polynomial::CalculatePoly2 - The data vector "
                                      "had the wrong size : " + data.size());

        double x1[3], x2[3];
        for (size_t i = 0; i < 3; i++) {
            x1[i] = (startBin + i);
            x2[i] = std::pow(startBin + i, 2);
        }

        double denom =
                (x1[1] * x2[2] - x2[1] * x1[2]) - x1[0] * (x2[2] - x2[1] * 1) +
                x2[0] * (x1[2] - x1[1] * 1);

        double p0 = ((data[x1[0]] * (x1[1] * x2[2] - x2[1] * x1[2]) -
                      x1[0] *
                      (data[x1[1]] * x2[2] - x2[1] * data[x1[2]]) +
                      x2[0] *
                      (data[x1[1]] * x1[2] - x1[1] * data[x1[2]])) / denom);
        double p1 = (((data[x1[1]] * x2[2] - x2[1] * data[x1[2]]) -
                      data[x1[0]] * (x2[2] - x2[1] * 1) +
                      x2[0] * (data[x1[2]] - data[x1[1]] * 1)) / denom);
        double p2 = (((x1[1] * data[x1[2]] - data[x1[1]] * x1[2]) -
                      x1[0] * (data[x1[2]] - data[x1[1]] * 1) +
                      data[x1[0]] * (x1[2] - x1[1] * 1)) / denom);

        //Put the coefficients into a vector in ascending power order
        vector<double> coeffs = {p0, p1, p2};

        return make_pair(p0 - ((p1 * p1) / (4 * p2)), coeffs);
    }

    template<class T>
    static const pair<double, vector<double> > CalculatePoly3(
            const vector<T> &data, const unsigned int &startBin) {
        if (data.size() < 4)
            throw range_error("Polynomial::CalculatePoly3 - The data vector "
                                      "had the wrong size : " + data.size());

        double x1[4], x2[4], x3[4];
        for (size_t i = 0; i < 4; i++) {
            x1[i] = (startBin + i);
            x2[i] = std::pow(startBin + i, 2);
            x3[i] = std::pow(startBin + i, 3);
        }

        double denom = (x1[1] * (x2[2] * x3[3] - x2[3] * x3[2]) -
                        x1[2] * (x2[1] * x3[3] - x2[3] * x3[1]) +
                        x1[3] * (x2[1] * x3[2] - x2[2] * x3[1])) -
                       (x1[0] * (x2[2] * x3[3] - x2[3] * x3[2]) -
                        x1[2] * (x2[0] * x3[3] - x2[3] * x3[0]) +
                        x1[3] * (x2[0] * x3[2] - x2[2] * x3[0])) +
                       (x1[0] * (x2[1] * x3[3] - x2[3] * x3[1]) -
                        x1[1] * (x2[0] * x3[3] - x2[3] * x3[0]) +
                        x1[3] * (x2[0] * x3[1] - x2[1] * x3[0])) -
                       (x1[0] * (x2[1] * x3[2] - x2[2] * x3[1]) -
                        x1[1] * (x2[0] * x3[2] - x2[2] * x3[0]) +
                        x1[2] * (x2[0] * x3[1] - x2[1] * x3[0]));

        double p0 = (double) (
                (data[x1[0]] * (x1[1] * (x2[2] * x3[3] - x2[3] * x3[2]) -
                                x1[2] * (x2[1] * x3[3] - x2[3] * x3[1]) +
                                x1[3] * (x2[1] * x3[2] - x2[2] * x3[1])) -
                 data[x1[1]] * (x1[0] * (x2[2] * x3[3] - x2[3] * x3[2]) -
                                x1[2] * (x2[0] * x3[3] - x2[3] * x3[0]) +
                                x1[3] * (x2[0] * x3[2] - x2[2] * x3[0])) +
                 data[x1[2]] * (x1[0] * (x2[1] * x3[3] - x2[3] * x3[1]) -
                                x1[1] * (x2[0] * x3[3] - x2[3] * x3[0]) +
                                x1[3] * (x2[0] * x3[1] - x2[1] * x3[0])) -
                 data[x1[3]] * (x1[0] * (x2[1] * x3[2] - x2[2] * x3[1]) -
                                x1[1] * (x2[0] * x3[2] - x2[2] * x3[0]) +
                                x1[2] * (x2[0] * x3[1] - x2[1] * x3[0]))) /
                denom);

        double p1 = (double) (((data[x1[1]] * (x2[2] * x3[3] - x2[3] * x3[2]) -
                                data[x1[2]] * (x2[1] * x3[3] - x2[3] * x3[1]) +
                                data[x1[3]] * (x2[1] * x3[2] - x2[2] * x3[1])) -
                               (data[x1[0]] * (x2[2] * x3[3] - x2[3] * x3[2]) -
                                data[x1[2]] * (x2[0] * x3[3] - x2[3] * x3[0]) +
                                data[x1[3]] * (x2[0] * x3[2] - x2[2] * x3[0])) +
                               (data[x1[0]] * (x2[1] * x3[3] - x2[3] * x3[1]) -
                                data[x1[1]] * (x2[0] * x3[3] - x2[3] * x3[0]) +
                                data[x1[3]] * (x2[0] * x3[1] - x2[1] * x3[0])) -
                               (data[x1[0]] * (x2[1] * x3[2] - x2[2] * x3[1]) -
                                data[x1[1]] * (x2[0] * x3[2] - x2[2] * x3[0]) +
                                data[x1[2]] *
                                (x2[0] * x3[1] - x2[1] * x3[0]))) /
                              denom);

        double p2 = (double) (
                ((x1[1] * (data[x1[2]] * x3[3] - data[x1[3]] * x3[2]) -
                  x1[2] * (data[x1[1]] * x3[3] - data[x1[3]] * x3[1]) +
                  x1[3] * (data[x1[1]] * x3[2] - data[x1[2]] * x3[1])) -
                 (x1[0] * (data[x1[2]] * x3[3] - data[x1[3]] * x3[2]) -
                  x1[2] * (data[x1[0]] * x3[3] - data[x1[3]] * x3[0]) +
                  x1[3] * (data[x1[0]] * x3[2] - data[x1[2]] * x3[0])) +
                 (x1[0] * (data[x1[1]] * x3[3] - data[x1[3]] * x3[1]) -
                  x1[1] * (data[x1[0]] * x3[3] - data[x1[3]] * x3[0]) +
                  x1[3] * (data[x1[0]] * x3[1] - data[x1[1]] * x3[0])) -
                 (x1[0] * (data[x1[1]] * x3[2] - data[x1[2]] * x3[1]) -
                  x1[1] * (data[x1[0]] * x3[2] - data[x1[2]] * x3[0]) +
                  x1[2] * (data[x1[0]] * x3[1] - data[x1[1]] * x3[0]))) /
                denom);

        double p3 = (double) (
                ((x1[1] * (x2[2] * data[x1[3]] - x2[3] * data[x1[2]]) -
                  x1[2] * (x2[1] * data[x1[3]] - x2[3] * data[x1[1]]) +
                  x1[3] * (x2[1] * data[x1[2]] - x2[2] * data[x1[1]])) -
                 (x1[0] * (x2[2] * data[x1[3]] - x2[3] * data[x1[2]]) -
                  x1[2] * (x2[0] * data[x1[3]] - x2[3] * data[x1[0]]) +
                  x1[3] * (x2[0] * data[x1[2]] - x2[2] * data[x1[0]])) +
                 (x1[0] * (x2[1] * data[x1[3]] - x2[3] * data[x1[1]]) -
                  x1[1] * (x2[0] * data[x1[3]] - x2[3] * data[x1[0]]) +
                  x1[3] * (x2[0] * data[x1[1]] - x2[1] * data[x1[0]])) -
                 (x1[0] * (x2[1] * data[x1[2]] - x2[2] * data[x1[1]]) -
                  x1[1] * (x2[0] * data[x1[2]] - x2[2] * data[x1[0]]) +
                  x1[2] * (x2[0] * data[x1[1]] - x2[1] * data[x1[0]]))) /
                denom);

        //Put the coefficients into a vector in ascending power order
        vector<double> coeffs = {p0, p1, p2, p3};

        // Calculate the maximum of the polynomial.
        double xmax;
        double node1 =
                (-2 * p2 + std::sqrt(4 * p2 * p2 - 12 * p3 * p1)) / (6 * p3);
        double node2 =
                (-2 * p2 - std::sqrt(4 * p2 * p2 - 12 * p3 * p1)) / (6 * p3);

        //Check if the curvature at node1 is positive or negative. If it is
        // negative then we have the maximum. If not then node2 is the
        // maximum.
        if ((2 * p2 + 6 * p3 * node1) < 0)
            xmax = node1;
        else
            xmax = node2;

        return make_pair(p0 + p1 * xmax + p2 * xmax * xmax +
                         p3 * xmax * xmax * xmax, coeffs);
    }
}//Polynomial namespace

namespace Statistics {
    template<class T>
    inline double CalculateAverage(const vector<T> &data) {
        double sum = 0.0;
        for (typename vector<T>::const_iterator i = data.begin();
             i != data.end(); i++)
            sum += *i;
        sum /= data.size();
        return sum;
    }

    //This calculation for the standard deviation assumes that we are
    // analyzing the full population, which we are in this case.
    template<class T>
    inline double CalculateStandardDeviation(const vector<T> &data,
                                             const double &mean) {
        double stddev = 0.0;
        for (typename vector<T>::const_iterator it = data.begin();
             it != data.end(); it++)
            stddev += pow(*it - mean, 2);
        stddev = sqrt(stddev / (double) data.size());
        return stddev;
    }

    ///@brief Do a quick and simple integration of the provided data using the
    /// trapezoidal rule. We will not be subtracting the baseline or anything
    /// like that to keep things general.
    ///@param[in] data : The data that we want to integrate.
    ///@return The integrated value
    template<class T>
    inline double CalculateIntegral(const vector<T> &data) {
        if (data.size() < 2)
            throw range_error("Statistical::CalculateIntegral - The data "
                                      "vector was too small to integrate. We "
                                      "need at least a size of 2.");
        double integral = 0.0;
        for (unsigned int i = 1; i < data.size(); i++)
            integral += 0.5 * (double(data[i - 1] + data[i]));
        return integral;
    }
}

namespace TraceFunctions {
    ///The minimum length that is necessary for a good baseline calculation.
    static const unsigned int minimum_baseline_length = 5;

    ///@brief Compute the trace baseline and its standard deviation. This
    /// function takes a data range in the event that someone wants to
    /// specify a range that is not at the beginning of the trace, or specify
    /// some other range. The range given must have a minimum given by the
    /// minimum_baseline_length variable.
    ///@param[in] data : The vector of data containing only the baseline.
    ///@param[in] range : the low and high range that we are going to use for
    /// the baseline
    ///@return A pair with the first element being the average of the
    /// baseline and the second element being the standard deviation of the
    /// baseline.
    template<class T>
    inline pair<double, double> CalculateBaseline(
            const vector<T> &data,
            const pair<unsigned int, unsigned int> &range) {
        if (data.size() == 0)
            throw range_error("TraceFunctions::ComputeBaseline - Data vector "
                                      "sized 0");

        if (range.second < range.first)
            throw range_error("TraceFunctions::ComputeBaseline - Bad range : "
                                      "High > Low");

        if (data.size() < (range.second - range.first))
            throw range_error("TraceFunctions::ComputeBaseline - Data vector "
                                      "size is smaller than requested range.");

        if (range.second - range.first < minimum_baseline_length)
            throw range_error("TraceFunctions::ComputeBaseline - The range specified is smaller than the minimum necessary range.");

        double baseline =
                Statistics::CalculateAverage(
                        vector<T>(data.begin(), data.begin() +
                                                range.second));
        double stddev =
                Statistics::CalculateStandardDeviation(
                        vector<T>(data.begin(), data.begin() +
                                                range.second),
                        baseline);
        return make_pair(baseline, stddev);
    }

    ///@brief This function uses a third order polynomial to calculate the
    /// true position of the maximum for the given data. We look at the
    /// points on either side of the maximum value to determine which side
    /// the true maximum lies on. We then pass the data vector and the bin
    /// that we want to start the fit on to the fitting class.
    /// @param[in] data : The data that we would like to find the true
    /// maximum of
    /// @param[in] maxInfo : The low resolution maximum information that we
    /// need to determine where to start the fit.
    /// @return An STL pair containing the maximum that we found and the
    template<class T>
    inline pair<double, vector<double> > ExtrapolateMaximum(
            const vector<T> &data,
            const pair<unsigned int, double> &maxInfo) {
        if (data.size() < 4)
            throw range_error("TraceFunctions::ExtrapolateMaximum - "
                                      "The data vector has less than 4 "
                                      "elements, we need at least four "
                                      "elements to determine the desired "
                                      "information.");
        unsigned int fitStartBin;

        if (data[maxInfo.first - 1] >= data[maxInfo.first + 1])
            fitStartBin = maxInfo.first - 2;
        else
            fitStartBin = maxInfo.first - 1;

        // Find the true maximum by fitting with a third order polynomial.
        return Polynomial::CalculatePoly3(data, fitStartBin);
    }


    ///@brief This function finds the maximum bin and the value of the
    /// maximum bin
    /// for the provided vector. The search is targeted by using the trace 
    /// delay that was set for the traces. At this point we are not going to 
    /// be calculating the high resolution maximum. We need additional 
    /// information about the bit resolution of the module to check for a 
    /// saturated trace. Since that would remove the generality of this 
    /// function we do not perform this check here. It's up to the user to 
    /// verify the results of this function for saturations. 
    /// NOTE: We do not subtract the baseline at this stage.
    /// @param[in] data : The vector of data that we will be using to find
    /// the maximum
    /// @param[in] traceDelayInBins : The value of the trace delay that was
    /// set for this particular trace.
    /// @return A STL pair containing the bin and value of the maximum found
    /// in the trace.
    template<class T>
    inline pair<unsigned int, double> FindMaximum(
            const vector<T> &data,
            const unsigned int &traceDelayInBins) {
        stringstream msg;
        if (data.size() == 0)
            throw range_error("TraceFunctions::FindMaximum - The data was of "
                                      "size 0.");

        //if high bound is outside the trace then we throw a range error.
        if (traceDelayInBins > data.size()) {
            msg << "TraceFunctions::FindMaxiumum - The requested trace delay ("
                << traceDelayInBins << ") was larger than the size of the data "
                << "vector(" << data.size() << ".";
            throw range_error(msg.str());
        }

        //If the trace delay is smaller than the minimum_baseline_length then
        // we will throw an error.
        if (traceDelayInBins < minimum_baseline_length) {
            msg << "TraceFunctions::FindMaximum - The provided traceDelayInBins"
                << "(" << traceDelayInBins << ") was too small it must"
                << " be greater than " << minimum_baseline_length;
            throw range_error(msg.str());
        }

        //We need to target our search so that we do not get traces that are
        // too close to beginning of the trace. The lower bound for the
        // search will be the beginning of the trace plus the
        // minimum_baseline_length.
        typename vector<T>::const_iterator itPos =
                max_element(data.begin() + minimum_baseline_length,
                            data.begin() + traceDelayInBins);


        if (itPos == data.end()) {
            msg << "TraceFunctions::FindMaximum - No maximum could"
                << " be found in the range : [" << minimum_baseline_length
                << "," << traceDelayInBins << "].";
            throw range_error(msg.str());
        }
        return make_pair((unsigned int) (itPos - data.begin()), *itPos);
    }

    ///@TODO Fix this method so that it works properly.
    template<class T>
    inline unsigned int FindLeadingEdge(const vector<T> &data,
                                        const double &fraction,
                                        const pair<unsigned int, double> &maxInfo) {
        if (fraction <= 0)
            throw range_error("TraceFunctions::FindLeadingEdge - The "
                                      "threshold was below zero.");
        if (data.size() < minimum_baseline_length)
            throw range_error("TraceFunctions::FindLeadingEdge - The data "
                                      "vector did not contain enough "
                                      "information to find the leading edge.");
        if (data.size() < maxInfo.first)
            throw range_error("TraceFunctions::FindLeadingEdge - The "
                                      "position of the maximum is outside of "
                                      "the size of the data vector.");

        double threshold = fraction * maxInfo.second;
        unsigned int idx = 9999;
        for (size_t index = maxInfo.first;
             index > minimum_baseline_length; index--)
            if (data[index - 1] < threshold && data[index] >= threshold)
                idx = index - 1;
        return idx;
    }

    ///This is an exclusive calculation, meaning that the value at the low
    /// and high end of the calculation will not be used to calculate the
    /// integral.
    template<class T>
    inline double CalculateQdc(const vector<T> &data,
                               const pair<unsigned int, unsigned int> &range) {
        stringstream msg;
        if (data.size() == 0)
            throw range_error("TraceFunctions::CalculateQdc - The size of "
                                      "the data vector was zero.");
        if (data.size() < range.second) {
            msg << "TraceFunctions::CalculateQdc - The specified "
                << "range was larger than the range : [" << range.first
                << "," << range.second << "].";
            throw range_error(msg.str());
        }

        if(range.first > range.second) {
            msg << "TraceFunctions::CalculateQdc - The specified "
                << "range was inverted.";
            throw range_error(msg.str());
        }
        return Statistics::CalculateIntegral(
                vector<T>(data.begin() + range.first,
                          data.begin() + range.second));
    }

    template<class T>
    inline double CalculateTailRatio(const vector<T> &data,
                                     const pair<unsigned int, unsigned int> &range,
                                     const double &qdc) {
        stringstream msg;
        if (data.size() == 0)
            throw range_error(
                    "TraceFunctions::CalculateTailRatio - The size of "
                            "the data vector was zero.");
        if (data.size() < range.second) {
            msg << "TraceFunctions::CalculateTailRatio - The specified "
                << "range was larger than the range : [" << range.first
                << "," << range.second << "].";
            throw range_error(msg.str());
        }

        if (qdc == 0)
            throw range_error("TraceFunctions::CalculateTailRatio - The QDC "
                                      "had a value of zero. This will cause "
                                      "issues.");

        return Statistics::CalculateIntegral(
                vector<T>(data.begin() + range.first,
                          data.begin() + range.second)) / qdc;

    }

    ///@brief This namespace holds functions that are used to validate the
    /// functions.
    ///@TODO Impelement the validation functions for the functions in the
    /// TraceFunctions namespace. Necessary to simplify the codebase.
    namespace Validation {


    }
}

namespace IeeeStandards {
    ///This function converts an IEEE Floating Point number into a standard
    /// decimal format. This function was stolen almost verbatim from
    /// utilities.c provided by XIA. This data format is used by XIA to store
    /// both TAU and the Baseline. Magic numbers abound since we're
    /// literally following a prescription on how this information is
    /// stored.
    ///https://en.wikipedia.org/wiki/IEEE_floating_point#IEEE_754-2008
    ///@param[in] IeeeFloatingNumber : The IEEE Floating point number that we
    /// want to convert to decimal
    ///@return The decimal number that's been decoded from the input.
    inline double IeeeFloatingToDecimal(
            const unsigned int &IeeeFloatingNumber) {
        double result;
        short signbit = (short)(IeeeFloatingNumber >> 31);
        short exponent = (short)((IeeeFloatingNumber & 0x7F800000) >> 23) - 127;
        double mantissa =
                1.0 + (double)(IeeeFloatingNumber & 0x7FFFFF) / pow(2.0, 23.0);
        if(signbit == 0)
            result = mantissa * pow(2.0, (double)exponent);
        else
            result = - mantissa * pow(2.0, (double)exponent);
        return result;
    }
}

#endif //PIXIESUITE_HELPERFUNCTIONS_HPP
