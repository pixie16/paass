#ifndef __DAMM_PLOTIDS_H_
#define __DAMM_PLOTIDS_H_ 1

const int GENERIC_CHANNEL = 10;

namespace dammIds {

    // in ScintProcessor.cpp
    namespace scint {
        namespace neutr {
            namespace betaGated {
                const int D_ENERGY_DETX = 1900; // for 3 detectors (1-3)
            }
            namespace gammaGated {
                const int D_ENERGY_DETX = 1903; // for 3 detectors (1-3)
            }
            namespace betaGammaGated {
                const int D_ENERGY_DETX = 1906; // for 3 detectors (1-3)
            }
        } // neutr namespace
        const int DD_TQDCBETA         = 1909;
        const int DD_MAXBETA          = 1910;
        const int DD_TQDCLIQUID       = 1911;
        const int DD_MAXLIQUID        = 1912;
        const int D_DISCRIM           = 1913;
        const int DD_NGVSE            = 1914;
        const int DD_TOFLIQUID        = 1915;
        const int DD_TRCLIQUID        = 1916;
    } // scint namespace

    // in mcp.cpp
    namespace mcp {	
        const int D_POSX   = 1921;
        const int D_POSY   = 1922;
        const int DD_POSXY = 1923;
    }


    //in VandleProcessor.cpp
    namespace vandle{ 
        const int D_PROBLEMS  = 2600;
        const int DD_PROBLEMSQDC = 2601;

        //HISTOGRAMS FOR INDIVIDUAL ENDS OF BARS
        const int DD_TQDCBARS         = 2000;
        const int DD_MAXIMUMBARS      = 2001;
        const int DD_TIMEDIFFBARS     = 2002;
        const int DD_TOFBARS          = 2003;
        const int DD_CORTOFBARS       = 2004;
        const int DD_TQDCBETA         = 2005;
        const int DD_MAXBETA          = 2006;

        //HISTOGRAMS FOR EACH BAR
        const int DD_TQDCAVEVSTDIFF   = 2010;

        //HISTOGRAMS FOR EACH BAR W.R.T. EACH TRIGGER
        const int DD_TDIFFVSTOF       = 2030;
        const int DD_MAXRVSTOF        = 2070;
        const int DD_MAXLVSTOF        = 2110;
        const int DD_MAXBETA1VSTOF    = 2150;
        const int DD_MAXBETA2VSTOF    = 2190;
        const int DD_TQDCAVEVSTOF     = 2230;

        const int DD_TDIFFVSCORTOF    = 2270;
        const int DD_MAXRVSCORTOF     = 2310;
        const int DD_MAXLVSCORTOF     = 2350;
        const int DD_MAXBETA1VSCORTOF = 2390;
        const int DD_MAXBETA2VSCORTOF = 2430;
        const int DD_TDIFFVSCORCORTOF = 2470;
        const int DD_TQDCAVEVSCORTOF  = 2510;
    }//vandle namespace
    namespace waveformanalyzer{
	
    } // waveformanalyzer namespace

    namespace pulserprocessor{ //in PulserProcessor.cpp 
        const int D_TIMEDIFF     = 5000;
        const int D_PROBLEMSTUFF = 5001;

        const int DD_QDC         = 5002;
        const int DD_MAX         = 5003;
        const int DD_PVSP        = 5004;
        const int DD_MAXVSTDIFF  = 5005;
        const int DD_QDCVSMAX    = 5006;
        const int DD_AMPMAPSTART = 5007;
        const int DD_AMPMAPSTOP  = 5008;
    } // pulser namespace

    // in correlator.cpp
    namespace correlator {
        const int D_CONDITION            = 6000;
        const int D_TIME_BW_IMPLANTS     = 6001;
        const int D_TIME_BW_ALL_IMPLANTS = 6002;
    } // correlator namespace

    // in dssd_sub.cpp
    namespace dssd {
        const int DD_IMPLANT_POSITION = 1725;
        const int DD_DECAY_POSITION   = 1726;
        const int DD_IMPLANT_FRONT_ENERGY__POSITION = 1741;
        const int DD_IMPLANT_BACK_ENERGY__POSITION  = 1742;
        const int DD_DECAY_FRONT_ENERGY__POSITION   = 1743;
        const int DD_DECAY_BACK_ENERGY__POSITION    = 1744;

        const int DD_ENERGY__DECAY_TIME_GRANX = 1750;
    } // dssd namespace

    // in SsdProcessor.cpp
    namespace ssd {
        const int NUM_DETECTORS = 4;
        const int DD_POSITION__ENERGY_DETX = 2701; // for x detectors
    } // ssd namespace

    // in ImplantSsdProcessor.cpp
    namespace implantSsd {
        const int MAX_TOF = 5;

        const int DD_IMPLANT_ENERGY__LOCATION  = 2740;
        const int DD_DECAY_ENERGY__LOCATION    = 2741;
        const int DD_ENERGY__LOCATION_BEAM     = 2742;
        const int DD_ENERGY__LOCATION_NOBEAM   = 2743;
        const int DD_ENERGY__LOCATION_VETO     = 2744;
        const int DD_ENERGY__LOCATION_PROJLIKE = 2745;
        const int DD_ENERGY__LOCATION_UNKNOWN  = 2746;
        const int DD_LOC_VETO__LOC_SSD         = 2747;
        const int DD_TOTENERGY__ENERGY         = 2748;
        const int DD_ALL_ENERGY__LOCATION      = 2749;
        const int DD_FISSION_ENERGY__LOCATION  = 2750;

        const int DD_DECAY_ALL_ENERGY__TX     = 2800;
        const int DD_DECAY_NOBEAM_ENERGY__TX  = 2810;
        const int DD_VETO_ENERGY__TX          = 2820;
        const int DD_FIRST_DECAY_ENERGY__TX   = 2830;

        const int DD_ALL_ENERGY__TOFX         = 2850;
        const int DD_IMPLANT_ENERGY__TOFX     = 2860;   
        const int DD_VETO_ENERGY__TOFX        = 2870;
        const int DD_ALL_ENERGY__TOFX_GATED   = 2880;

        const int D_TDIFF_FOIL_IMPLANT = 2900;
        const int D_TDIFF_FOIL_IMPLANT_MULT1 = 2901;

        const int D_FAST_DECAY_TRACE  = 4100;
        const int D_HIGH_ENERGY_TRACE = 4200;
    }

    // in MtcProcessor.cpp
    namespace mtc {
        const int D_TDIFF0        = 1510;
        const int D_TDIFF1        = 1511;
        const int D_TDIFFSUM      = 1512;
        const int D_MOVETIME      = 1513;
        const int D_COUNTER       = 1520;
        const int D_COUNTER_MOVE0 = 1521;
        const int D_COUNTER_MOVE1 = 1522;
    } // mtc namespace

    namespace logic {
      const int MAX_LOGIC = 10; /*< maximum number of logic signals */
      const int D_COUNTER_START = 3000;
      const int D_COUNTER_STOP  = 3005;
      const int D_TDIFF_STARTX  = 3010;
      const int D_TDIFF_STOPX   = 3020;
      const int D_TDIFF_SUMX    = 3030;
      const int D_TDIFF_LENGTHX = 3050;
    } // logic namespace

    namespace triggerlogic {
      const int DD_RUNTIME_LOGIC = 3080;
    }

    // in trace.cpp
    namespace trace {
        const int DD_TRACE            = 1850;
        const int DD_FILTER1          = 1851;
        const int DD_FILTER2          = 1852;
        const int DD_FILTER3          = 1853;
        const int DD_AVERAGE_TRACE    = 1854;
        const int DD_REJECTED_TRACE   = 1855;
        const int D_ENERGY1           = 1865;
        const int D_ENERGY2           = 1866;	
        const int DD_DOUBLE_TRACE     = 1870;
        const int DD_ENERGY2__TDIFF   = 1871;
        const int DD_ENERGY2__ENERGY1 = 1872;
        const int DD_TRIPLE_TRACE         = 1880;
        const int DD_TRIPLE_TRACE_FILTER1 = 1881;
        const int DD_TRIPLE_TRACE_FILTER2 = 1882;
        const int DD_TRIPLE_TRACE_FILTER3 = 1883;
        // 1D-traces from the extracter
        const int D_TRACE = 4000;
    } // trace namespace
    
    // in pixie_std.cpp
    namespace misc {
        const int D_HIT_SPECTRUM = 1400;
        const int D_SUBEVENT_GAP = 1401;
        const int D_EVENT_LENGTH = 1402;
        const int D_EVENT_GAP    = 1403;
        const int D_EVENT_MULTIPLICITY = 1404;
        const int D_BUFFER_END_TIME    = 1405;
        const int DD_RUNTIME_SEC       = 1406;
        const int DD_DEAD_TIME_CUMUL   = 1407; // not currently used
        const int DD_BUFFER_START_TIME = 1408;
        const int DD_RUNTIME_MSEC      = 1410;
        const int D_NUMBER_OF_EVENTS   = 1411;
    } // misc namespace
}

// function prototypes
/*!
  plotting routine to interact with DAMM
*/
void plot(int dammid, double val1, double val2 = -1, double val3 = -1,
	  const char* name="h");

// miscellaneous damm fortran functions
extern "C" bool bantesti_(const int &, const int &, const int &);
extern "C" void count1cc_(const int &, const int &, const int &);
extern "C" void set2cc_(const int &, const int &, const int &, const int &);

// c++ wrapper function for hd* calls
void DeclareHistogram1D(int dammId, int xSize, const char* title,
			int halfWordsPerChan, int xHistLength,
			int xLow, int xHigh);
void DeclareHistogram1D(int dammId, int xSize, const char* title,
			int halfWordsPerChan = 2);
void DeclareHistogram1D(int dammId, int xsize, const char* title,
			int halfWordsPerChan, int contraction);
void DeclareHistogram2D(int dammId, int xSize, int ySize,
			const char *title, int halfWordsPerChan,
			int xHistLength, int xLow, int xHigh,
			int yHistLength, int yLow, int yHigh);
void DeclareHistogram2D(int dammId, int xSize, int ySize,
			const char* title, int halfWordsPerChan = 1);
void DeclareHistogram2D(int dammId, int xSize, int ySize,
			const char* title, int halfWordsPerChan,
			int xContraction, int yContraction);

// powers of 2 for damm sizes
extern const int S1, S2, S3, S4, S5, S6, S7, S8, S9, SA, SB, SC, SD, SE, SF;

#endif // __DAMM_PLOTIDS_H_
