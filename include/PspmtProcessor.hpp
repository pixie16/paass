/** \file NaIProcessor.hpp
 * 
 * 
 */

#ifndef __PSPMTPROCESSOR_HPP_
#define __PSPMTPROCESSOR_HPP_

#include "EventProcessor.hpp"

namespace dammIds{
  namespace pspmt{
    
    // OFFSET = 700//    
    const int D_RAW1=0;
    const int D_RAW2=1;
    const int D_RAW3=2;
    const int D_RAW4=3;
    const int D_RAWD=4;
    const int D_SUM=5;
    const int DD_POS1_RAW=6;
    const int DD_POS2_RAW=7;
    const int DD_POS1=8;
    const int DD_POS2=9;
    
    const int D_ENERGY_TRACE1=10;
    const int D_ENERGY_TRACE2=11;
    const int D_ENERGY_TRACE3=12;
    const int D_ENERGY_TRACE4=13;
    const int D_ENERGY_TRACED=14;
    const int D_ENERGY_TRACESUM=15;
    const int DD_POS1_RAW_TRACE=16;
    const int DD_POS2_RAW_TRACE=17;
    const int DD_POS1_TRACE=18;
    const int DD_POS2_TRACE=19;
    
    const int D_QDC_TRACE1=20;
    const int D_QDC_TRACE2=21;
    const int D_QDC_TRACE3=22;
    const int D_QDC_TRACE4=23;
    const int D_QDC_TRACED=24;
 
    const int DD_ESLEW=30;
   
    const int D_TEMP0=80;
    const int D_TEMP1=81;
    const int D_TEMP2=82;
    const int D_TEMP3=83;
    const int D_TEMP4=84;
    const int D_TEMP5=85;
    
 
    
    const int DD_DOUBLE_TRACE=77;
    const int DD_SINGLE_TRACE=78;
    
  }
}

class PspmtProcessor : public EventProcessor
{
 public:
  PspmtProcessor(void);
  virtual void DeclarePlots(void);
  virtual bool Process(RawEvent &rEvent);
private:
  
  struct PspmtData {
    void Clear(void);
protected:
  } data;
};

#endif // __PSPMTPROCESSOR_HPP_
