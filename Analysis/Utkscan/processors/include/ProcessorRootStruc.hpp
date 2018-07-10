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
    double sTime=0;
    unsigned long vMulti=0;
    unsigned long long ExtTimeStamp=0;
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


#endif //PAASS_PROCESSORSTRUC_HPP
