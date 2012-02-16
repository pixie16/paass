/** \file IonChamberProcessor.h
 *
 * Processor for ion chamber
 */

#ifndef __IONCHAMBERPROCESSOR_H_
#define __IONCHAMBERPROCESSOR_H_

#include <deque>

#include "EventProcessor.h"

class IonChamberProcessor : public EventProcessor 
{
 private:  
  static const size_t noDets = 6;
  static const size_t timesToKeep = 1000;
  static const double minTime;

  struct Data {
    double raw[noDets];
    double cal[noDets];
    int mult;

    void Clear(void);
  } data;

  double lastTime[noDets];
  std::deque<double> timeDiffs[noDets];
 public:
  IonChamberProcessor(); // no virtual c'tors
  virtual bool Process(RawEvent &event);
  virtual void DeclarePlots(void) const;
  // nice and simple raw derived class

#ifdef useroot
  bool AddBranch(TTree *tree);
  void FillBranch(void);
#endif // USEROOT  
};

namespace dammIds
{
  namespace ionChamber {
    // 1d spectra
    const int D_ENERGYSUM     = 2000;
    const int D_ENERGYTHREE_GROUPX = 2010; // + starting det
    const int D_ENERGYTWO_GROUPX   = 2020; // + starting det
    
    const int D_DTIME_DETX    = 2030; // + detector num
    const int D_RATE_DETX     = 2040; // + detector num
    // 2d spectra
    const int DD_ESUM__ENERGY_DETX   = 2050; // + detector num    
    const int DD_EBACK__ENERGY_DETX  = 2100; // + detector num
  };
};

#endif // __IONCHAMBERPROCSSEOR_H_
