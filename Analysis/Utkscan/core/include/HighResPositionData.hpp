/*! \file HighResPositionData.hpp
 *  \brief Class to hold all of the information for high resolution position
 *  \author S. V. Paulauskas, T. T. King
 *  \date November 9, 2014
 */
#ifndef __HIGHRESPOSITIONDATA_HPP__
#define __HIGHRESPOSITIONDATA_HPP__

#include "ChanEvent.hpp"
#include "Constants.hpp"
#include "Globals.hpp"

//! Class for holding information for high resolution position.
class HighResPositionData {
public:
    /** Default constructor */
    HighResPositionData() {};

    /** Default destructor */
    virtual ~HighResPositionData() {};

    double GetTotIntegral() const {return T_sum;}

    ///@return high resolution position parallel to r^hat from Trace QDC
    double GetHighResZPos() const {
      return -1.0*(T_ft + T_fb - T_bt - T_bb) / T_sum ;
    }

    ///@return high resolution position perpendicular to r^(hat) (parallel to phi^hat) from Trace QDC
    double GetHighResYPos() const {
      return (T_ft + T_bt - T_fb - T_bb) / T_sum ;      
    }

    ///@return high resolution position parallel to r^hat from Pixie Filter
    double GetFilterZPos() const {
      return -1.0*(F_ft + F_fb - F_bt - F_bb) / F_sum ;
    }

    ///@return high resolution position perpendicular to r^(hat) (parallel to phi^hat) from Pixie Filter
    double GetFilterYPos() const {
      return (F_ft + F_bt - F_fb - F_bb) / F_sum ;      
    }

    ///@return high resolution position parallel to r^hat from Pixie QDC
    double GetPixieQdcZPos() const {
      return -1.0*(Q_ft + Q_fb - Q_bt - Q_bb) / Q_sum ;
    }

    ///@return high resolution position perpendicular to r^(hat) (parallel to phi^hat) from Pixie QDC
    double GetPixieQdcYPos() const {
      return (Q_ft + Q_bt - Q_fb - Q_bb) / Q_sum ;      
    }

    double GetFTtqdc() const {
     return T_ft;
    }

    double GetBTtqdc() const {
      return T_bt;
    }

    double GetFBtqdc() const {
      return T_fb;
    }

    double GetBBtqdc() const {
      return T_bb;
    }

     double GetFTqdc() const {
     return Q_ft;
    }

    double GetBTqdc() const {
      return Q_bt;
    }

    double GetFBqdc() const {
      return Q_fb;
    }

    double GetBBqdc() const {
      return Q_bb;
    }

    bool GetSaturation() const{
         return kSaturated;
}

    ///@return True if the trace was successfully analyzed
    bool GetIsValid() const {
      //  if ( T_ft > 0 && T_fb > 0 && T_bt > 0 && T_bb > 0) return (true);
        if ( Q_ft > 0 && Q_fb > 0 && Q_bt > 0 && Q_bb > 0 && !kSaturated) return (true);
        else return (false);
    }

    void AddChanEvent(ChanEvent *chIn){
        if(chIn->GetChanID().HasTag("FT") ){
          T_ft = chIn->GetTrace().GetQdc(); F_ft = chIn->GetEnergy(); if(chIn->GetQdc().size()==8) Q_ft = chIn->GetQdc().at(0)-chIn->GetQdc().at(1);
        }
        if(chIn->GetChanID().HasTag("FB") ){
          T_fb = chIn->GetTrace().GetQdc(); F_fb = chIn->GetEnergy(); if(chIn->GetQdc().size()==8) Q_fb = chIn->GetQdc().at(0)-chIn->GetQdc().at(1);
        }
        if(chIn->GetChanID().HasTag("BB") ){
          T_bb = chIn->GetTrace().GetQdc(); F_bb = chIn->GetEnergy(); if(chIn->GetQdc().size()==8) Q_bb = chIn->GetQdc().at(0)-chIn->GetQdc().at(1);
        }
        if(chIn->GetChanID().HasTag("BT") ){
          T_bt = chIn->GetTrace().GetQdc(); F_bt = chIn->GetEnergy(); if(chIn->GetQdc().size()==8) Q_bt = chIn->GetQdc().at(0)-chIn->GetQdc().at(1);
        }

      if (!kSaturated && chIn->IsSaturated()) kSaturated=true;
    
      T_sum = T_ft + T_fb + T_bb + T_bt;
      F_sum = F_ft + F_fb + F_bb + F_bt;
      Q_sum = Q_ft + Q_fb + Q_bb + Q_bt;
      //Q_sum = 9999;
    }

    void ClearEvent(){
      T_ft = 0.0;
      T_fb = 0.0;
      T_bt = 0.0;
      T_bb = 0.0;
      T_sum = 0.0;
      
      F_ft = 0.0;
      F_fb = 0.0;
      F_bt = 0.0;
      F_bb = 0.0;
      F_sum = 0.0;
      
      Q_ft = 0.0;
      Q_fb = 0.0;
      Q_bt = 0.0;
      Q_bb = 0.0;
      Q_sum = 0.0;
    }

private:
    double T_ft;
    double T_fb;
    double T_bt;
    double T_bb;
    double T_sum;

    double F_ft;
    double F_fb;
    double F_bt;
    double F_bb;
    double F_sum;

    //unsigned int Q_ft;
    //unsigned int Q_fb;
    //unsigned int Q_bt;
    //unsigned int Q_bb;
    //unsigned int Q_sum;

    double Q_ft;
    double Q_fb;
    double Q_bt;
    double Q_bb;
    double Q_sum;
    
    bool kSaturated=false;

};

/** Defines a map to hold timing data for a channel. */
//typedef std::map<TimingDefs::TimingIdentifier, HighResPositionData> PositionMap;
#endif // __HIGHRESPOSITIONDATA_HPP__
