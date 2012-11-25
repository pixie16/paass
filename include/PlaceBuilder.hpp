#ifndef PLACEBUILDER_H
#define PLACEBUILDER_H

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include "Globals.hpp"
#include "Places.hpp"
#include "Exceptions.hpp"

/** Create place, alter or add existing place to the tree.
* Accepts map <string, string> to pass place's parameters. The map should
* consist of the following keys:
* "parent" - name of a parent place or "root" if there is no parent
* "name" - name of the place to be added or created, if the name
*          follow a pattern "aaa_bbb_X-Y" where X, Y are integer
*          numbers, it is understood a as range of places, e.g.
*          ge_clover_high_0-3
*          will be parsed as ge_clover_high_0, ge_clover_high_1,
*          ge_clover_high_2, ge_clover_high_3
*          with all the other parametrs common for these four places
*          The primary use of this synatax is to include basic places
*          (automatically created) in the tree.
*  "type" - must be of one of existing Places type, currently these
*           are ("PlaceDetector", "PlaceThreshold", "PlaceThresholdOR,
*           "PlaceCounter", "PlaceOR", "PlaceAND").
*
*           If left empty, it will be assumed
*           that this place exists already.
*  "reset" - optional, "true" by default, sets place resetable ("true")
*           or non-resetable ("false").
*  "coincidence" - optional, "true" by default, sets relation with the
*          parent place to coin. ("true") or anti-coin. ("false")
*  "fifo"  - optional, "2" by default, sets the depth of place's fifo
*          queue
*  "range_min" - requaried for PlaceThreshold, sets the lower
*              threshold
*  "range_max" - requaried for PlaceThreshold, sets the high
*              threshold
*/
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
