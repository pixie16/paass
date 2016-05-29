/** \file Globals.hpp
 * \brief constant parameters used in pixie16 analysis
 * \author K. A. Miernik
 */
#ifndef __GLOBALS_HPP_
#define __GLOBALS_HPP_

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cmath>
#include <cstdlib>
#include <stdint.h>

#include "pugixml.hpp"

#include "Exceptions.hpp"
#include "Messenger.hpp"
#include "pixie16app_defs.h"
#include "TrapFilterParameters.hpp"

/** A macro defining what kind of NAN to throw */
#ifndef NAN
#include <limits>
#define NAN (numeric_limits<float>::quiet_NaN())
#endif

/** \brief Pixie module related things that should not change between revisions
 *
 * "Constant" constants, i.e. those who won't change going from different
 * verison (revision) of board, some magic numbers used in code etc.
 * For "variable" constants i.e. revision related or experiment related see
 * Globals class. */
namespace pixie {
    typedef uint32_t word_t; ///< a pixie word
    typedef uint16_t halfword_t; ///< a half pixie word
    typedef uint32_t bufword_t; ///< word in a pixie buffer

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

    /** \return tst bit function from pixie16 files
     * \param [in] bit : bit to test with
     * \param [in] value : value to compare with */
    inline unsigned long TstBit(unsigned short bit, unsigned long value) {
	return ((value & (unsigned long)(pow(2.0, (double)bit))) >> bit);
    }
}

//! Namespace defining some buffer related constants
namespace readbuff {
    const int STATS = -10;//!< a stats buffer
    const int ERROR = -100;//!< Defines an error buffer
}

///! Namespace defining some information for Timing related stuff
namespace TimingDefs {
    /** Defines an ID for Timing detectors */
    typedef std::pair<unsigned int, std::string> TimingIdentifier;
}

//! Some common string conversion operations
namespace strings {
    /** Converts string to double or throws an exception if not successful
    * \param [in] s : String to convert to double
    * \return The double created from the string */
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

    /** Converts string to int or throws an exception if not successful
    * \param [in] s : string to convert to int
    * \return Integer made out of input string */
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
      * problem for true and false words.
      * \param [in] s : String to convert to bool
      * \return A bool from the input string */
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
      * delimiters are removed from returned vector of tokens.
      * \param [in] str : The string to break up
      * \param [in] delimiter : character to break up on
      * \return The vector of tokens */
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
}

/** \brief Singleton class holding global parameters.*/
class Globals {
public:
    /** \return only instance of Globals class.*/
    static Globals* get();
    ~Globals();

    /** \return true if any reject region was defined */
    bool hasReject() const { return hasReject_; }
    bool hasRaw() const {return(hasRaw_);}

    /** \return the adc clock in seconds */
    double adcClockInSeconds() const {return adcClockInSeconds_;}
    /** \return the length of the big VANDLE bars length in cm */
    double bigLength() const {return(bigLength_);}
    /** \return the length of the big VANDLE bars in ns */
    double bigLengthTime() const {return(bigLength_/speedOfLightBig_);}
    /** \return the pixie clock in seconds */
    double clockInSeconds() const {return(clockInSeconds_);}
    /** \return the starting point in the trace for the n-gamma discrimination */
    double discriminationStart() const {return(discriminationStart_);}
    /** \return the event size in seconds */
    double eventInSeconds() const {return eventInSeconds_; }
    /** \return the filter clock in seconds */
    double filterClockInSeconds() const { return filterClockInSeconds_; }
    /** \return the length of the medium VANDLE bars in cm */
    double mediumLength() const {return(mediumLength_);}
    /** \return the length of the medium VANDLE bars in ns */
    double mediumLengthTime() const {return(mediumLength_/speedOfLightMedium_);}
    /** \return the mass of the neutron in MeV/c/c */
    double neutronMass() const {return(neutronMass_);}
    /** \return the compression factor for the QDCs from the trace (VANDLE related) */
    double qdcCompression() const {return (qdcCompression_);}
    /** \return the length of the small VANDLE bar in cm */
    double smallLength() const {return(smallLength_);}
    /** \return the length of the small VANDLE bar in ns */
    double smallLengthTime() const {return(smallLength_/speedOfLightSmall_);}
    /** \return the speed of light in cm/ns */
    double speedOfLight() const {return(speedOfLight_);}
    /** \return the speed of light in the Big VANDLE bars in cm/ns */
    double speedOfLightBig() const {return(speedOfLightBig_);}
    /** \return the speed of light in the medium VANDLE bars in cm/ns */
    double speedOfLightMedium() const {return(speedOfLightMedium_);}
    /** \return the speed of light in the small VANDLE bars in cm/ns */
    double speedOfLightSmall() const {return(speedOfLightSmall_);}
    /** \return the cutoff on the std deviation of the baseline for fitting */
    double sigmaBaselineThresh() const {return(sigmaBaselineThresh_);}
    /** \return the cutoff on the std deviation of the baseline for fitting */
    double siPmtSigmaBaselineThresh() const {return(siPmtSigmaBaselineThresh_);}
    /** \return the frequency of the system clock in Hz*/
    double systemClockFreqInHz() const {return(sysClockFreqInHz_);}
    /** \return the trace delay of the traces in ns */
    double traceDelay() const {return(traceDelay_);}
    /** \return the trace length of the traces in ns */
    double traceLength() const {return(traceLength_);}
    /** \return the approximate size of the trapezoidal walk in ns */
    double trapezoidalWalk() const {return(trapezoidalWalk_);}

    /** \return the event width */
    int eventWidth() const { return eventWidth_; }

    /** \return the waveform range for standard PMT signals */
    std::pair<unsigned int, unsigned int> waveformRange(const std::string &str) const {
	if(waveformRanges_.find(str) != waveformRanges_.end())
	    return(waveformRanges_.find(str)->second);
	return(std::make_pair(5,10));
    }
    /** \return the requested fitting parameters */
    std::pair<double, double> fitPars(const std::string &str) const {
	if(fitPars_.find(str) != fitPars_.end())
	    return(fitPars_.find(str)->second);
	return(std::make_pair(0.254373,0.208072));
    }

    /** \return the trapezoidal filter parameters for the requested detector type:subtype */
    std::pair<TrapFilterParameters,TrapFilterParameters> trapFiltPars(const std::string &str) const { 
	if(trapFiltPars_.find(str) != trapFiltPars_.end())
	    return(trapFiltPars_.find(str)->second);
	return(std::make_pair(TrapFilterParameters(125,125,10),
			      TrapFilterParameters(125,125,10)));
    }



    /*! \return Joined path to the passed filename by adding the configPath_
     * This is temporary solution as long as there are some files not
     * incorporated into Config.xml
     * \param [in] fileName : the path for the configuration files */
    std::string configPath(std::string fileName) {
        std::stringstream ss;
        ss << configPath_ << fileName;
        return ss.str();
    }
    /** \return the revision for the data */
    std::string revision() const { return revision_; }

    /** \return the maximum words */
    unsigned int maxWords() const { return maxWords_; }

    /** \return max number of traces stored in 2D spectra
     * with traces. If not set, by default is 16. */
    unsigned short numTraces() const { return numTraces_; }

    /*! \return rejection regions to exclude from scan.
     * Values should be given in seconds in respect to the beginning
     of the file */
    std::vector< std::pair<int, int> > rejectRegions() const {return reject_; };
private:
    /** Default Constructor */
    Globals();
    Globals(Globals const&);//!< Overload of the constructor
    void operator=(Globals const&); //!< copy constructor
    static Globals* instance; //!< Create the static instance of the class

    /** Check that some of the values make sense */
    void SanityCheck();
    /** Warn that we have an unknown parameter in the XML configuration file
    * \param [in] m : an instance of the messenger to send the warning
    * \param [in] it : an iterator pointing to the location of the unknown */
    void WarnOfUnknownParameter(Messenger &m, pugi::xml_node_iterator &it);

    bool hasReject_;//!< Has a rejection region
    bool hasRaw_; //!< True for plotting Raw Histograms in DAMM

    double adcClockInSeconds_; //!< adc clock in second
    double clockInSeconds_;//!< the ACQ clock in seconds
    double discriminationStart_;//!< starting sample for the n-gamma discrimination
    double eventInSeconds_;//!< event width in seconds
    double filterClockInSeconds_;//!< filter clock in seconds
    double bigLength_;//!< length of big VANDLE bars in cm
    double mediumLength_;//!< length of medium VANDLE bars in cm
    double neutronMass_;//!< mass of neutrons in MeV/c/c
    double qdcCompression_;//!< QDC compression factor for VANDLE related plots
    double smallLength_;//!< length of small VANDLE bars in cm
    double sigmaBaselineThresh_;//!< threshold on fitting for Std dev. of the baseline
    double siPmtSigmaBaselineThresh_;//!< threshold on fitting for Std dev. of the baseline for SiPMTs
    double speedOfLight_;//!< speed of light in cm/ns
    double speedOfLightBig_;//!< speed of light in big VANDLE bars in cm/ns
    double speedOfLightMedium_;//!< speed of light in medium VANDLE bars in cm/ns
    double speedOfLightSmall_;//!< speed of light in small VANDLE bars in cm/ns
    double sysClockFreqInHz_; //!< frequency of the system clock
    double traceDelay_;//!< the trace delay in ns
    double traceLength_;//!< the trace length in ns
    double trapezoidalWalk_;//!< the approximate walk in ns of the trap filter

    int eventWidth_; //!< the size of the events

    std::map<std::string, std::pair<unsigned int, unsigned int> > waveformRanges_; //!< Map containing ranges for the waveforms
    std::map<std::string, std::pair<double,double> > fitPars_; //!< Map containing all of the parameters to be used in the fitting analyzer for a type:subtype
    std::map<std::string, std::pair<TrapFilterParameters,TrapFilterParameters> > trapFiltPars_; //!<Map containing all of the trapezoidal filter parameters for a given type:subtype

    std::string configPath_; //!< configuration path
    std::string revision_;//!< the pixie revision

    unsigned int maxWords_;//!< maximum words in the
    unsigned short numTraces_;//!< number of traces to plot

    std::vector< std::pair<int, int> > reject_;//!< rejection range in time
};
#endif
