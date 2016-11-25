///@file unittest-WalkCorrector.cpp
///@brief Program that will test functionality of the WalkCorrector
///@author S. V. Paulauskas
///@date November 25, 2016
#include <iostream>

#include <cmath>

#include <UnitTest++.h>

#include "Identifier.hpp"
#include "WalkCorrector.hpp"

using namespace std;

///@brief Test for the method Model_None that will always return a zero.
TEST_FIXTURE(WalkCorrector, Test_Model_None) {
    CHECK(Model_None() == 0.0);
}

///@brief Test for method Model_A
TEST_FIXTURE(WalkCorrector, Test_ModelA) {
    vector<double> par = {0.5,2.1,3.7,0.4,0.1};
    double raw = 20.3;
    double expected = par[0] + par[1] / (par[2] + raw) +
        par[3] * exp(-raw / par[4]);
    double result = Model_A(par, raw);
    CHECK(result == expected);
}

TEST_FIXTURE(WalkCorrector, Test_Model_B1) {
    vector<double> par = {0.5,2.1,3.7,0.4};
    double raw = 20.3;
    double expected = par[0] + (par[1] + par[2] / (raw + 1.0)) *
                                       exp(-raw / par[3]);
    double result = Model_B1(par, raw);
    CHECK(result == expected);
}

TEST_FIXTURE(WalkCorrector, Test_Model_B2) {
    vector<double> par = {0.5,2.1,3.7};
    double raw = 20.3;
    double expected = par[0] + par[1] * exp(-raw / par[2]);
    double result = Model_B2(par, raw);
    CHECK(result == expected);
}

TEST_FIXTURE(WalkCorrector, Test_Model_VS_Low) {
    vector<double> par = {0.5,2.1,3.7};
    double raw = 20.3;
    double expected = 1.09099*log(raw)-7.76641;
    double result = Model_VS(par,raw);
    CHECK(result == expected);
}

TEST_FIXTURE(WalkCorrector, Test_Model_VS_High) {
    vector<double> par = {0.5,2.1,3.7};
    double raw = 4000;
    double expected = 0.0;
    double result = Model_VS(par,raw);
    CHECK(result == expected);
}

TEST_FIXTURE(WalkCorrector, Test_Model_VS) {
    vector<double> par = {0.5,2.1,3.7};
    double raw = 300.;
    double expected = -(9.13743e-12)*pow(raw,3.) + (1.9485e-7)*pow(raw,2.)
                      -0.000163286*raw-2.13918;
    double result = Model_VS(par,raw);
    CHECK(result == expected);
}

TEST_FIXTURE(WalkCorrector, Test_Model_VB) {
    vector<double> par = {0.5,2.1,3.7};
    double raw = 300.;
    double expected = -(1.07908*log10(raw)-8.27739);
    double result = Model_VB(par,raw);
    CHECK(result == expected);
}

TEST_FIXTURE(WalkCorrector, Test_Model_VD) {
    vector<double> par = {0.5,2.1,3.7};
    double raw = 300.;
    double expected = 92.7907602830327 * exp(-raw/186091.225414275) +
                      0.59140785215161 * exp(raw/2068.14618331387) -
                      95.5388835298589;
    double result = Model_VD(par,raw);
    CHECK(result == expected);
}

int main(int argv, char* argc[]) {
//    Identifier id("unit", "test", 123);

    return(UnitTest::RunAllTests());
}
