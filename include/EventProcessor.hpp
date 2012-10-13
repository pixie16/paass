/*! \file EventProcessor.hpp
 * 
 * Generic event processor which particular processing 
 * of individual devices can be derived from
 */

#ifndef __EVENTPROCESSOR_HPP_
#define __EVENTPROCESSOR_HPP_

#include <map>
#include <set>
#include <string>

#include <sys/times.h>

#include "Plots.hpp"

// forward declarations
class DetectorDriver;
class DetectorSummary;
class RawEvent;

#ifdef useroot
class TTree;
#endif

class EventProcessor {
 private:
    // things associated with timing
    tms tmsBegin;
    double userTime;
    double systemTime;
    double clocksPerSecond;


 protected:
    // define the associated detector types and only initialize if present
    std::string name;
    std::set<std::string> associatedTypes;    
    bool initDone;
    bool didProcess;
    // map of associated detector summary
    std::map<std::string, const DetectorSummary *> sumMap;

    Plots histo;

    virtual void plot(int dammId, double val1, double val2 = -1, double val3 = -1, const char* name="h") {
        histo.Plot(dammId, val1, val2, val3, name);
    }
    virtual void DeclareHistogram1D(int dammId, int xSize, const char* title) {
        histo.DeclareHistogram1D(dammId, xSize, title);
    }
    virtual void DeclareHistogram2D(int dammId, int xSize, int ySize, const char* title) {
        histo.DeclareHistogram2D(dammId, xSize, ySize, title);
    }

 public:
    EventProcessor();
    EventProcessor(int offset, int range);
    virtual ~EventProcessor();

    // declare associated damm plots (called by drrsub_)
    virtual void DeclarePlots(void);
    virtual const std::set<std::string>& GetTypes(void) const {
      return associatedTypes; 
    }
    virtual bool DidProcess(void) const {
      return didProcess;
    }
    // return true on success
    virtual bool HasEvent(void) const;
    virtual bool Init(DetectorDriver &driver);
    virtual bool PreProcess(RawEvent &event);   
    virtual bool Process(RawEvent &event);   
    void EndProcess(void); // stop the process timer
    std::string GetName(void) const {
      return name;
    }
#ifdef useroot
    virtual bool AddBranch(TTree *tree);
    virtual void FillBranch(void);
#endif
};

#endif // __EVENTPROCESSOR_HPP_
