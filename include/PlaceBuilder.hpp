/** \file PlaceBuilder.hpp
* \brief Defines the various places for the TreeCorrelator
* \author K. A. Miernik
* \date November 23, 2012
*/
#ifndef __PLACEBUILDER_HPP__
#define __PLACEBUILDER_HPP__

#include <string>
#include <map>

#include "Globals.hpp"
#include "Places.hpp"
#include "Exceptions.hpp"

/** \brief Create place, alter or add existing place to the tree.
*
* Accepts map <string, string> to pass place's parameters. The map should
* consist of the following keys:
* "parent" - name of a parent place or "root" if there is no parent
* "name" - name of the place to be added or created, if the name
*          follow a pattern "aaa_bbb_X-Y" where X, Y are integer
*          numbers, it is understood as a range of places, e.g.
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
*   "init" - optional, "false" by default, sets the initial status of the
*            place
*  "range_min" - requaried for PlaceThreshold, sets the lower
*              threshold
*  "range_max" - requaried for PlaceThreshold, sets the high
*              threshold
*/
class PlaceBuilder {
    public:
        /** \return the created place
        * \param [in] params : the parameters used to create the place
        * \param [in] verbose : set the verbosity of the builder */
        Place* create(std::map<std::string, std::string>& params, bool verbose);
    private:
        /** \return the created PlaceDetector
        * \param [in] params : the parameters used to create the place */
        Place* createPlaceDetector(std::map<std::string, std::string>& params);
        /** \return the created PlaceThreshold
        * \param [in] params : the parameters used to create the place */
        Place* createPlaceThreshold(std::map<std::string, std::string>& params);
        /** \return the created PlaceThresholdOR
        * \param [in] params : the parameters used to create the place */
        Place* createPlaceThresholdOR(std::map<std::string, std::string>& params);
        /** \return The created place counter
        * \param [in] params : the parameters used to create the place */
        Place* createPlaceCounter(std::map<std::string, std::string>& params);
        /** \return the created PlaceOR
        * \param [in] params : the parameters used to create the place */
        Place* createPlaceOR(std::map<std::string, std::string>& params);
        /** \return the created PlaceAND
        * \param [in] params : the parameters used to create the place */
        Place* createPlaceAND(std::map<std::string, std::string>& params);
};
#endif
