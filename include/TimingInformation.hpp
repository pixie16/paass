/** \file TimingInformation.hpp
 * \brief Structures and info for Timing Analysis
 * \author S. V. Paulauskas
 * \date May 9, 2011
 */
#ifndef __TIMINGINFORMATION_HPP_
#define __TIMINGINFORMATION_HPP_
#include <map>

#ifdef useroot
#include "Rtypes.h"
#endif

#include "TimingCalibrator.hpp"

// see Trace.hpp
class Trace;
// see ChanEvent.hpp
class ChanEvent;

//! The class holding all the timing structures
class TimingInformation {
public:

    //! Structure to contain all of the timing and trace related information
    struct TimingData {
        /** Default constructor */
        TimingData(void);

        /** Constructor using the channel event
        * \param [in] chan : the channel event for grabbing values from */
        TimingData(ChanEvent *chan);

        const Trace &trace;//!< the trace for the channel

        bool dataValid;//!< flag to set if the data is valid

        double aveBaseline;//!< the average baseline of the trace
        double discrimination;//!< the n-gamma discrimination value (if applicable)
        double highResTime;//!< the high resolution time
        double maxpos;//!< position of the maximum value in the trace
        double maxval;//!< maximum value in the trace
        double phase;//!< phase for the channel
        double snr;//!< The Signal to noise ratio
        double stdDevBaseline;//!< Standard deviation of the baseline
        double tqdc;//!< QDC from the trace
        double walk;//!< The value of the walk from the HRT algorithm
        double walkCorTime;//!< Walk corrected time for the channel

        int numAboveThresh; //!< the number of bins above a threshold
    };

    //! Defines a structure to hold all of the various bits of bar data
    struct BarData {
        /** \brief The constructor for the structure
        * \param [in] Right : The right side of the bar
        * \param [in] Left : The left side of the bar
        * \param [in] cal : The timing calibration for the bar
        * \param [in] type : The type of bar that we have
        */
        BarData(const TimingData& Right, const TimingData& Left,
                const TimingCalibration &cal, const std::string &type);

        /** \brief Checks to see if the event could physically be inside the bar
        * \param [in] timeDiff : The time difference inside the bar
        * \param [in] type : the type of bar to check for
        * \return True if the even was inside the bar */
        bool BarEventCheck(const double &timeDiff, const std::string &type);

        /** \brief Calculates the flight path of the particle
        * \param [in] timeDiff : The time difference inside the bar
        * \param [in] cal : The timing calibration for the bar
        * \param [in] type : the type of bar
        * \return The flight path of the particle
        */
        double CalcFlightPath(double &timeDiff,
                              const TimingCalibration &cal,
                              const std::string &type);

        bool event; //!< true if there was an event in the bar

        double flightPath; //!< flight path of the particle
        double lMaxVal;//!< Max value of the left end of the bar
        double lqdc;//!< QDC of the left side of the bar
        double lTime;//!< Arrival time of left side of the bar
        double qdc;//!< QDC for the bar
        double qdcPos;//!< Position derived from the QDC
        double rMaxVal;//!< Max value for the right side of the bar
        double rqdc;//!< QDC for the right side of the bar
        double rTime;//!< Arrival time of the right side of the bar
        double theta;//!< the angle of the particle w.r.t. the source
        double timeAve;//!< The average arrival time of the left and right ends
        double timeDiff;//!< The time difference between the two ends
        double walkCorTimeDiff;//!< The walk corrected time difference
        double walkCorTimeAve;//!< The walk corrected time average

        std::map<unsigned int, double> timeOfFlight; //!< map of the ToFs
        std::map<unsigned int, double> corTimeOfFlight;//!< map of the Cor ToFs
        std::map<unsigned int, double> energy;//!< map of the energies
    };

    /** Defines a map to hold timing data for a channel. */
    typedef std::map<Vandle::BarIdentifier,
                     struct TimingData> TimingDataMap;
    /** Defines a map to hold VANDLE Bars
    * (or any other kind of 2 signal detector) */
    typedef std::map<Vandle::BarIdentifier,
                     struct BarData> BarMap;
    /** Defines a map to hold time-of-flight variables */
    typedef std::map<unsigned int, double> TimeOfFlightMap;

    /** Calculate the energy from the time of flight, using a correction
    * \param [in] timeOfFlight : The time of flight to use for the calculation
    * \param [in] z0 : The perpendicular distance between the bar and the source
    * \return The particle energy
    */
    double CalcEnergy(const double &timeOfFlight, const double &z0);

#ifdef useroot
    ///Data structure for outputting to ROOT (deprecated)
    struct DataRoot {
        static const size_t maxMultiplicity = 10; //!< maximum multiplicity

        DataRoot(void); //!< Constructor for the class

        UInt_t   multiplicity; //!< multiplicity of the vandle event
        UInt_t   dummy;//!< a filler variable

        Double_t aveBaseline[maxMultiplicity]; //!< array for baselines
        Double_t discrimination[maxMultiplicity];//!< array for discrimination
        Double_t highResTime[maxMultiplicity];//!< array for HRT
        Double_t maxpos[maxMultiplicity];//!< array for position of max value
        Double_t maxval[maxMultiplicity];//!< array for max values
        Double_t phase[maxMultiplicity];//!< array for phases
        Double_t stdDevBaseline[maxMultiplicity];//!< array for Standard Deviation of Baseline
        Double_t tqdc[maxMultiplicity];//!< array for QDC from trace
        UInt_t   location[maxMultiplicity];//!< array for locations
    };
#endif
};
#endif
