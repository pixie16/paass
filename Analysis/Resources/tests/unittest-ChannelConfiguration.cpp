///@file unittest-ChannelConfiguration.cpp
///@brief Program that will test functionality of ChannelConfiguration
///@author S. V. Paulauskas
///@date November 25, 2016
#include <iostream>

#include <cmath>

#include <UnitTest++.h>

#include "ChannelConfiguration.hpp"

using namespace std;

///Testing the set/get for the location
TEST_FIXTURE (ChannelConfiguration, Test_GetAndSetLocation) {
    unsigned int loc = 111;
    SetLocation(loc);
    CHECK (GetLocation() == loc);
}

///Testing the set/get for the type
TEST_FIXTURE (ChannelConfiguration, Test_GetAndSetType) {
    string type = "unittest";
    SetType(type);
    CHECK (GetType() == type);
}

///Testing the set/get for the subtype
TEST_FIXTURE (ChannelConfiguration, Test_GetAndSetSubtype) {
    string type = "unittest";
    SetSubtype(type);
    CHECK (GetSubtype() == type);
}

///Testing the set/get for the Trace Delay
TEST_FIXTURE (ChannelConfiguration, Test_GetAndSetTraceDelay) {
    unsigned int delay = 123;
    SetTraceDelayInSamples(delay);
    CHECK (GetTraceDelayInSamples() == delay);
}

///Testing the set/get for the Fitting Parameters
TEST_FIXTURE (ChannelConfiguration, Test_GetAndSetFittingPars) {
    pair<double, double> pars(123., 124.);
    SetFittingParameters(pars);
    CHECK (GetFittingParameters() == pars);
}

///Testing the set/get for the Energy Filter Parameters
TEST_FIXTURE (ChannelConfiguration, Test_GetAndSetEnergyFilterPars) {
    TrapFilterParameters pars(111., 222., 333.);
    SetEnergyFilterParameters(pars);
    CHECK (GetEnergyFilterParameters().GetFlattop() == pars.GetFlattop() &&
           GetEnergyFilterParameters().GetRisetime() == pars.GetRisetime() &&
           GetEnergyFilterParameters().GetT() == pars.GetT() );
}

///Testing the set/get for the Trigger Filter Parameters
TEST_FIXTURE (ChannelConfiguration, Test_GetAndSetTriggerFilterPars) {
    TrapFilterParameters pars(111., 222., 333.);
    SetTriggerFilterParameters(pars);
    CHECK (GetTriggerFilterParameters().GetFlattop() == pars.GetFlattop() &&
           GetTriggerFilterParameters().GetRisetime() == pars.GetRisetime() &&
           GetTriggerFilterParameters().GetT() == pars.GetT() );
}

///Testing the set/get for the CFD parameters
TEST_FIXTURE (ChannelConfiguration, Test_GetAndSetCfdPars) {
    tuple<double, double, double> pars(123., 124., 222.);
    SetCfdParameters(pars);
    CHECK (GetCfdParameters() == pars);
}

///Testing the set/get for the Baseline Threshold
TEST_FIXTURE (ChannelConfiguration, Test_GetAndSetBaselineThreshold) {
    double threshold = 12.;
    SetBaselineThreshold(threshold);
    CHECK (GetBaselineThreshold() == threshold);
}

///Testing the set/get for the Waveform Range
TEST_FIXTURE (ChannelConfiguration, Test_GetAndSetWaveformRange) {
    pair<unsigned int, unsigned int> range(12, 122);
    SetWaveformBoundsInSamples(range);
    CHECK (GetWaveformBoundsInSamples().first == range.first && GetWaveformBoundsInSamples().second == range.second);
}

///Testing the set/get for the Discrimination start value
TEST_FIXTURE (ChannelConfiguration, Test_GetAndSetDiscriminationStart) {
    double discrim = 12.;
    SetDiscriminationStartInSamples(discrim);
    CHECK (GetDiscriminationStartInSamples() == discrim);
}

///Testing the set/get for tags
TEST_FIXTURE (ChannelConfiguration, Test_GetAndSetTag) {
    string tag = "unittest";
    AddTag(tag);
    CHECK(HasTag(tag));

    set<string> testset;
    testset.insert(tag);
    CHECK (GetTags() == testset);
}

///Testing the case that we have a missing tag
TEST_FIXTURE (ChannelConfiguration, Test_HasMissingTag) {
    string tag = "unittest";
    CHECK(!HasTag(tag));
}

//Check the comparison operators
TEST_FIXTURE (ChannelConfiguration, Test_Comparison) {
    string type = "unit";
    string subtype = "test";
    unsigned int loc = 112;
    SetSubtype(subtype);
    SetType(type);
    SetLocation(loc);

    ChannelConfiguration id(type, subtype, loc);

    CHECK(*this == id);

    SetLocation(123);
    CHECK(*this > id);

    SetLocation(4);
    CHECK(*this < id);
}

///Testing that the place name is returning the proper value
TEST_FIXTURE (ChannelConfiguration, Test_PlaceName) {
    string type = "unit";
    string subtype = "test";
    unsigned int loc = 112;
    SetSubtype(subtype);
    SetType(type);
    SetLocation(loc);
    CHECK (GetPlaceName() == "unit_test_112");
}

///Test that the zero function is performing it's job properly
TEST_FIXTURE (ChannelConfiguration, Test_Zero) {
    ChannelConfiguration id("unit", "test", 123);
    id.Zero();
    CHECK(*this == id);
}

int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}
