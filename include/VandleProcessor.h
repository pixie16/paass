/** \file VandleProcessor.h
 * 
 * Class for handling Vandle Bars
 */

#ifndef __VANDLEPROCESSOR_H_
#define __VANDLEPROCESSOR_H_

#include "EventProcessor.h"
#include <map>
#include <vector>

#ifdef useroot
class TTree;
#endif

class VandleProcessor : public EventProcessor
{
 public:
    VandleProcessor(); // no virtual c'tors
    virtual void DeclarePlots(void) const;
    virtual bool Process(RawEvent &event);
    
#ifdef useroot
    virtual bool AddBranch(TTree *tree);
    virtual void FillBranch(void);
#endif
    
 private:
    struct VandleData 
    {
	VandleData(string type ="");
	VandleData(ChanEvent *chan);
	
	string detSubtype;
	double tqdc;
	double max;
	double phase;
	double stdDevBaseline;
	double aveBaseline;
	double highResTime;
	
	bool goodDataCheck;

	vector<int> trace;
    };
    
    struct VandleBarData
    {
	VandleBarData(const VandleData& Right, const VandleData& Left, const double &distance);
	double BendBar(double &timeDiff, const double &distance, const string &barType);
	
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

    virtual double InverseVelocity (const double &corTOF, const double &distance);
    virtual double CorrectTOF(double &TOF, double &corRadius, double &corBarOffset, const double &distance, const string &barType);
    virtual void ImproperDetOrder(void);
    virtual void AnalyzeData(void);
    virtual bool RetrieveData(RawEvent &event);
    virtual bool BarEventCheck(const double &timeDiff, const string &barType);
    virtual bool GoodDataCheck(const VandleData& DataCheck);
        
    map<int, struct VandleData> vandleEndsMap;
    map<int, struct VandleBarData> vandleBarMap;
    map<int, struct VandleData> scintMap;
    
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
