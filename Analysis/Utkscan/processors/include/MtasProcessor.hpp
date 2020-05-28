/*
*
*   MTAS processor
*
*   Written by DVM.
*   Last updated: --/--/--
*
*   Will update description later
*/

#ifndef _MTASPROCESSOR_HPP_
#define _MTASPROCESSOR_HPP_

#include <string>
#include <vector>
#include <map>

#include "RawEvent.hpp"
#include "EventProcessor.hpp"
#include "PaassRootStruct.hpp"
#include "BarDetector.hpp"
#include "HighResTimingData.hpp"


using namespace std;


// forward declarations
class DetectorSummary;
class RawEvent;
class ChanEvent;

// Class to process MTAS related events
class MtasProcessor : public EventProcessor {
public:
    // Default Constructor
    MtasProcessor();

    // Default Destructor
    ~MtasProcessor() {};

    // Declare the plots used in the analysis
    virtual void DeclarePlots(); // (void) const in pixie_slim

    // Preprocess the MTAS data
    //@param [in] event : the event to preprocess
    //@return true if successful
    virtual bool PreProcess(RawEvent &event);

    // Process the event for MTAS stuff
    //@param [in] event : the event to process
    //@return Returns true if the processing was successful
    virtual bool Process(RawEvent &event);
    double siliconThresh = 10;
    bool beta_ev;
    double MTAS_time,siliTime,T_diff_MTAS_Sili,max_sili_en,max_raw_sili_en;
    const ChanEvent* silimax;

private:
    DetectorSummary *mtasSummary;
    DetectorSummary *siliSummary;
    DetectorSummary *logiSummary;
    DetectorSummary *thermSummary;
    DetectorSummary *refmodSummary;

    static bool isTapeMoveOn;
    static bool isMeasureOn;
    static bool isBkgOn;
    static bool isLaserOn;
    static bool isIrradOn;
    static unsigned cycleNumber;
    static double measureOnTime;
    double firstTime;
    BarMap bars_;
    // vector<ChanEvent *> mtasList;
    // ChanEvent *maxEvent_;
    map<string,int> silimap = {{"a",9},{"b",10},{"c",11},{"d",12},{"e",13},{"f",14},{"g",15},{"h",1},{"i",2},{"j",3},{"k",4},{"l",5},{"m",6},{"n",7}};
    struct MtasData // you have to convert it
    {
        MtasData(ChanEvent *chan); //single argument constructor

        string detSubtype;
        double energy;
        double calEnergy;
        double time;
        double location;
        string detGroup;
    };

    struct SiliData
    {
        SiliData(string type = "");
        SiliData(ChanEvent *chan);

        string detSubtype;
        double energy;
        double calEnergy;
        double time;
        double location;
    };

    struct LogiData
    {
        LogiData(ChanEvent *chan);

        string detSubtype;
        double energy;
        double calEnergy;
        double time;
        double location;
    };

    processor_struct::MTAS MTASstruct; // MTAS root Struct

 };



#endif // _MTASPROCESSOR_HPP_
