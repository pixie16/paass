//
// Created by vincent on 12/18/16.
//

#ifndef PIXIESUITE_UNITTESTEXAMPLETRACE_HPP_HPP
#define PIXIESUITE_UNITTESTEXAMPLETRACE_HPP_HPP

//This is a trace taken from a medium VANDLE module.
static const vector<unsigned int> data = {
        437, 436, 434, 434, 437, 437, 438, 435, 434, 438, 439, 437, 438, 434,
        435, 439, 438, 434, 434, 435, 437, 440, 439, 435, 437, 439, 438, 435,
        436, 436, 437, 439, 435, 433, 434, 436, 439, 441, 436, 437, 439, 438,
        438, 435, 434, 434, 438, 438, 434, 434, 437, 440, 439, 438, 434, 436,
        439, 439, 437, 436, 434, 436, 438, 437, 436, 437, 440, 440, 439, 436,
        435, 437, 501, 1122, 2358, 3509, 3816, 3467, 2921, 2376, 1914, 1538,
        1252, 1043, 877, 750, 667, 619, 591, 563, 526, 458, 395, 403, 452, 478,
        492, 498, 494, 477, 460, 459, 462, 461, 460, 456, 452, 452, 455, 453,
        446, 441, 440, 444, 456, 459, 451, 450, 447, 445, 449, 456, 456, 455
};

//An empty data vector to test error checking.
static const vector<unsigned int> empty_data;
//A data vector that contains constant data.
static const vector<unsigned int> const_data = {1000, 4};
//A data vector to test the integration
static const vector<unsigned int> integration_data = {0, 1, 2, 3, 4, 5};
//The expected value from the CalculateIntegral function
static const double expected_integral = 12.5;

//The expected maximum from the poly3 fitting
static const double expected_poly3_val = 3818.0718412264;
//The expected maximum from the pol2 fitting
static const double expected_poly2_val = 10737.0720588236;

//This is the expected value of the maximum
static const double expected_maximum_value = 3816;
//This is the expected position of the maximum
static const unsigned int expected_max_position = 76;
//This is the pair made from the expected maximum information
static const pair<unsigned int, double> expected_max_info(expected_max_position,
                                                          expected_maximum_value);


#endif //PIXIESUITE_UNITTESTEXAMPLETRACE_HPP_HPP
