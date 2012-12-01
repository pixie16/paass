#include "PlaceBuilder.hpp"

using namespace std;

Place* PlaceBuilder::create (map<string, string>& params) {
    string type = params["type"];
    if (verbose::CORRELATOR_INIT)
        cout << "Type: " << type << endl;
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

Place* PlaceBuilder::createPlaceDetector (map<string, string>& params) {
    bool reset = strings::to_bool(params["reset"]);
    int fifo = strings::to_int(params["fifo"]);
    Place* p = new PlaceDetector(reset, fifo);
    return p;
}

Place* PlaceBuilder::createPlaceThreshold (map<string, string>& params) {
    bool reset = strings::to_bool(params["reset"]);
    int fifo = strings::to_int(params["fifo"]);
    double low_limit = strings::to_double(params["low_limit"]);
    double high_limit = strings::to_double(params["high_limit"]);
    Place* p = new PlaceThreshold(low_limit, high_limit, reset, fifo);
    return p;
}

Place* PlaceBuilder::createPlaceThresholdOR (map<string, string>& params) {
    bool reset = strings::to_bool(params["reset"]);
    int fifo = strings::to_int(params["fifo"]);
    double low_limit = strings::to_double(params["low_limit"]);
    double high_limit = strings::to_double(params["high_limit"]);
    Place* p = new PlaceThresholdOR(low_limit, high_limit, reset, fifo);
    return p;
}

Place* PlaceBuilder::createPlaceCounter (map<string, string>& params) {
    bool reset = strings::to_bool(params["reset"]);
    int fifo = strings::to_int(params["fifo"]);
    Place* p = new PlaceCounter(reset, fifo);
    return p;
}

Place* PlaceBuilder::createPlaceOR (map<string, string>& params) {
    bool reset = strings::to_bool(params["reset"]);
    int fifo = strings::to_int(params["fifo"]);
    Place* p = new PlaceOR(reset, fifo);
    return p;
}

Place* PlaceBuilder::createPlaceAND (map<string, string>& params) {
    bool reset = strings::to_bool(params["reset"]);
    int fifo = strings::to_int(params["fifo"]);
    Place* p = new PlaceAND(reset, fifo);
    return p;
}
