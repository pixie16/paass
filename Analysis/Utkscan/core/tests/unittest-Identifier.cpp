///@file unittest-Identifier.cpp
///@brief Program that will test functionality of Identifier
///@author S. V. Paulauskas
///@date November 25, 2016
#include <iostream>

#include <cmath>

#include <UnitTest++.h>

#include "Identifier.hpp"

using namespace std;

///Testing the set/get for the dammid
TEST_FIXTURE (Identifier, Test_GetAndSetDammID) {
    unsigned int dammid = 12;
    SetDammID(dammid);
    CHECK (GetDammID() == dammid);
}

///Testing the set/get for the location
TEST_FIXTURE (Identifier, Test_GetAndSetLocation) {
    unsigned int loc = 111;
    SetLocation(loc);
    CHECK (GetLocation() == loc);
}

///Testing the set/get for the type
TEST_FIXTURE (Identifier, Test_GetAndSetType) {
    string type = "unittest";
    SetType(type);
    CHECK (GetType() == type);
}

///Testing the set/get for the subtype
TEST_FIXTURE (Identifier, Test_GetAndSetSubtype) {
    string type = "unittest";
    SetSubtype(type);
    CHECK (GetSubtype() == type);
}

///Testing the set/get for tags
TEST_FIXTURE (Identifier, Test_GetAndSetTag) {
    string tag = "unittest";
    AddTag(tag);
    CHECK(HasTag(tag));

    set<string> testset;
    testset.insert(tag);
    CHECK (GetTags() == testset);
}

///Testing the case that we have a missing tag
TEST_FIXTURE (Identifier, Test_HasMissingTag) {
    string tag = "unittest";
    CHECK(!HasTag(tag));
}

//Check the comparison operators
TEST_FIXTURE (Identifier, Test_Comparison) {
    string type = "unit";
    string subtype = "test";
    unsigned int loc = 112;
    SetSubtype(subtype);
    SetType(type);
    SetLocation(loc);

    Identifier id(type, subtype, loc);

    CHECK(*this == id);

    SetLocation(123);
    CHECK(*this > id);

    SetLocation(4);
    CHECK(*this < id);
}

///Testing that the place name is returning the proper value
TEST_FIXTURE (Identifier, Test_PlaceName) {
    string type = "unit";
    string subtype = "test";
    unsigned int loc = 112;
    SetSubtype(subtype);
    SetType(type);
    SetLocation(loc);
    CHECK (GetPlaceName() == "unit_test_112");
}

///Test that the zero function is performing it's job properly
TEST_FIXTURE (Identifier, Test_Zero) {
    Identifier id("unit", "test", 123);
    id.Zero();
    CHECK(*this == id);
}

int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}
