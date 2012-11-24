#ifndef PLACEBUILDER_H
#define PLACEBUILDER_H

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include "Globals.hpp"
#include "Places.hpp"
#include "Exceptions.hpp"

class PlaceBuilder {
    public:
        Place* create(std::map<std::string, std::string>& params);
    private:
        Place* createPlaceDetector(std::map<std::string, std::string>& params);
        Place* createPlaceThreshold(std::map<std::string, std::string>& params);
        Place* createPlaceThresholdOR(std::map<std::string,
                                      std::string>& params);
        Place* createPlaceCounter(std::map<std::string, std::string>& params);
        Place* createPlaceOR(std::map<std::string, std::string>& params);
        Place* createPlaceAND(std::map<std::string, std::string>& params);
};

#endif
