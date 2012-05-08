/** \file VandleProcessor.h
 * 
 * Class for handling Vandle Bars
 */

#ifndef __VANDLEPROCESSOR_H_
#define __VANDLEPROCESSOR_H_

#include <map>
#include <vector>

#include "EventProcessor.h"
#include "Trace.h"

#ifdef useroot
class TTree;
#endif

class VandleProcessor : public EventProcessor
{
 public:
    VandleProcessor(); // no virtual c'tors
    virtual void DeclarePlots(void) const;
    virtual bool Process(RawEvent &event);
    virtual bool Init(DetectorDriver &);
    
#ifdef useroot
    virtual bool AddBranch(TTree *tree);
    virtual void FillBranch(void);
#endif
    
 private:
    struct VandleCal {
	double r0;
	double z0;
	double xOffset;
	double zOffset;
	double tofOffset0;
	double tofOffset1;
    };

    struct VandleData 
    {
	VandleData(string type ="");
	VandleData(ChanEvent *chan);
	
	string detSubtype;
	double tqdc;
	double maxval;
	double maxpos;
	double phase;
	double stdDevBaseline;
	double aveBaseline;
	double highResTime;
	
	Trace trace;
    };
    
    struct VandleBarData
    {
	VandleBarData(const VandleData& Right, const VandleData& Left, const VandleCal &Cal);
	double BendBar(double &timeDiff, const VandleCal& Cal, const string &barType);
	
	double barQDC;
	string barType;
	double timeAve;
	double timeDiff;
	double corRadius;
    };

    struct VandleDataRoot
    {
	VandleDataRoot();
	
	double maxRight;
	double maxLeft;
	double qdcRight;
	double qdcLeft;
	double timeDiff;
	double TOF;
    } vandledataroot;

    virtual double InverseVelocity (const double &corTOF, const double &z0);
    virtual double CorrectTOF(double &TOF, double &corRadius, const double &z0, const string &barType);
    
    virtual void ImproperDetOrder(void);
    virtual void AnalyzeData(void);
    virtual void ReadVandleCal(void);
    
    virtual bool RetrieveData(RawEvent &event);
    virtual bool BarEventCheck(const double &timeDiff, const string &barType);
    virtual bool GoodDataCheck(const VandleData& DataCheck);
    
    std::map<int, struct VandleData> vandleEndsMap;
    std::map<int, struct VandleBarData> vandleBarMap;
    std::map<int, struct VandleData> scintMap;
    std::map<int, struct VandleCal> vandleCalMap;
    
    int counter;
    
    static const double C_BAR_S =   6.32911;                   // speed of light in 60cm bar cm/ns
    static const double C_BAR_B =   7.61499;                   // speed of light in 200cm bar cm/ns
    static const double N_RAD_S =   32;                        // neutron radius for the center of small bar in cm
    static const double N_RAD_B =   150;                       // neutron radius for the center of large bar in cm
    static const double LENGTH_SMALL_CM = 60;                           // length of small vandle bar in cm
    static const double LENGTH_BIG_CM   = 200;                          // length of large vandle bar in cm
    static const double NUMBARS= 1;                            // number of VANDLE bars in the setup
    static const double LENGTH_SMALL_NS;
    static const double LENGTH_BIG_NS;
    
}; //Class VandleProcessor
#endif // __VANDLEPROCESSOR_H_
