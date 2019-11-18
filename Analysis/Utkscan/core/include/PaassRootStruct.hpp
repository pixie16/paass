/**Created by T.T. King 06/22/2018
 * */
/** PixTreeEvent class added by R. Yokoyama 08/31/2018 **/
#ifndef PAASS_PAASSSTRUC_HPP
#define PAASS_PAASSSTRUC_HPP

#include <TObject.h>
#include <TString.h>

namespace processor_struct {

struct CLOVERS {
    double energy = -999;
    double rawEnergy = -999;
    double time = -999;
    int detNum = -999;
    int cloverNum = -999;
};

static const CLOVERS CLOVERS_DEFAULT_STRUCT;

struct DOUBLEBETA {
    int detNum = -999;
    double energy = -999;
    double rawEnergy = -999;
    double timeAvg = -999;
    double timeDiff = -999;
    double timeL = -999;
    double timeR = -999;
    double barQdc = -999;
    double tMaxValL = -999;
    double tMaxValR = -999;
    bool isLowResBeta = false;
    bool isHighResBeta = false;
};
static const DOUBLEBETA DOUBLEBETA_DEFAULT_STRUCT;

struct DSSD { 
    int x = -999;
    int y = -999;
    double Energy_front = -999;
    double Energy_back = -999;
    double Tof = -999;
    int QDC1 = - 9999;
    int QDC2 = -9999;
    double TMCP1 = -999;
    double TMCP2 = - 999;
    double Veto = -999;
    double Time = -999;
    double Side = -999;
    int NSide = -1;
    std::vector<unsigned int> Trace_Front;
    std::vector<unsigned int> Trace_Back;
    std::vector<unsigned int> Trace_TOF1;
    std::vector<unsigned int> Trace_TOF2;
    std::vector<unsigned int> Trace_Side;
    std::vector<unsigned int> Trace_Veto;

    bool pile_up = false;
};
static const DSSD DSSD_DEFAULT_STRUCT;

struct GAMMASCINT {
    double energy = -999;
    double rawEnergy = -999;
    double qdc = -999;
    bool isDynodeOut = false;
    int detNum = -999;
    double time = -999;
    TString group = "";
    TString subtype = "";
};
static const GAMMASCINT GAMMASCINT_DEFAULT_STRUCT;

struct LOGIC {
    bool tapeCycleStatus = false;
    bool beamStatus = false;
    bool tapeMoving = false;

    double lastTapeCycleStartTime = -999;
    double lastBeamOnTime = -999;
    double lastBeamOffTime = -999;
    double lastTapeMoveStartTime = -999;
    double lastProtonPulseTime = -999;
    double lastSuperCycleTime = -999;

    int cycleNum = -999;
};
static const LOGIC LOGIC_DEFAULT_STRUCT;

struct PSPMT {
    double energy = -999;
    double time = -999;
    TString subtype = "";
    TString tag = "";
    double traceMaxVal = -999;
    int traceMaxPos = -999;
    double preBaseAvg = -999;
    double postBaseAvg = -999;
    bool invalidTrace = false;
};
static const PSPMT PSPMT_DEFAULT_STRUCT;

struct ROOTDEV {
    double energy = -999;
    double rawEnergy = -999;
    double timeSansCfd = -999;
    double time = -999;
    int detNum = -999;   //the instance number of RD in the xml Map
    int modNum = -999;   // the physical module number
    int chanNum = -999;  // the physical channel number
    TString subtype = "";
    TString group = "";
    bool pileup = false;                   //Did pixie detect pileup in the event
    bool saturation = false;               //Did the trace go out of the ADC range
    std::vector<unsigned int> trace = {};  //The trace if present
    double baseline = -999;
    double stdBaseline = -999;
    double phase = -999;
    double tqdc = -999;                      //QDC from trace (requires the Waveform Analyzer)
    int maxPos = -999;                       //Max location in the trace  (requires the Waveform Analyzer)
    double maxVal = -999;                    //Max value in the trace (requires the Waveform Analyzer)
    double extMaxVal = -999;                 // Extrapolated Max value in the trace (requires the Waveform Analyzer)
    double highResTime = -999;               //High Resolution Time derived from the trace fitting (requires the Waveform and Fitting Analyzer)
    std::vector<unsigned int> qdcSums = {};  //output the onboard qdc sums if present
    bool hasValidTimingAnalysis = false;
    bool hasValidWaveformAnalysis = false;
};
static const ROOTDEV ROOTDEV_DEFAULT_STRUCT;

struct SINGLEBETA {
    int detNum = -999;
    double energy = -999;
    double rawEnergy = -999;
    double time = - 999;
    double qdc = -999;
    double tMaxVal = -999;
    bool isLowResBeta = false;
    bool isHighResBeta = false;
    bool hasTraceFit = false;
}; 
static const SINGLEBETA SINGLEBETA_DEFAULT_STRUCT;

struct VANDLES {
    std::string barType = "";
    double tof = -999;
    double corTof = -999;
    double qdcPos = -999;
    double qdc = -999;
    int barNum = -999;
    double tAvg = -999;
    double tDiff = -999;
    double wcTavg = -999;
    double wcTdiff = -999;    
    int sNum = -999;  //start detector number
    int vMulti = 0;
    double sTime = -999;
    double sQdc = -999;
};
static const VANDLES VANDLES_DEFAULT_STRUCT;
}  // namespace processor_struct

class PixTreeEvent : public TObject {
   public:
    PixTreeEvent() {}

    /* copy constructor */
    PixTreeEvent(const PixTreeEvent &obj) : TObject(obj) {
        externalTS1 = obj.externalTS1;
        externalTS2 = obj.externalTS2;
        eventNum = obj.eventNum;
        fileName = obj.fileName;
        clover_vec_ = obj.clover_vec_;
        doublebeta_vec_ = obj.doublebeta_vec_;
        dssd_vec_ = obj.dssd_vec_;
        gamma_scint_vec_ = obj.gamma_scint_vec_;
        logic_vec_ = obj.logic_vec_;
        pspmt_vec_ = obj.pspmt_vec_;
        root_dev_vec_ = obj.root_dev_vec_;
        singlebeta_vec_ = obj.singlebeta_vec_;
        vandle_vec_ = obj.vandle_vec_;
    }

    virtual ~PixTreeEvent() {}

    /* clear vectors and init all the values */
    virtual void Reset() {
        externalTS1 = 0;
        externalTS2 = 0;
        eventNum = 0;
        fileName = "";
        clover_vec_.clear();
        doublebeta_vec_.clear();
        dssd_vec_.clear();
        gamma_scint_vec_.clear();
        logic_vec_.clear();
        pspmt_vec_.clear();
        root_dev_vec_.clear();
        singlebeta_vec_.clear();
        vandle_vec_.clear();
    }

    /* data structures to be filled in the ROOT TTree */
    ULong64_t externalTS1 = 0;
    ULong64_t externalTS2 = 0;
    Double_t eventNum = 0;
    std::string fileName = "";
    std::vector<processor_struct::CLOVERS> clover_vec_;
    std::vector<processor_struct::DOUBLEBETA> doublebeta_vec_;
    std::vector<processor_struct::DSSD> dssd_vec_;
    std::vector<processor_struct::GAMMASCINT> gamma_scint_vec_;
    std::vector<processor_struct::LOGIC> logic_vec_;
    std::vector<processor_struct::PSPMT> pspmt_vec_;
    std::vector<processor_struct::ROOTDEV> root_dev_vec_;
    std::vector<processor_struct::SINGLEBETA> singlebeta_vec_;
    std::vector<processor_struct::VANDLES> vandle_vec_;

    ClassDef(PixTreeEvent, 1)
};

#endif  //PAASS_PROCESSORSTRUC_HPP
