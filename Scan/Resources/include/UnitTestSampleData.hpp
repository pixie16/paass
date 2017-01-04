///@file UnitTestSampleData.hpp
///@brief This header provides sample data that can be used by Unit Tests to
/// ensure proper functionality.
///@author S. V. Paulauskas
///@date December 18, 2016

#ifndef PIXIESUITE_UNITTESTSAMPLEDATA_HPP
#define PIXIESUITE_UNITTESTSAMPLEDATA_HPP

#include <utility>
#include <vector>

///This namespace contains the raw channel information that was used to
/// construct the headers in the unittest_encoded_data namespace. These values
/// are also used when testing the data encoding.
namespace unittest_decoded_data {
    //Here is all of the expected data for the above header.
    static const unsigned int channelNumber = 13;
    static const unsigned int crateId = 0;
    static const unsigned int expected_size = 1;
    static const unsigned int ts_high = 26001;
    static const unsigned int ts_low = 123456789;
    static const unsigned int cfd_fractional_time = 1234;
    static const unsigned int slotId = 2;
    static const unsigned int energy = 2345;
    static const unsigned int ex_ts_high = 26001;
    static const unsigned int ex_ts_low = 987654321;
    static const bool cfd_forced_trigger = true;
    static const bool cfd_source_trigger_bit = true;
    static const bool pileup_bit = true;
    static const bool trace_out_of_range = true;
    static const bool virtual_channel = true;

    static const std::vector<unsigned int> energy_sums = {12, 13, 14};

    static const std::vector<unsigned int> qdc = {123, 456, 789, 987, 654,
                                                      321, 135, 791};

    //Need to figure out where to put these as they are Firmware / Frequency
    // specific values. They are for R30747, 250 MS/s.
    static const double ts = 111673568120085;
    static const double ts_w_cfd = 223347136240170.075317;
}

///This namespace contains Firmware / Frequency specific headers that are
/// used to test the decoding of data. The headers that we have here include
/// the 2 words that are inserted by poll2 so that the
/// XiaListModeDataDecoder::DecodeBuffer method will function properly.
namespace unittest_encoded_data {
    //A buffer with zero length
    unsigned int empty_buffer[2] = {0, 0};

    //A buffer that for an empty module
    unsigned int empty_module_buffer[2] = {2, 0};

    ///A header with a header length 20 instead of the true header length 4
    unsigned int header_w_bad_headerlen[6] = {4, 0, 3887149, 123456789, 26001,
                                              2345};

    ///A header where the event length doesn't match what it should be.
    unsigned int header_w_bad_eventlen[6] = {59, 0, 7749677, 123456789, 26001,
                                             8128809};

    namespace R30474_250 {
        //Standard header without anything fancy includes the two additional
        // words we add in poll for the length and the module number
        unsigned int header[6] = {4, 0, 540717, 123456789, 26001, 2345};

        //The header is the standard 4 words. The trace is 62 words, which gives
        // a trace length of 124. This gives us an event length of 66.
        // We have 2 words for the Pixie Module Data Header.
        unsigned int header_N_trace[68] = {
                66, 0, 8667181, 123456789, 26001, 8128809,
                28574133, 28443058, 28639669, 28508598, 28705202, 28639671,
                28443062, 28770739, 28443062, 28508594, 28836277, 28508599,
                28770741, 28508598, 28574132, 28770741, 28377523, 28574130,
                28901815, 28639668, 28705207, 28508598, 28443058, 28705206,
                28443058, 28836277, 28705207, 28574130, 28770743, 28574133,
                28574130, 28639670, 28639668, 28836280, 28574135, 28639667,
                73531893, 229968182, 227217128, 155716457, 100796282, 68355300,
                49152877, 40567451, 36897359, 30016014, 26411403, 31326660,
                32637420, 31261166, 30081484, 30212558, 29884876, 29622724,
                29688263, 28901822, 29098424, 30081480, 29491651, 29163967,
                29884865, 29819336
        };

        //A header that also contains a QDC
        unsigned int header_N_qdc[14] = {
                12, 0, 1622061, 123456789, 26001, 2345,
                123, 456, 789, 987, 654, 321, 135, 791
        };

        //This header has the CFD fractional time set to 1234.
        unsigned int header_N_Cfd[6]{4, 0, 540717, 123456789, 80897425, 2345};

        std::vector<unsigned int> header_vec =
                {540717, 123456789, 26001, 2345};

        std::vector<unsigned int> header_vec_w_trc = {
                8667181, 123456789, 26001, 8128809,
                28574133, 28443058, 28639669, 28508598, 28705202, 28639671,
                28443062, 28770739, 28443062, 28508594, 28836277, 28508599,
                28770741, 28508598, 28574132, 28770741, 28377523, 28574130,
                28901815, 28639668, 28705207, 28508598, 28443058, 28705206,
                28443058, 28836277, 28705207, 28574130, 28770743, 28574133,
                28574130, 28639670, 28639668, 28836280, 28574135, 28639667,
                73531893, 229968182, 227217128, 155716457, 100796282, 68355300,
                49152877, 40567451, 36897359, 30016014, 26411403, 31326660,
                32637420, 31261166, 30081484, 30212558, 29884876, 29622724,
                29688263, 28901822, 29098424, 30081480, 29491651, 29163967,
                29884865, 29819336
        };

        std::vector<unsigned int> header_vec_w_qdc = {
                1622061, 123456789, 26001, 2345,
                123, 456, 789, 987, 654, 321, 135, 791
        };

        std::vector<unsigned int> header_vec_w_qdc_n_trc = {
                9748525, 123456789, 26001, 8128809,
                123, 456, 789, 987, 654, 321, 135, 791,
                28574133, 28443058, 28639669, 28508598, 28705202, 28639671,
                28443062, 28770739, 28443062, 28508594, 28836277, 28508599,
                28770741, 28508598, 28574132, 28770741, 28377523, 28574130,
                28901815, 28639668, 28705207, 28508598, 28443058, 28705206,
                28443058, 28836277, 28705207, 28574130, 28770743, 28574133,
                28574130, 28639670, 28639668, 28836280, 28574135, 28639667,
                73531893, 229968182, 227217128, 155716457, 100796282, 68355300,
                49152877, 40567451, 36897359, 30016014, 26411403, 31326660,
                32637420, 31261166, 30081484, 30212558, 29884876, 29622724,
                29688263, 28901822, 29098424, 30081480, 29491651, 29163967,
                29884865, 29819336
        };
    }
}

namespace unittest_cfd_variables {
    //Pair containing the fraction and the shift
    static const std::pair<double, double> cfd_test_pars(0.5, 2.);
}

namespace unittest_fit_variables {
    //Set the <beta, gamma> for the fitting from the results of a gnuplot script
    static const std::pair<double, double> fitting_parameters(0.2659404170,
                                                              0.2080547991796);
}

namespace unittest_trace_variables {
    //This is a trace taken using a 12-bit 250 MS/s module from a medium VANDLE
    // module.
    static const std::vector<unsigned int> trace = {
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

    static const std::vector<double> trace_sans_baseline = {
            0.257143, -0.742857, -2.74286, -2.74286, 0.257143, 0.257143,
            1.25714, -1.74286, -2.74286, 1.25714, 2.25714, 0.257143, 1.25714,
            -2.74286, -1.74286, 2.25714, 1.25714, -2.74286, -2.74286, -1.74286,
            0.257143, 3.25714, 2.25714, -1.74286, 0.257143, 2.25714, 1.25714,
            -1.74286, -0.742857, -0.742857, 0.257143, 2.25714, -1.74286,
            -3.74286, -2.74286, -0.742857, 2.25714, 4.25714, -0.742857,
            0.257143, 2.25714, 1.25714, 1.25714, -1.74286, -2.74286, -2.74286,
            1.25714, 1.25714, -2.74286, -2.74286, 0.257143, 3.25714, 2.25714,
            1.25714, -2.74286, -0.742857, 2.25714, 2.25714, 0.257143, -0.742857,
            -2.74286, -0.742857, 1.25714, 0.257143, -0.742857, 0.257143,
            3.25714, 3.25714, 2.25714, -0.742857, -1.74286, 0.257143, 64.2571,
            685.257, 1921.26, 3072.26, 3379.26, 3030.26, 2484.26, 1939.26,
            1477.26, 1101.26, 815.257, 606.257, 440.257, 313.257, 230.257,
            182.257, 154.257, 126.257, 89.2571, 21.2571, -41.7429, -33.7429,
            15.2571, 41.2571, 55.2571, 61.2571, 57.2571, 40.2571, 23.2571,
            22.2571, 25.2571, 24.2571, 23.2571, 19.2571, 15.2571, 15.2571,
            18.2571, 16.2571, 9.25714, 4.25714, 3.25714, 7.25714, 19.2571,
            22.2571, 14.2571, 13.2571, 10.2571, 8.25714, 12.2571, 19.2571,
            19.2571, 18.2571
    };

    //This is the region that should be defined as the waveform for the above
    // trace
    static const std::vector<unsigned int> waveform(trace.begin() + 71,
                                                    trace.begin() + 86);

    //An empty data vector to test error checking.
    static const std::vector<unsigned int> empty_vector_uint;
    static const std::vector<double> empty_vector_double;

    //A data vector that contains constant data.
    static const std::vector<unsigned int> const_vector_uint = {1000, 4};

    /// This is the expected value of the maximum
    static const double maximum_value = 3816;

    /// This is the expected position of the maximum
    static const unsigned int max_position = 76;

    /// This is the pair made from the expected maximum information
    static const std::pair<unsigned int, double> max_pair(
            max_position, maximum_value);

    //These two values were obtained using the first 70 values of the above trace.
    //The expected baseline value was obtained using the AVERAGE function in
    // Google Sheets.
    static const double baseline = 436.7428571;

    //The expected standard deviation was obtained using the STDEVP function in
    // Google Sheets.
    static const double standard_deviation = 1.976184739;

    //Pair of these results to test with and use in other places
    static const std::pair<double, double> baseline_pair(baseline,
                                                         standard_deviation);

    ///The trace delay in bins for the signal above.
    static const unsigned int trace_delay = 80;

    // Variables related to calculation of the tail ratio
    static const double tail_ratio = 0.2960894762;

    // Variables related to the calculation of the extrapolated maximum
    static const std::vector<double> extrapolated_max_coeffs =
            {-15641316.0007084, 592747.666694852, -7472.00000037373,
             31.3333333349849};

    //Value of the extrapolated maximum for the above trace
    static const double extrapolated_maximum = 3818.0718412264;
    //Pair of the maximum position and extrapolated maximum
    static const std::pair<unsigned int, double> extrapolated_maximum_pair(
            max_position, extrapolated_maximum);
}

namespace unittest_helper_functions {
    //--------------------------------------------------------------------------
    //A data vector to test the integration
    static const std::vector<unsigned int> integration_data =
            {0, 1, 2, 3, 4, 5};
    //The expected value from the CalculateIntegral function
    static const double integral = 12.5;
    static const std::pair<unsigned int, unsigned int> qdc_pair(2, 5);
    //Expected QDC result of the integral for the above pair
    static const double integration_qdc = 6;


    //--------------------------------------------------------------------------
    //A data vector that contains only the four points for the Poly3 Fitting.
    static const std::vector<unsigned int> poly3_data
            (unittest_trace_variables::trace.begin() + 74,
             unittest_trace_variables::trace.begin() + 78);

    //The expected maximum from the poly3 fitting
    static const double poly3_maximum =
            unittest_trace_variables::extrapolated_maximum;

    //A vector containing the coefficients obtained from gnuplot using the data
    // from pol3_data with an x value starting at 0
    static const std::vector<double> poly3_coeffs =
            {2358.0, 1635.66666666667, -516.0, 31.3333333333333};

    //--------------------------------------------------------------------------
    //A data vector containing only the three points for the Poly2 fitting
    static const std::vector<unsigned int> poly2_data(
            unittest_trace_variables::trace.begin() + 73,
            unittest_trace_variables::trace.begin() + 76);

    // Variables related to Polynomial::CalculatePoly2
    //The expected maximum from the pol2 fitting
    static const double poly2_val = 10737.0720588236;

    //Vector containing the expected coefficients from the poly 2 fit
    static const std::vector<double> poly2_coeffs =
            {1122.0, 1278.5, -42.4999999999999};

    //--------------------------------------------------------------------------
    //This is the expected position of the leading edge of signal.
    static const unsigned int leading_edge_position = 72;
    static const double leading_edge_fraction = 0.5;
    static const double bad_fraction = -0.5;
}

#endif //PIXIESUITE_UNITTESTEXAMPLETRACE_HPP_HPP
