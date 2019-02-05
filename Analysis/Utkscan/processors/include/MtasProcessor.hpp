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

#include "../../core/include/RawEvent.hpp"
#include "../include/EventProcessor.hpp"


using std::vector;
using std::string;


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

    struct MtasData // you have to convert it
    {
        MtasData(ChanEvent *chan); //single argument constructor

        string detSubtype;
        double energy;
        double calEnergy;
        double time;
        double location;
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
