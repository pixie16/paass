/** \file TimingInformation.hpp
 * \brief File holding structures and info for Timing Analysis
 */
#ifndef __TIMINGINFORMATION_HPP_
#define __TIMINGINFORMATION_HPP_
#include <map>

#ifdef useroot
#include "Rtypes.h"
#endif

#include "TimingCalibrator.hpp"
//#include "ChanEvent.hpp"
//#include "Trace.hpp"

// see Trace.hpp
class Trace;
// see ChanEvent.hpp
class ChanEvent;

class TimingInformation {
public:
    struct TimingData {
        TimingData(void);
        TimingData(ChanEvent *chan);
        const Trace &trace;

        bool dataValid;

        double aveBaseline;
        double discrimination;
        double highResTime;
        double maxpos;
        double maxval;
        double phase;
        double snr;
        double stdDevBaseline;
        double tqdc;
        double walk;
        double walkCorTime;

        int numAboveThresh;
    };

    struct BarData {
        BarData(const TimingData& Right, const TimingData& Left,
                const TimingCalibration &cal, const std::string &type);
        bool BarEventCheck(const double &timeDiff, const std::string &type);
        double CalcFlightPath(double &timeDiff,
                              const TimingCalibration &cal,
                              const std::string &type);
        bool event;

        double flightPath;
        double lMaxVal;
        double lqdc;
        double lTime;
        double qdc;
        double qdcPos;
        double rMaxVal;
        double rqdc;
        double rTime;
        double theta;
        double timeAve;
        double timeDiff;
        double walkCorTimeDiff;
        double walkCorTimeAve;

        std::map<unsigned int, double> timeOfFlight;
        std::map<unsigned int, double> corTimeOfFlight;
        std::map<unsigned int, double> energy;
    };

    typedef std::map<Vandle::BarIdentifier,
                     struct TimingData> TimingDataMap;
    typedef std::map<Vandle::BarIdentifier,
                     struct BarData> BarMap;
    typedef std::map<unsigned int, double> TimeOfFlightMap;

    double CalcEnergy(const double &timeOfFlight, const double &z0);

#ifdef useroot
    struct DataRoot {
        static const size_t maxMultiplicity = 10;

        DataRoot(void);

        UInt_t   multiplicity;
        UInt_t   dummy;

        Double_t aveBaseline[maxMultiplicity];
        Double_t discrimination[maxMultiplicity];
        Double_t highResTime[maxMultiplicity];
        Double_t maxpos[maxMultiplicity];
        Double_t maxval[maxMultiplicity];
        Double_t phase[maxMultiplicity];
        Double_t stdDevBaseline[maxMultiplicity];
        Double_t tqdc[maxMultiplicity];
        UInt_t   location[maxMultiplicity];
    };
#endif
};
#endif
