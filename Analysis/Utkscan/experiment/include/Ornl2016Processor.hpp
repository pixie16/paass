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
    Ornl2016Processor();

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

// Bools for controling output as set in the cfg
    bool debugging = false;
    bool Pvandle = false;
    std::string Vout;
    bool VoutRoot = false;
    bool VoutDamm = false;
    std::pair<double, std::string> SupBetaWin;

    std::set<std::string> hagIgnore; // Timeout corner for bad detectors
    std::set<std::string> naiIgnore; // Timeout corner for bad detectors
    std::set<std::string> geIgnore; // Timeout corner for bad detectors

    bool hasLRbeta;
    double betaSubTime;

    TFile *rootFName_;
    TFile *rootFName2_;
    TFile *rootFName3_;
    TFile *rootFName4_;
    TFile *rootFName5_;

    TTree *LaBr_tree;
    TTree *NaI_tree;
    TTree *HPGe_tree;
    TTree *addBack_tree;

    TBranch *singBranch;
    TBranch *gProcBranch;
    TBranch *lProcBranch;
    TBranch *nProcBranch;


//aux branches
    double aux_LaBrEn=0;
    double aux_LaBrNum=0;
    double aux_LaBrTime=0;
    bool aux_LaBrHasLRBeta=false;
    double aux_LaBrTdiff = 0;
    double aux_LaBrMulti=0;

    double aux_NaIEn=0;
    double aux_NaINum=0;
    double aux_NaITime=0;
    bool aux_NaIHasLRBeta=false;
    double aux_NaITdiff = 0;
    double aux_NaIMulti=0;

    double aux_GeEn=0;
    double aux_GeNum=0;
    double aux_GeTime=0;
    bool aux_GeHasLRBeta=false;
    double aux_GeTdiff = 0;
    double aux_GeMulti=0;

    double aux_BetaEn=0;
    double aux_BetaTime=0;
    double aux_BetaMulti=0;

    int aux_cycle=-1;
    double aux_cycleSTime=0;
    double aux_eventNum=-1;

    // This is a vector of double pairs
    // The data is stored as <<Time,Energy>,...>
    std::vector<std::pair<double,double>> BetaList;

    // gamma AddBack Structure
    struct PROSS {
        double AbE;
        double AbEvtNum;
        double Multi;
//        double SymX;
//        double SymY;

    } Gpro, Lpro, Npro;

    TH2D *qdcVtof_;
    TH2D *tofVGe_;
    TH2D *tofVNai_;
    TH2D *tofVLabr_;


    //functions for root preocessing
    void rootArrayreset(double arrayName[], int arraySize);
    void rootGstrutInit2(PROSS &strutName);
    void rootAuxRoot(std::string &type);

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


    bool to_bool(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        std::istringstream is(str);
        bool b;
        is >> std::boolalpha >> b;
        return b;
    }

};

#endif
