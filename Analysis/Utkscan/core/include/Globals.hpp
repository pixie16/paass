///@file Globals.hpp
///@brief constant parameters used in pixie16 analysis
///@author D. T. Miller, K. A. Miernik, S. V. Paulauskas
///@date Sometime before June 9, 2012
#ifndef _PAASS_GLOBALS_HPP_
#define _PAASS_GLOBALS_HPP_

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cmath>
#include <cstdlib>
#include <stdint.h>

#include "Constants.hpp"
#include "Exceptions.hpp"
#include "Messenger.hpp"
#include "TrapFilterParameters.hpp"

///! Namespace defining some information for Timing related stuff
namespace TimingDefs {
    ///Defines an ID for Timing detectors 
    typedef std::pair<unsigned int, std::string> TimingIdentifier;
}

///@brief Singleton class holding global parameters.
class Globals {
public:
    ///@return only instance of Globals class.
    static Globals *get();

    ///@return only instance of Globals class.
    static Globals *get(const std::string &file);

    ///The default destructor
    ~Globals();

    ///Method that appends the output path to the provided string.
    ///@param[in] a : The string that we want to append to the output path. Most often this is going to be a filename
    /// of some sort.
    ///@return The concatenation of the provided string and the Output Path
    std::string AppendOutputPath(const std::string &a) { return outputPath_ + a; }

    ///@return the adc clock in seconds
    double GetAdcClockInSeconds() const { return adcClockInSeconds_; }

    ///@param[in] freq : The frequency of the module
    ///@return the correct adc clock conversion factor for the given freq
    double GetAdcClockInSeconds(const int &freq) const {
        if (adcClockTickToSeconds_.find(freq) != adcClockTickToSeconds_.end()) {
            return (adcClockTickToSeconds_.find(freq)->second);
        } else {
            std::cout << "ERROR:: Globals::GetAdcClockInSeconds(): Unknown Frequency, using Revision Default" << std::endl;
            return adcClockInSeconds_;
        }
    }

    ///@return the pixie clock in seconds
    double GetClockInSeconds() const { return clockInSeconds_; }

    ///@param[in] freq : The frequency of the module
    ///@return the correct clock conversion factor for the given freq
    double GetClockInSeconds(const int &freq) const {
        if (clockTickToSeconds_.find(freq) != clockTickToSeconds_.end()) {
            return (clockTickToSeconds_.find(freq)->second);
        } else {
            std::cout << "ERROR:: Globals::GetClockInSeconds(): Unknown Frequency, using Revision Default" << std::endl;
            return clockInSeconds_;
        }
    }

    ///@return the configuration file
    std::string GetConfigFileName() const { return configFile_; }

    ///@return the event size in seconds
    double GetEventLengthInSeconds() const { return eventLengthInSeconds_; }

    ///@return the event width
    unsigned int GetEventLengthInTicks() const { return eventLengthInTicks_; }

    ///@return the filter clock in seconds
    double GetFilterClockInSeconds() const { return filterClockInSeconds_; }

    ///@param[in] freq : The frequency of the module
    ///@return the correct filter clock conversion factor for the given freq
    double GetFilterClockInSeconds(const int &freq) const {
        if (filterClockTickToSeconds_.find(freq) != filterClockTickToSeconds_.end()) {
            return (filterClockTickToSeconds_.find(freq)->second);
        } else {
            std::cout << "ERROR:: Globals::GetFilterClockInSeconds(): Unknown Frequency, using Revision Default" << std::endl;
            return filterClockInSeconds_;
        }
    }

    ///@return returns name of specified output file
    std::string GetOutputFileName() const { return outputFilename_; }

    ///@return Path where additional files will be output.
    std::string GetOutputPath() { return outputPath_; }

    ///@return the revision for the data
    std::string GetPixieRevision() const { return revision_; }

    ///@return rejection regions to exclude from scan.
    std::vector<std::pair<unsigned int, unsigned int> > GetRejectionRegions() const { return reject_; }

    ///@return the frequency of the system clock in Hz
    double GetSystemClockFreqInHz() const { return sysClockFreqInHz_; }

    ///@return the length of the big VANDLE bars in ns
    double GetVandleBigLengthInNs() const { return Vandle::lengthOfBigBarInCm / vandleBigSpeedOfLight_; }

    ///@return the speed of light in the Big VANDLE bars in cm/ns
    double GetVandleBigSpeedOfLightInCmPerNs() const { return vandleBigSpeedOfLight_; }

    ///@return the length of the medium VANDLE bars in ns
    double GetVandleMediumLengthInNs() const { return Vandle::lengthOfMediumBarInCm / vandleMediumSpeedOfLight_; }

    ///@return the speed of light in the medium VANDLE bars in cm/ns
    double GetVandleMediumSpeedOfLightInCmPerNs() const { return vandleMediumSpeedOfLight_; }

    ///@return the length of the small VANDLE bar in ns
    double GetVandleSmallLengthInNs() const { return (Vandle::lengthOfSmallBarInCm / vandleSmallSpeedOfLight_); }

    ///@return the speed of light in the small VANDLE bars in cm/ns
    double GetVandleSmallSpeedOfLightInCmPerNs() const { return vandleSmallSpeedOfLight_; }

   ///@return the speed of light in a NEXT module in cm/ns
   double GetNEXTSpeedOfLightInCmPerNs() const { return nextSpeedOfLight_; }

    ///@return true if any reject region was defined
    bool HasRejectionRegion() const { return !reject_.empty(); }

    ///@return true if we will define the raw histograms
    bool HasRawHistogramsDefined() const { return hasRawHistogramsDefined_; }

    ///Sets the Pixie-16 ADC clock speed in seconds.
    ///@param[in] a : The parameter that we are going to set
    void SetAdcClockInSeconds(const double &a) { adcClockInSeconds_ = a; }

    ///Sets the speed Pixie-16 ADC clock in seconds sensitive to mixed frequecy setups.
    ///@param[in] a : The parameter that we are going to set
    ///@param[in] freq : The Frequency  associated with this conversion
    void SetAdcClockInSeconds( const int &freq, const double &conversionFactor) {
        adcClockTickToSeconds_.emplace(freq, conversionFactor);
    }

    ///Sets the speed Pixie-16 clock in seconds.
    ///@param[in] a : The parameter that we are going to set
    void SetClockInSeconds(const double &a) { clockInSeconds_ = a; }

    ///Sets the speed Pixie-16 clock in seconds sensitive to mixed frequecy setups.
    ///@param[in] a : The parameter that we are going to set
    ///@param[in] freq : The Frequency  associated with this conversion
    void SetClockInSeconds( const int &freq, const double &conversionFactor) {
        clockTickToSeconds_.emplace(freq, conversionFactor);
    }

    ///Sets the event length in seconds that we will use to create events.
    ///@param[in] a : The parameter that we are going to set
    void SetEventLengthInSeconds(const double &a) { eventLengthInSeconds_ = a; }

    ///Sets the event length in clock ticks that we will use to create events.
    ///@param[in] a : The parameter that we are going to set
    void SetEventLengthInTicks(const unsigned int &a) { eventLengthInTicks_ = a; }

    ///Sets the Pixie-16 Filter clock value.
    ///@param[in] a : The parameter that we are going to set
    void SetFilterClockInSeconds( const double &a) { filterClockInSeconds_ = a; }

    ///Sets the speed Pixie-16 Filter clock in seconds sensitive to mixed frequecy setups.
    ///@param[in] a : The parameter that we are going to set
    ///@param[in] freq : The Frequency  associated with this conversion
    void SetFilterClockInSeconds( const int &freq, const double &conversionFactor) {
        filterClockTickToSeconds_.emplace(freq, conversionFactor);
    }

    ///Sets a flag that controls if we output the raw histograms to DAMM
    ///@param[in] a : The parameter that we are going to set
    void SetHasRawHistogramsDefined(const bool &a) { hasRawHistogramsDefined_ = a; }

    ///Sets output Filename from scan interface
    ///@param[in] a : The parameter that we are going to set
    void SetOutputFilename(const std::string &a) { outputFilename_ = a; }

    ///Sets the path that we are going to output all of the files to.
    ///@param[in] a : The parameter that we are going to set
    void SetOutputPath(const std::string &a) { outputPath_ = a; }

    ///Sets the rejection regions parsed from the config file
    ///@param[in] a : The rejection regions.
    void SetRejectionRegions(const std::vector<std::pair<unsigned int, unsigned int>> &a) { reject_ = a; }

    ///Sets the revision of the pixie modules that we had.
    ///@param[in] a : The parameter that we are going to set
    ///@TODO this will eventually be supersceded by information coming
    /// directly from the Map node.
    void SetRevision(const std::string &a) { revision_ = a; }

    ///Sets the speed of light in a Big VANDLE module.
    ///@param[in] a : The speed of light in units of cm/ns
    void SetVandleBigSpeedOfLight(const double &a) { vandleBigSpeedOfLight_ = a; }

    ///Sets the speed of light in a Medium VANDLE module.
    ///@param[in] a : The speed of light in units of cm/ns
    void SetVandleMediumSpeedOfLight(const double &a) { vandleMediumSpeedOfLight_ = a; }

    ///Sets the speed of light in a small VANDLE module.
    ///@param[in] a : The speed of light in units of cm/ns
    void SetVandleSmallSpeedOfLight(const double &a) { vandleSmallSpeedOfLight_ = a; }

    ///Sets the speed of light in a NEXT module.
    ///@param[in] a : The speed of light in units of cm/ns
    void SetNEXTSpeedOfLight(const double &a) { nextSpeedOfLight_ = a; }

private:
    ///Default Constructor 
    Globals(const std::string &file);

    ///The default constructor
    Globals(Globals const &);

    ///Copy constructor
    void operator=(Globals const &);

    ///The one and only one instance of the class.
    static Globals *instance_;

    ///A method that simply initializes all of the member varaiables to some
    /// default values. This will prevent too many errors down the line if
    /// they are not set properly due to invalid up configuration files.
    void InitializeMemberVariables(void);

    std::map<int, double> clockTickToSeconds_;                //!< map of frequencies and conversion factors
    std::map<int, double> adcClockTickToSeconds_;             //!< map of frequencies and conversion factors for Adc Ticks->Seconds
    std::map<int, double> filterClockTickToSeconds_;             //!< map of frequencies and conversion factors for Dsp Ticks->Seconds
    double adcClockInSeconds_;                                   //!< adc clock in second
    double clockInSeconds_;                                      //!< the ACQ clock in seconds
    std::string configFile_;                                     //!< The configuration file
    double eventLengthInSeconds_;                                //!< event width in seconds
    unsigned int eventLengthInTicks_;                            //!< the size of the events
    double filterClockInSeconds_;                                //!< filter clock in seconds
    bool hasRawHistogramsDefined_;                               //!< True if we are plotting Raw Histograms
    std::string outputFilename_;                                 //!<Output Filename
    std::string outputPath_;                                     //!< The path to additional configuration files
    std::string revision_;                                       //!< the pixie revision
    double sysClockFreqInHz_;                                    //!< frequency of the system clock
    std::vector<std::pair<unsigned int, unsigned int>> reject_;  ///< Rejection regions
    double vandleBigSpeedOfLight_;                               //!< speed of light in big VANDLE bars in cm/ns
    double vandleMediumSpeedOfLight_;                            //!< speed of light in medium VANDLE bars in cm/ns
    double vandleSmallSpeedOfLight_;                             //!< speed of light in small VANDLE bars in cm/ns
    double nextSpeedOfLight_;                                    //!< speed of light in NEXT module in cm/ns
};

#endif  // #ifdef _PAASS_GLOBALS_HPP_
