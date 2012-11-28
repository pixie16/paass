/****\file TimingInformation.hpp
 *
 *File holding structures and 
 *   info for Timing Analysis
 */

#ifndef __TIMINGINFORMATION_HPP_
#define __TIMINGINFORMATION_HPP_

#include <map>
//#include "ChanEvent.hpp"

#ifdef useroot
#include "Rtypes.h"
#endif

// Forward declarations for hell of circular dependencies
// see Trace.hpp
class Trace;
// see ChanEvent.hpp
class ChanEvent;

class TimingInformation
{
 public:
    struct TimingCal {
	double lrtOffset;
	double r0;
	double tofOffset0;
	double tofOffset1;
	double xOffset;
	double z0;
	double zOffset;
    };
    
    struct TimingData 
    {
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
	double signalToNoise;
	double stdDevBaseline;
	double tqdc;
	double walk;
	double walkCorTime;
    };
    
#ifdef useroot
    struct DataRoot
    {
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

    struct BarData
    {
	BarData(const TimingData& Right, const TimingData& Left, 
		const TimingCal &cal, const std::string &type);
	bool BarEventCheck(const double &timeDiff, 
			   const std::string &type);
        double CalcFlightPath(double &timeDiff, const TimingCal &cal, 
			      const std::string &type);
	
        bool event;
	
	double flightPath;
	double lMaxVal;
	double qdc;
	double qdcPos;
	double rMaxVal;
	double theta;
	double timeAve;
	double timeDiff;
	double walkCorTimeDiff;
	
	std::map<unsigned int, double> timeOfFlight;
	std::map<unsigned int, double> corTimeOfFlight;
	std::map<unsigned int, double> energy;
    };

    //define types for the keys and maps
    typedef std::pair<unsigned int, std::string> IdentKey;
    typedef std::map<IdentKey, struct TimingData> TimingDataMap;
    typedef std::map<IdentKey, struct BarData> BarMap;
    typedef std::map<IdentKey, struct TimingCal> TimingCalMap;
    typedef std::map<unsigned int, double> TimeOfFlightMap;

    double CalcEnergy(const double &timeOfFlight, const double &z0);
    
    static double GetConstant(const std::string &value);
    static TimingCal GetTimingCal(const IdentKey &identity);
    static void ReadTimingCalibration(void);
    static void ReadTimingConstants(void);
    
 private:
    static const double qdcCompression = 4.0;

    static std::map<std::string, double> constantsMap;
    static TimingCalMap calibrationMap;
}; // class TimingInformation
#endif //__TIMINGINFORMATION_HPP_
