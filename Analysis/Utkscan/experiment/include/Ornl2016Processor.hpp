/** \file Ornl2016Processor.hpp
 * \brief A class to process data from the OLTF experiments in Feb/March 2016 using
 * VANDLE at ORNl
 *
 *\author S. V. Paulauskas
 *\date Feb, 2016
 */

#ifndef __Ornl2016PROCESSOR_HPP_
#define __Ornl2016PROCESSOR_HPP_

#include <fstream>


#include "EventProcessor.hpp"


#ifdef useroot

#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TH1D.h>
#include <TH2D.h>

#endif

class ScintAddBack {
public:
    /** Default constructor setting things to zero */
    ScintAddBack() {
        energy = time = multiplicity = 0;
    }

    /** Default constructor setting default values
     * \param [in] ienergy : the initial energy
     * \param [in] itime : the initial time
     * \param [in] imultiplicity : multiplicity of the event */
    ScintAddBack(double ienergy, double itime, double imultiplicity) {
        energy = ienergy;
        time = itime;
        multiplicity = imultiplicity;
    }

    double energy;//!< Energy of the addback event
    double time;//!< time of the addback event
    double multiplicity;//!< multiplicity of the event
};

/// Class to process VANDLE analysis for Ornl2016 campaign  at the OLTF
class Ornl2016Processor : public EventProcessor {
public:
    /** Default Constructor */
    Ornl2016Processor(double gamma_threshold_L, double sub_event_L, double gamma_threshold_N, double sub_event_N,
                      double gamma_threshold_G, double sub_event_G);

    /** Default Destructor */
    ~Ornl2016Processor();

    /** Declare the plots used in the analysis */
    virtual void DeclarePlots(void);


    /** Preprocess the Ornl2016 data
     * \param [in] event : the event to preprocess
     * \return true if successful */
    virtual bool PreProcess(RawEvent &event);

    /** Process the event
    * \param [in] event : the event to process
    * \return Returns true if the processing was successful */
    virtual bool Process(RawEvent &event);

    /** Returns the events that were added to the vaious  addback_ vectors */
    std::vector<ScintAddBack> GetLaddBackEvent(void) { return (LaddBack_); }

    std::vector<ScintAddBack> GetNaddBackEvent(void) { return (NaddBack_); }

    std::vector<ScintAddBack> GetGaddBackEvent(void) { return (GaddBack_); }


private:

    TTree *Taux;
    TTree *Tvan;
    TTree *Wave;

    TBranch *singBranch;
    TBranch *gProcBranch;
    TBranch *lProcBranch;
    TBranch *nProcBranch;
    TBranch *mVanBranch;
    TBranch *BwaveBranch;
    TBranch *VwaveBranch;

    struct BWave{
        double Ltrace[131];
        double Rtrace[131];
        double Lbaseline;
        double Rbaseline;
        double LmaxLoc;
        double RmaxLoc;
        double Lamp;
        double Ramp;
        double BarQdc;
        double Lsnr;
        double Rsnr;
        double Lqdc;
        double Rqdc;
        double Tdiff;
        double Lphase;
        double Rphase;
    } Bwave;

    struct VWave{
        double Ltrace[131];
        double Rtrace[131];
        double Lbaseline;
        double Rbaseline;
        double LmaxLoc;
        double RmaxLoc;
        double Lamp;
        double Ramp;
        double BarQdc;
        double TOF;
        double Lsnr;
        double Rsnr;
        int VbarNum;
    } Vwave;

    struct RAY {
        double LaBr[16];
        double NaI[10];
        double Ge[4];
        double beta;
        double eventNum;
        int cycle;
        int gMulti;
        int nMulti;
        int lMulti;
        int bMulti;
    } sing;

    struct PROSS {
        double AbE;
        double AbEvtNum;
        double Multi;
//        double SymX;
//        double SymY;

    } Gpro, Lpro, Npro;

    struct NBAR {
        double LaBr[16];
        double NaI[10];
        double Ge[4];
        double tof;
        double cortof;
        double qdc ;
        double betaEn;
        double snrl;
        double snrr;
        double Qpos;
        double tDiff;
        int barid;

    } mVan;

    TFile *rootFName_;

    TFile *rootFName2_;
    TH2D *qdcVtof_;
    TH2D *tofVGe_;
    TH2D *tofVNai_;
    TH2D *tofVLabr_;

    TFile *rootFName3_;

    //functions for root preocessing
    void rootArrayreset(double arrayName[], int arraySize);

    void rootGstrutInit(RAY &strutName);

    void rootGstrutInit2(PROSS &strutName);

    void rootNstrutInit(NBAR &strutName);

    //void rootBWaveInit(BWave &strutName);

    void rootVWaveInit(VWave &strutName);

    //thresholds and windows for gamma addback for LaBr3 (L*) and NaI (N*)
    double LgammaThreshold_;
    double LsubEventWindow_;
    double NgammaThreshold_;
    double NsubEventWindow_;
    double GgammaThreshold_;
    double GsubEventWindow_;


    std::vector<ScintAddBack> LaddBack_;
    std::vector<ScintAddBack> NaddBack_;
    std::vector<ScintAddBack> GaddBack_;


};

#endif


