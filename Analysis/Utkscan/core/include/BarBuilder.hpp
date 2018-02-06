/*! \file BarBuilder.hpp
 *  \brief A class to build bar style detectors
 *  \author S. V. Paulauskas
 *  \date December 15, 2014
*/
#ifndef __BARBUILDER_HPP__
#define __BARBUILDER_HPP__

#include "BarDetector.hpp"
#include "HighResTimingData.hpp"

//! Class that builds bars out of a list of ends
class BarBuilder {
public:
    /** Default constructor */
    BarBuilder(){};
    /** Constructor taking the map of channels to build bars with
     * \param [in] vec : Reference to the vector to build channels with */
    BarBuilder(const std::vector<ChanEvent*> vec){list_ = vec;};
    /** Default destructor */
    virtual ~BarBuilder(){};

    /** Gets the built bar map. If you have used the default constructor
     * you must call the BuildBars method <strong> first </strong>.
     * \return A BarMap of the bars having traces. */
    BarMap GetBarMap(void) {return(hrtBars_);};
    
    /** Gets the built bar map. If you have used the default constructor
     * you must call the BuildBars method <strong> first </strong>.
     * \return A BarMap of the bars having no traces */
    std::map<unsigned int, std::pair<double,double> > GetLrtBarMap(void) {return(lrtBars_);};
    
    /** Builds BarDetectors from the individual channel maps. We make assumptions
	that the bars are not going to be vastly out of order, such that the 
	location / 2 = bar number. This is a safe assumption for current 
	(as of 04/22/2016) VANDLE firmware. */
    void BuildBars(void);

    /** Sets the channel list to build bars out of. This list <strong>
     * must </strong> contain both ends of the detector.
     * \param [in] a : The channel list to build bars out of. */
    void SetChannelList(const std::vector<ChanEvent*> a){list_ = a;};
private:
    /** The bar number calculated from the location. We assume here
     * that the bars are located in adjacent slots so that they are always
     * paired in a {0,1} {2,3} {4,5} ... {n,n+1} manner. This is especially
     * true if using a VANDLE firmware (As of 04/22/2016).
     * \param [in] loc : the location of the current event
     * \return The calculated bar number */
    unsigned int CalcBarNumber(const unsigned int &loc);

    /** Clears out the data maps from any previously built bars and ends */
    void ClearMaps(void);

    /** Fills the ends of the detector into two separate maps. Things labeled
     * {left, up,top} are filled into one map, and things labeled {right, down,bottom}
     * are filled into another map. Currently these are the only six
     * recognized end types that one may have, this can be expanded later if
     * others should arise. */
    void FillMaps(void);

    BarMap hrtBars_; //!< Map containing bars with high resolution timing..
    std::map<unsigned int, std::pair<double,double> > lrtBars_; //!<Map with low res bars
    std::map<unsigned int, unsigned int> lefts_; //!< Map containing the left sides of bars
    std::map<unsigned int, unsigned int> rights_; //!< Map containing the left sides of bars
    std::vector<ChanEvent*> list_; //!< Vector of events to build bars out of.
};
#endif // __BARBUILDER_HPP_
