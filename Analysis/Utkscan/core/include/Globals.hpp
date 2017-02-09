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
#include "TrapFilterParameters.hpp"

/** A macro defining what kind of NAN to throw */
#ifndef NAN
#include <limits>
#define NAN (numeric_limits<float>::quiet_NaN())
#endif

///! Namespace defining some information for Timing related stuff
namespace TimingDefs {
    /** Defines an ID for Timing detectors */
    typedef std::pair<unsigned int, std::string> TimingIdentifier;
}

/** \brief Singleton class holding global parameters.*/
class Globals {
public:
    /** \return only instance of Globals class.*/
    static Globals *get();

    /** \return only instance of Globals class.*/
    static Globals *get(const std::string &file);

    ~Globals();

    /** \return true if any reject region was defined */
    bool hasReject() const { return hasReject_; }

    /** \return true if we will define the raw histograms */
    bool hasRaw() const { return (hasRaw_); }

    /** \return the adc clock in seconds */
    double adcClockInSeconds() const { return adcClockInSeconds_; }

    ///\return the bit resolution of the ADC (e.x. 12 bit = 4096)
    double bitResolution() const { return pow(2, bitResolution_); }

    /** \return the length of the big VANDLE bars length in cm */
    double bigLength() const { return (bigLength_); }

    /** \return the length of the big VANDLE bars in ns */
    double bigLengthTime() const { return (bigLength_ / speedOfLightBig_); }

    /** \return the pixie clock in seconds */
    double clockInSeconds() const { return (clockInSeconds_); }

    /** \return the starting point in the trace for the n-gamma discrimination */
    double discriminationStart() const { return (discriminationStart_); }

    /** \return the event size in seconds */
    double eventInSeconds() const { return eventInSeconds_; }

    /** \return the filter clock in seconds */
    double filterClockInSeconds() const { return filterClockInSeconds_; }

    /** \return the length of the medium VANDLE bars in cm */
    double mediumLength() const { return (mediumLength_); }

    /** \return the length of the medium VANDLE bars in ns */
    double mediumLengthTime() const {
        return (mediumLength_ / speedOfLightMedium_);
    }

    /** \return the mass of the neutron in MeV/c/c */
    double neutronMass() const { return (neutronMass_); }

    /** \return the compression factor for the QDCs from the trace (VANDLE related) */
    double qdcCompression() const { return (qdcCompression_); }

    /** \return the length of the small VANDLE bar in cm */
    double smallLength() const { return (smallLength_); }

    /** \return the length of the small VANDLE bar in ns */
    double smallLengthTime() const {
        return (smallLength_ / speedOfLightSmall_);
    }

    /** \return the speed of light in cm/ns */
    double speedOfLight() const { return (speedOfLight_); }

    /** \return the speed of light in the Big VANDLE bars in cm/ns */
    double speedOfLightBig() const { return (speedOfLightBig_); }

    /** \return the speed of light in the medium VANDLE bars in cm/ns */
    double speedOfLightMedium() const { return (speedOfLightMedium_); }

    /** \return the speed of light in the small VANDLE bars in cm/ns */
    double speedOfLightSmall() const { return (speedOfLightSmall_); }

    /** \return the cutoff on the std deviation of the baseline for fitting */
    double sigmaBaselineThresh() const { return (sigmaBaselineThresh_); }

    /** \return the cutoff on the std deviation of the baseline for fitting */
    double
    siPmtSigmaBaselineThresh() const { return (siPmtSigmaBaselineThresh_); }

    /** \return the frequency of the system clock in Hz*/
    double systemClockFreqInHz() const { return (sysClockFreqInHz_); }

    /** \return the trace delay of the traces in ns */
    double traceDelay() const { return (traceDelay_); }

    /** \return the event width */
    int eventWidth() const { return eventWidth_; }

    /** \return the waveform range for standard PMT signals */
    std::pair<unsigned int, unsigned int>
    waveformRange(const std::string &str) const {
        if (waveformRanges_.find(str) != waveformRanges_.end())
            return (waveformRanges_.find(str)->second);
        return (std::make_pair(5, 10));
    }

    /** \return the requested cfd parameters parameters */
    std::pair<double, double> cfdPars(const std::string &str) const {
        if (fitPars_.find(str) != fitPars_.end())
            return (fitPars_.find(str)->second);
        return (std::make_pair(0.5, 1));
    }

    /** \return the requested fitting parameters */
    std::pair<double, double> fitPars(const std::string &str) const {
        if (fitPars_.find(str) != fitPars_.end())
            return (fitPars_.find(str)->second);
        return (std::make_pair(0.254373, 0.208072));
    }

    /** \return the trapezoidal filter parameters for the requested detector type:subtype */
    std::pair<TrapFilterParameters, TrapFilterParameters>
    trapFiltPars(const std::string &str) const {
        if (trapFiltPars_.find(str) != trapFiltPars_.end())
            return (trapFiltPars_.find(str)->second);
        return (std::make_pair(TrapFilterParameters(125, 125, 10),
                               TrapFilterParameters(125, 125, 10)));
    }

    /*! \return returns name of specified output file */
    std::string outputFile() const {return outputFilename_;}

    /*! \return path to use to output files, can be different from output
     * file path
     * \param [in] fileName : the path for the configuration files */
    std::string outputPath(std::string fileName) {
        std::stringstream ss;
        ss << outputPath_ << "/" << fileName;
        return ss.str();
    }

    /** \return the revision for the data */
    std::string revision() const { return (revision_); }

    /** \return the configuration file */
    std::string configfile() const { return (configFile_); }

    /** \return max number of traces stored in 2D spectra
     * with traces. If not set, by default is 16. */
    unsigned short numTraces() const { return numTraces_; }

    /*! \return rejection regions to exclude from scan.
     * Values should be given in seconds in respect to the beginning
     of the file */
    std::vector<std::pair<int, int> > rejectRegions() const { return reject_; };

    /*! Sets output Filename from scan interface */
    void SetOutputFilename(const std::string &a){outputFilename_ = a; }

private:
    /** Default Constructor */
    Globals(const std::string &file);

    Globals(Globals const &);//!< Overload of the constructor
    void operator=(Globals const &); //!< copy constructor
    static Globals *instance; //!< Create the static instance of the class

    /** Check that some of the values make sense */
    void SanityCheck();

    /** Warn that we have an unknown parameter in the XML configuration file
    * \param [in] m : an instance of the messenger to send the warning
    * \param [in] it : an iterator pointing to the location of the unknown */
    void WarnOfUnknownParameter(Messenger &m, pugi::xml_node_iterator &it);

    bool hasReject_;//!< Has a rejection region
    bool hasRaw_; //!< True for plotting Raw Histograms in DAMM

    double adcClockInSeconds_; //!< adc clock in second
    double bitResolution_;//!<The Bit resolution of the digitizer that we used.
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

    int eventWidth_; //!< the size of the events

    std::map<std::string, std::pair<unsigned int, unsigned int> > waveformRanges_; //!< Map containing ranges for the waveforms
    std::map<std::string, std::pair<double, double> > fitPars_; //!< Map containing all of the parameters to be used in the fitting analyzer for a type:subtype
    std::map<std::string, std::pair<double, double> > cfdPars_; //!< Map containing all of the parameters to be used in the cfd analyzer for a type:subtype
    std::map<std::string, std::pair<TrapFilterParameters, TrapFilterParameters> > trapFiltPars_; //!<Map containing all of the trapezoidal filter parameters for a given type:subtype

    std::string outputFilename_; //!<Output Filename
    std::string configFile_;//!< The configuration file
    std::string outputPath_;//!< The path to additional configuration files
    std::string revision_;//!< the pixie revision

    unsigned short numTraces_;//!< number of traces to plot

    std::vector<std::pair<int, int> > reject_;//!< rejection range in time
};

#endif
