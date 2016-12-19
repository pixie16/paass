//
// Created by vincent on 12/18/16.
//

#ifndef PIXIESUITE_UNITTESTEXAMPLETRACE_HPP_HPP
#define PIXIESUITE_UNITTESTEXAMPLETRACE_HPP_HPP

#include <stdexcept>
#include <utility>
#include <vector>

namespace unittest_trace_variables {
    //This is a trace taken from a medium VANDLE module.
    static const std::vector<unsigned int> data = {
            437, 436, 434, 434, 437, 437, 438, 435, 434, 438, 439, 437, 438,
            434, 435, 439, 438, 434, 434, 435, 437, 440, 439, 435, 437, 439,
            438, 435, 436, 436, 437, 439, 435, 433, 434, 436, 439, 441, 436,
            437, 439, 438, 438, 435, 434, 434, 438, 438, 434, 434, 437, 440,
            439, 438, 434, 436, 439, 439, 437, 436, 434, 436, 438, 437, 436,
            437, 440, 440, 439, 436, 435, 437, 501, 1122, 2358, 3509, 3816,
            3467, 2921, 2376, 1914, 1538, 1252, 1043, 877, 750, 667, 619,
            591, 563, 526, 458, 395, 403, 452, 478, 492, 498, 494, 477, 460,
            459, 462, 461, 460, 456, 452, 452, 455, 453, 446, 441, 440, 444,
            456, 459, 451, 450, 447, 445, 449, 456, 456, 455
    };

    static const std::vector<unsigned int> waveform(data.begin() + 71,
                                                    data.begin() + 86);

    //An empty data vector to test error checking.
    static const std::vector<unsigned int> empty_data;
    //A data vector that contains constant data.
    static const std::vector<unsigned int> const_data = {1000, 4};

    //--------------------------------------------------------------------------
    // Variables related to Testing the integral
    //A data vector to test the integration
    static const std::vector<unsigned int> integration_data =
            {0, 1, 2, 3, 4, 5};
    //The expected value from the CalculateIntegral function
    static const double expected_integral = 12.5;

    //--------------------------------------------------------------------------
    // Variables related to Polynomial::CalculatePoly3

    //The expected maximum from the poly3 fitting
    static const double expected_poly3_val = 3818.0718412264;

    //A data vector that contains only the four points for the Poly3 Fitting.
    static const std::vector<unsigned int> poly3_data(data.begin() + 74,
                                                      data.begin() + 78);
    //A vector containing the coefficients obtained from gnuplot using the data
    // from pol3_data with an x value starting at 0
    static const std::vector<double> expected_poly3_coeffs =
            {2358.0, 1635.66666666667, -516.0, 31.3333333333333};

    //--------------------------------------------------------------------------
    // Variables related to Polynomial::CalculatePoly2
    //The expected maximum from the pol2 fitting
    static const double expected_poly2_val = 10737.0720588236;

    //A data vector containing only the three points for the Poly2 fitting
    static const std::vector<unsigned int> poly2_data(data.begin() + 73,
                                                      data.begin() + 76);
    //Vector containing the expected coefficients from the poly 2 fit
    static const std::vector<double> expected_poly2_coeffs =
            {1122.0, 1278.5, -42.4999999999999};

    //--------------------------------------------------------------------------
    // Variables related to calculation of the maximum of a trace
    /// This is the expected value of the maximum
    static const double expected_maximum_value = 3816;
    /// This is the expected position of the maximum
    static const unsigned int expected_max_position = 76;
    /// This is the pair made from the expected maximum information
    static const std::pair<unsigned int, double> expected_max_info(
            expected_max_position,
            expected_maximum_value);

    //--------------------------------------------------------------------------
    // Variables related to the calculation of the extrapolated maximum
    static const std::vector<double> expected_coeffs =
            {-15641316.0007084, 592747.666694852, -7472.00000037373,
             31.3333333349849};

    //--------------------------------------------------------------------------
    // Variables related to calculation of the baseline of the trace

    //These two values were obtained using the first 70 values of the above trace.
    //The expected baseline value was obtained using the AVERAGE function in
    // Google Sheets.
    static const double expected_baseline = 436.7428571;
    //The expected standard deviation was obtained using the STDEVP function in
    // Google Sheets.
    static const double expected_standard_deviation = 1.976184739;
    //Pair of these results to test with and use in other places
    static const std::pair<double, double> expected_baseline_pair
            (expected_baseline, expected_standard_deviation);
    ///The trace delay in bins for the signal above.
    static const unsigned int trace_delay = 80;

    //--------------------------------------------------------------------------
    // Variables related to calculation of the tail ratio
    static const double expected_ratio = 0.2960894762;

    //--------------------------------------------------------------------------
    //Variables related to fitting the above trace
    //Set the <beta, gamma> for the fitting from the results of a gnuplot script
    std::pair<double, double> expected_trace_pars(0.2659404170,
                                                  0.208054799179688);

}

#endif //PIXIESUITE_UNITTESTEXAMPLETRACE_HPP_HPP
