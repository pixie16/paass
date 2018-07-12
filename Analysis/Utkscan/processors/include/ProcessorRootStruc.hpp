/**Created by T.T. King 06/22/2018
 * */
#ifndef PAASS_PROCESSORSTRUC_HPP
#define PAASS_PROCESSORSTRUC_HPP

struct VANDLES {
    double tof=0;
    double corTof=0;
    double qdcPos=0;
    double qdc=0;
    int barNum=0;
    std::string barType="";
    double tdiff=0;
    double sTime=0; //start detector time
    unsigned int sNum =0; //start detector number
    int vMulti=0;
} ;

struct GAMMASCINT{
    bool HasLowResBeta = false;
    bool HasTrigBeta =false;
    double Energy = -999;
    double RawEnergy = -999;
    int DetNum = -999;
    double Time = -999;
    double BetaGammaTDiff = -999;
    double BetaEnergy = -999;
    double BetaMulti = -999;
    double BetaTime = -999;
    double EvtNum = -999;
    double BunchNum = -999;
    double LastBunchTime = -999;

    int NumGroup = -1; // group numbers (0-3) for small hag, 0-2 for nai.
    // REQUIRES a Type condition to separate the types
    std::string Type = "";

    int NumType = -1; //order in decreasing mass, 0 = nai, 1 = big hag, 2= small hag
    std::string Group = "";
} ;

struct CLOVERS{
    double Time = -999;
    double LastCycleTime = -999;
    double Energy = -999;
    double RawEnergy =-999;
    int DetNum = -999;
    int CloverNum = -999;
    bool HasLowResBeta = false;

};

struct PSPMT { 
  ///Contains both low and high gain PSPMT information
  double xa_l = -999; 
  double xb_l = -999; 
  double ya_l = -999; 
  double yb_l = -999; 
  double xa_h = -999; 
  double xb_h = -999; 
  double ya_h = -999; 
  double yb_h = -999; 
  double dy_l = -999;
  double dy_h = -999;
  double dyL_time = -999;
  double dyH_time = -999;
  double xposL =-999;
  double yposL =-999;
  double xposH =-999;
  double yposH =-999;
  int anodeLmulti = -999;
  int anodeHmulti = -999;
  int dyLmulti = -999;
  int dyHmulti = -999;
};


#endif //PAASS_PROCESSORSTRUC_HPP
