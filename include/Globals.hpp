/** \file Globals.hpp
 * \brief constant parameters used in pixie16 analysis
 */
#ifndef __GLOBALS_HPP_
#define __GLOBALS_HPP_

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cstdlib>
#include <stdint.h>

#include "pugixml.hpp"

#include "pixie16app_defs.h"

#include "Exceptions.hpp"
#include "Messenger.hpp"

/** "Constant" constants, i.e. those who won't change going from different
 * verison (revision) of board, some magic numbers used in code etc.
 * For "variable" constants i.e. revision related or experiment related see
 * Globals class. */
namespace pixie {
    typedef uint32_t word_t; //< a pixie word
    typedef uint16_t halfword_t; //< a half pixie word
    typedef uint32_t bufword_t; //< word in a pixie buffer

    /** buffer and module data are terminated with a "-1" value
    *   also used to indicate when a quantity is out of range or peculiar data
    *   this should theoretically be the same as UINT_MAX in climits header
    */
    const pixie::word_t U_DELIMITER = (pixie::word_t)-1;

    /** THIS SHOULD NOT BE SET LARGER THAN 1,000,000
     * this defines the maximum amount of data that will be
     * received in a spill.
     */
    const unsigned int TOTALREAD = 1000000;

    /** An arbitrary vsn used to pass clock data */
    const pixie::word_t clockVsn = 1000;
    /** Number of channels in a module. */
    const size_t numberOfChannels = 16;
};

namespace readbuff {
    const int STATS = -10;
    const int ERROR = -100;
}

/** Some common string conversion operations */
namespace strings {
    /** Converts string to double or throws an exception if not successful */
    inline double to_double (std::string s) {
        std::istringstream iss(s);
        double value;
        if (!(iss >> value)) {
            std::stringstream ss;
            ss << "strings::to_double: Could not convert string '"
               << s << "' to double" << std::endl;
            throw ConversionException(ss.str());
        }
        return value;
    }

    /** Converts string to int or throws an exception if not successful */
    inline int to_int (std::string s) {
        std::istringstream iss(s);
        int value;
        if (!(iss >> value)) {
            std::stringstream ss;
            ss << "strings::to_int: Could not convert string '"
               << s << "' to int" << std::endl;
            throw ConversionException(ss.str());
        }
        return value;
    }

    /** Converts string to bool (True, true, 1 and False, false, 0) are
      * accepted; throws an exception if not succesful. Notice tolower
      * will work only with ascii, not with utf-8, but shouldn't be a
      * problem for true and false words. */
    inline bool to_bool (std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (s == "true" || s == "1")
            return true;
        else if (s == "false" || s == "0")
            return false;
        else {
            std::stringstream ss;
            ss << "strings::to_bool: Could not convert string '"
               << s << "' to bool" << std::endl;
            throw ConversionException(ss.str());
        }
    }

    /** Tokenizes the string, splitting it on a given delimiter.
      * delimiters are removed from returned vector of tokens.*/
    inline std::vector<std::string> tokenize(std::string str,
                                             std::string delimiter) {
        std::string temp;
        std::vector<std::string> tokenized;
        while (str.find(delimiter) != std::string::npos) {
            size_t pos = str.find(delimiter);
            temp = str.substr(0, pos);
            str.erase(0, pos + 1);
            tokenized.push_back(temp);
        }
        tokenized.push_back(str);
        return tokenized;
    }
};

/** Singleton class holding global parameters.*/
class Globals {
public:
    /** \return only instance of Globals class.*/
    static Globals* get();
    ~Globals();

    /** \return true if any reject region was defined */
    bool hasReject() const { return hasReject_; }

    double adcClockInSeconds() const {return adcClockInSeconds_;}
    double clockInSeconds() const {return(clockInSeconds_);}
    double eventInSeconds() const {return eventInSeconds_; }
    double energyContraction() const { return energyContraction_; }
    double filterClockInSeconds() const { return filterClockInSeconds_; }
    double bigLength() const {return(bigLength_);}
    double bigLengthTime() const {return(bigLength_/speedOfLightBig_);}
    double mediumLength() const {return(mediumLength_);}
    double neutronMass() const {return(neutronMass_);}
    double smallLength() const {return(smallLength_);}
    double smallLengthTime() const {return(smallLength_/speedOfLightSmall_);}
    double speedOfLight() const {return(speedOfLight_);}
    double speedOfLightBig() const {return(speedOfLightBig_);}
    double speedOfLightMedium() const {return(speedOfLightMedium_);}
    double speedOfLightSmall() const {return(speedOfLightSmall_);}
    double discriminationStart() const {return(discriminationStart_);}
    double qdcCompression() const {return (qdcCompression_);}
    double sigmaBaselineThresh() const {return(sigmaBaselineThresh_);}
    double traceDelay() const {return(traceDelay_);}
    double traceLength() const {return(traceLength_);}
    double trapezoidalWalk() const {return(trapezoidalWalk_);}
    double waveformHigh() const {return(waveformHigh_);}
    double waveformLow() const {return(waveformLow_);}
    int eventWidth() const { return eventWidth_; }
    std::pair<double,double> vandlePars() {return(vandlePars_);}
    std::pair<double,double> startPars() {return(startPars_);}
    std::pair<double,double> pulserPars() {return(pulserPars_);}
    std::pair<double,double> tvandlePars() {return(tvandlePars_);}
    std::pair<double,double> liquidScintPars() {return(liquidScintPars_);}
    std::pair<double,double> siPmtPars() {return(siPmtPars_);}
    /*! \return Joined path to the passed filename by adding the configPath_
     * This is temporary solution as long as there are some files not
     * incorporated into Config.xml */
    std::string configPath(std::string fileName) {
        std::stringstream ss;
        ss << configPath_ << fileName;
        return ss.str();
    }
    std::string revision() const { return revision_; }
    unsigned int maxWords() const { return maxWords_; }
    /** \return max number of traces stored in 2D spectra
     * with traces. If not set, by default is 16. */
    unsigned short numTraces() const { return numTraces_; }
    /*! \return rejection regions to exclude from scan.
     * Values should be given in seconds in respect to the beginning
     of the file */
    std::vector< std::pair<int, int> > rejectRegions() const {return reject_; };
private:
    /** Make constructor, copy-constructor and operator =
    * private to complete singleton implementation.*/
    Globals();
    Globals(Globals const&);
    void operator=(Globals const&);
    static Globals* instance;

    void SanityCheck();
    void WarnOfUnknownParameter(Messenger &m, pugi::xml_node_iterator &it);

    bool hasReject_;
    double adcClockInSeconds_, clockInSeconds_, energyContraction_,
        eventInSeconds_, filterClockInSeconds_;
    double bigLength_, mediumLength_, neutronMass_, smallLength_,
        speedOfLight_, speedOfLightBig_, speedOfLightMedium_,
        speedOfLightSmall_;
    double discriminationStart_, qdcCompression_, sigmaBaselineThresh_,
        traceDelay_, traceLength_, trapezoidalWalk_, waveformHigh_,
        waveformLow_;
    int eventWidth_;
    std::pair<double,double> vandlePars_, startPars_, pulserPars_,
        tvandlePars_, liquidScintPars_, siPmtPars_;
    std::string configPath_, revision_;
    unsigned int maxWords_;
    unsigned short numTraces_;
    std::vector< std::pair<int, int> > reject_;
};
#endif
