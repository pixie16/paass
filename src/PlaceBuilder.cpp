/** \file PlaceBuilder.cpp
* \brief Defines the various places for the TreeCorrelator
* \author K. A. Miernik
* \date November 23, 2012
*/
#include <iostream>
#include <sstream>

#include "PlaceBuilder.hpp"
#include "Messenger.hpp"

using namespace std;

Place* PlaceBuilder::create (std::map<std::string, std::string>& params,
                             bool verbose) {
    string type = params["type"];
    if (verbose) {
        Messenger m;
        m.detail("Place type: " + type, 1);
    }
    if (type == "")
        return NULL;
    else if (type == "PlaceDetector")
        return createPlaceDetector(params);
    else if (type == "PlaceThreshold")
        return createPlaceThreshold(params);
    else if (type == "PlaceThresholdOR")
        return createPlaceThresholdOR(params);
    else if (type == "PlaceCounter")
        return createPlaceCounter(params);
    else if (type == "PlaceOR")
        return createPlaceOR(params);
    else if (type == "PlaceAND")
        return createPlaceAND(params);
    else {
        stringstream ss;
        ss << "PlaceBuilder: unknown place type" << type;
        throw GeneralException(ss.str());
        return NULL;
    }
}

Place* PlaceBuilder::createPlaceDetector (std::map<std::string, std::string>& params) {
    bool reset = strings::to_bool(params["reset"]);
    int fifo = strings::to_int(params["fifo"]);
    Place* p = new PlaceDetector(reset, fifo);
    return p;
}

Place* PlaceBuilder::createPlaceThreshold (std::map<std::string, std::string>& params) {
    bool reset = strings::to_bool(params["reset"]);
    int fifo = strings::to_int(params["fifo"]);
    double low_limit = strings::to_double(params["low_limit"]);
    double high_limit = strings::to_double(params["high_limit"]);
    Place* p = new PlaceThreshold(low_limit, high_limit, reset, fifo);
    return p;
}

Place* PlaceBuilder::createPlaceThresholdOR (std::map<std::string, std::string>& params) {
    bool reset = strings::to_bool(params["reset"]);
    int fifo = strings::to_int(params["fifo"]);
    double low_limit = strings::to_double(params["low_limit"]);
    double high_limit = strings::to_double(params["high_limit"]);
    Place* p = new PlaceThresholdOR(low_limit, high_limit, reset, fifo);
    return p;
}

Place* PlaceBuilder::createPlaceCounter (std::map<std::string, std::string>& params) {
    bool reset = strings::to_bool(params["reset"]);
    int fifo = strings::to_int(params["fifo"]);
    Place* p = new PlaceCounter(reset, fifo);
    return p;
}

Place* PlaceBuilder::createPlaceOR (std::map<std::string, std::string>& params) {
    bool reset = strings::to_bool(params["reset"]);
    int fifo = strings::to_int(params["fifo"]);
    Place* p = new PlaceOR(reset, fifo);
    return p;
}

Place* PlaceBuilder::createPlaceAND (std::map<std::string, std::string>& params) {
    bool reset = strings::to_bool(params["reset"]);
    int fifo = strings::to_int(params["fifo"]);
    Place* p = new PlaceAND(reset, fifo);
    return p;
}
