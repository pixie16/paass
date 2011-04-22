#ifndef __DAMM_PLOTIDS_H_
#define __DAMM_PLOTIDS_H_ 1

const int GENERIC_CHANNEL = 10;

namespace dammIds {
    // in mcp.cpp
    namespace mcp {	
	const int D_POSX   = 921;
	const int D_POSY   = 922;
	const int DD_POSXY = 923;
    }

    // in GeProcessor.cpp
    namespace ge {
	// clovers
	const unsigned int MAX_CLOVERS = 4; // for *_DETX spectra
	
	const int D_CLOVER_ENERGY_DETX   = 691; // for x detectors
	const int D_CLOVER_ENERGY_ALL    = 695;
	const int DD_CLOVER_ENERGY_RATIO = 1609;

	const int D_ENERGY          = 1500;
	const int D_ENERGY_NTOF1    = 1506;
	const int D_ENERGY_NTOF2    = 1507;
	const int D_ENERGY_HEN3     = 1508;

	const int D_ENERGY_LOWGAIN  = 1530;
	const int D_ENERGY_HIGHGAIN = 1540;

	const int D_ADD_ENERGY      = 1700;
	const int D_ADD_ENERGY_DETX = 1701; // for x detectors 

	const int D_MULT = 1800;

	// correlated with a decay
	namespace decayGated {
	    // 0-5 granularities (1 us -> 100 ms)
	    const int D_ENERGY = 1502;

	    const int D_ADD_ENERGY      = 1720;
	    const int D_ADD_ENERGY_DETX = 1721; // for x detectors
	    namespace withBeta {
		const int DD_ENERGY__DECAY_TIME_GRANX = 1730;
		namespace multiplicityGated {
		    const int DD_ENERGY__DECAY_TIME_GRANX = 1740;
		}
	    }
	    namespace withoutBeta {
		const int DD_ENERGY__DECAY_TIME_GRANX = 1750;
	    }
	    namespace matrix {
		const int DD_ENERGY_PROMPT = 1670;
		const int DD_ADD_ENERGY_PROMPT = 1682;
	    }
	} // decay-gated namespace
	namespace betaGated {
	    const int D_ENERGY        = 1501;
	    const int D_ENERGY_BETA0  = 1550;
	    const int D_ENERGY_BETA1  = 1551;
	    const int D_ENERGY_NTOF1  = 1552;
	    const int D_ENERGY_NTOF2  = 1553;
	    const int D_ENERGY_HEN3   = 1554;
	    const int D_TDIFF         = 1602;

	    const int D_ADD_ENERGY      = 1710;
	    const int D_ADD_ENERGY_DETX = 1711; // for x detectors

	    const int DD_TDIFF__GAMMA_ENERGY = 1603;
	    const int DD_TDIFF__BETA_ENERGY  = 1604;
	    namespace matrix {
		const int DD_ENERGY_PROMPT = 1660;
		const int DD_ADD_ENERGY_PROMPT = 1681;
	    }
	    namespace multiplicityGated {
		const int D_ENERGY = 1505;
		const int DD_TDIFF__GAMMA_ENERGY = 1607;
	    }
	} // beta-gated namespace
	namespace implantGated {
	    const int D_ENERGY = 1503;
	    const int DD_ENERGY__TDIFF = 1605;
	}
	namespace multiplicityGated {
	    const int D_ENERGY = 1504;
	    // cross-references
	    namespace betaGated = ::dammIds::ge::betaGated::multiplicityGated;
	}
	namespace matrix {
	    const int DD_ENERGY_PROMPT = 1600;
	    const int D_TDIFF          = 1601;
	    const int DD_ADD_ENERGY_PROMPT = 1680;
	    // cross-references
	    namespace betaGated = ::dammIds::ge::betaGated::matrix;
	    namespace decayGated = ::dammIds::ge::decayGated::matrix;
	} // gamma-gamma matrix
    } // end namespace ge

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
    namespace vandle{ //in VandleProcessor.cpp
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
	const int D_CONDITION = 6000;
	const int D_TIME_BW_IMPLANTS = 6001;
    } // correlator namespace
    // in dssd_sub.cpp
    namespace dssd {
	const int DD_IMPLANT_POSITION = 725;
	const int DD_DECAY_POSITION   = 726;
	const int DD_IMPLANT_FRONT_ENERGY__POSITION = 741;
	const int DD_IMPLANT_BACK_ENERGY__POSITION  = 742;
	const int DD_DECAY_FRONT_ENERGY__POSITION   = 743;
	const int DD_DECAY_BACK_ENERGY__POSITION    = 744;

	const int DD_ENERGY__DECAY_TIME_GRANX = 750;
    } // dssd namespace

    // in SsdProcessor.cpp
    namespace ssd {
	const int NUM_DETECTORS = 4;

        const int DD_POSITION__ENERGY_DETX = 2701; // for x detectors
    } // ssd namespace
    // in ImplantSsdProcessor.cpp
    namespace implantSsd {
	const int DD_IMPLANT_ENERGY__POSITION = 2741;
	const int DD_DECAY_ENERGY__POSITION   = 2743;
	const int DD_IMPLANT_ENERGY__TOF      = 2745;
	const int DD_ENERGY__DECAY_TIME_GRANX = 2750;
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
    namespace trace {
// in trace.cpp
	const int DD_TRACE            = 850;
	const int DD_FILTER1          = 851;
	const int DD_FILTER2          = 852;
	const int DD_FILTER3          = 853;
	const int DD_AVERAGE_TRACE    = 854;
	const int D_ENERGY1           = 865;
	const int D_ENERGY2           = 866;	
	const int DD_DOUBLE_TRACE     = 870;
	const int DD_ENERGY2__TDIFF   = 871;
	const int DD_ENERGY2__ENERGY1 = 872;
    } // trace namespace
    namespace misc {
// in detector_driver.cpp
	const int D_HAS_TRACE = 800;
// in pixie_std.cpp
	const int D_HIT_SPECTRUM = 1000;
	const int D_SUBEVENT_GAP = 1001;
	const int D_EVENT_LENGTH = 1002;
	const int D_EVENT_GAP    = 1003;
	const int D_EVENT_MULTIPLICITY = 1004;
	const int D_BUFFER_END_TIME    = 1005;
	const int DD_RUNTIME_SEC       = 1006;
	const int DD_DEAD_TIME_CUMUL   = 1007; // not currently used
	const int DD_BUFFER_START_TIME = 1008;
	const int DD_RUNTIME_MSEC      = 1010;
	const int D_NUMBER_OF_EVENTS   = 1011;
	namespace offsets {
	    const int D_RAW_ENERGY        = 100;
	    const int D_SCALAR            = 300;
	    const int D_TIME              = 400;
	    const int D_CAL_ENERGY        = 500;
	    const int D_CAL_ENERGY_REJECT = 600;
	}
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
void DeclareHistogram2D(int dammId, int xSize, int ySize,
			const char *title, int halfWordsPerChan,
			int xHistLength, int xLow, int xHigh,
			int yHistLength, int yLow, int yHigh);
void DeclareHistogram2D(int dammId, int xSize, int ySize,
			const char* title, int halfWordsPerChan = 1);

// powers of 2 for damm sizes
extern const int S1, S2, S3, S4, S5, S6, S7, S8, S9, SA, SB, SC, SD, SE, SF;

#endif // __DAMM_PLOTIDS_H_
