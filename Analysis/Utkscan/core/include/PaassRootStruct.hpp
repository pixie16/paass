/**Created by T.T. King 06/22/2018
 * */
/** PixTreeEvent class added by R. Yokoyama 08/31/2018 **/
#ifndef PAASS_PAASSSTRUC_HPP
#define PAASS_PAASSSTRUC_HPP

#include <TObject.h>
#include <TString.h>

namespace processor_struct {

struct BATO {
    std::vector<double> pQDCsums = {};
    double time = -999;
    double energy = -999;
    double qdc = -999;
    int padNum = -999;
    int detNum = -999;
};
static const BATO BATO_DEFAULT_STRUCT;

struct CLOVER {
    double energy = -999;
    double rawEnergy = -999;
    double time = -999;
    int detNum = -999;
    int cloverNum = -999;
};

static const CLOVER CLOVER_DEFAULT_STRUCT;

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

struct NEXT {
    /* Data container for NEXT */
    /* To be defined           */
};
static const NEXT NEXT_DEFAULT_STRUCT;

struct PID {
    double rfq_time = -999;
    double plastic_time = -999;
    double pin_0_time = -999;
    double pin_1_time = -999;
    double pin_0_energy = -999;
    double pin_1_energy = -999;
    double tac = -999;
    double tof = -999;
};
static const PID PID_DEFAULT_STRUCT;

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
    double time = -999;
    double qdc = -999;
    double tMaxVal = -999;
    bool isLowResBeta = false;
    bool isHighResBeta = false;
    bool hasTraceFit = false;
};
static const SINGLEBETA SINGLEBETA_DEFAULT_STRUCT;

struct VANDLE {
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
static const VANDLE VANDLE_DEFAULT_STRUCT;

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
        bato_vec_ = obj.bato_vec_;
        clover_vec_ = obj.clover_vec_;
        doublebeta_vec_ = obj.doublebeta_vec_;
        gammascint_vec_ = obj.gammascint_vec_;
        logic_vec_ = obj.logic_vec_;
        next_vec_ = obj.next_vec_;
        pid_vec_ = obj.pid_vec_;
        pspmt_vec_ = obj.pspmt_vec_;
        rootdev_vec_ = obj.rootdev_vec_;
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
        bato_vec_.clear();
        clover_vec_.clear();
        doublebeta_vec_.clear();
        gammascint_vec_.clear();
        logic_vec_.clear();
        next_vec_.clear();
        pid_vec_.clear();
        pspmt_vec_.clear();
        rootdev_vec_.clear();
        singlebeta_vec_.clear();
        vandle_vec_.clear();
    }

    /* data structures to be filled in the ROOT TTree */
    ULong64_t externalTS1 = 0;
    ULong64_t externalTS2 = 0;
    Double_t eventNum = 0;
    std::string fileName = "";
    std::vector<processor_struct::BATO> bato_vec_;
    std::vector<processor_struct::CLOVER> clover_vec_;
    std::vector<processor_struct::DOUBLEBETA> doublebeta_vec_;
    std::vector<processor_struct::GAMMASCINT> gammascint_vec_;
    std::vector<processor_struct::LOGIC> logic_vec_;
    std::vector<processor_struct::NEXT> next_vec_;
    std::vector<processor_struct::PID> pid_vec_;
    std::vector<processor_struct::PSPMT> pspmt_vec_;
    std::vector<processor_struct::ROOTDEV> rootdev_vec_;
    std::vector<processor_struct::SINGLEBETA> singlebeta_vec_;
    std::vector<processor_struct::VANDLE> vandle_vec_;

    ClassDef(PixTreeEvent, 1)
};

#endif  //PAASS_PROCESSORSTRUC_HPP
