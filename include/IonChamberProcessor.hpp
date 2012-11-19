/** \file IonChamberProcessor.hpp
 * \brief Processor for ion chamber
 */

#ifndef __IONCHAMBERPROCESSOR_HPP_
#define __IONCHAMBERPROCESSOR_HPP_

#include <deque>

#include "EventProcessor.hpp"

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
  virtual void DeclarePlots(void);
  // nice and simple raw derived class

#ifdef useroot
  bool AddBranch(TTree *tree);
  void FillBranch(void);
#endif // USEROOT  
};

#endif // __IONCHAMBERPROCSSEOR_HPP_
