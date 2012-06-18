#ifndef __DAMM_PLOTIDS_HPP_
#define __DAMM_PLOTIDS_HPP_ 1

namespace dammIds {
    const int GENERIC_CHANNEL = 10;

    namespace raw {
        const int OFFSET = 1;
        const int RANGE = 1899;

        const int D_RAW_ENERGY = 0;
        const int D_FILTER_ENERGY = 200; // intermediate step with "raw" filter energy	
        const int D_SCALAR = 400;/**< These get filled in PixieStd.cpp */
        const int D_TIME = 600; /**< These get filled in PixieStd.cpp */
        const int D_HAS_TRACE = 800;
        const int D_CAL_ENERGY = 1000;
        const int D_CAL_ENERGY_REJECT = 1200;
        const int D_HIT_SPECTRUM = 1800;
        const int D_SUBEVENT_GAP = 1801;
        const int D_EVENT_LENGTH = 1802;
        const int D_EVENT_GAP    = 1803;
        const int D_EVENT_MULTIPLICITY = 1804;
        const int D_BUFFER_END_TIME    = 1805;
        const int DD_RUNTIME_SEC       = 1806;
        const int DD_DEAD_TIME_CUMUL   = 1807; 
        const int DD_BUFFER_START_TIME = 1808;
        const int DD_RUNTIME_MSEC      = 1810;
        const int D_NUMBER_OF_EVENTS   = 1811;
    }

    namespace mcp {	
        const int OFFSET = 2000;
        const int RANGE = 10;
    }

    namespace scint {
        const int OFFSET = 2100;
        const int RANGE = 100;
    } 

    // in GeProcessor.cpp
    namespace ge {
        const int OFFSET = 2500;
        const int RANGE = 500;
    } 

    namespace logic {
        const int OFFSET = 3000;
        const int RANGE = 80;
        const int MAX_LOGIC = 10; /*< maximum number of logic signals */
    }

    namespace triggerlogic {
        const int OFFSET = 3080;
        const int RANGE = 20;
    }

    namespace vandle{ //The RANGE can be greatly reduced. -SVP
        const int OFFSET = 3100;
        const int RANGE = 900;
    }

    namespace waveformanalyzer{ 
        const int OFFSET = 4000;
        const int RANGE = 100;
    }

    // in SsdProcessor.cpp
    namespace ssd {
        const int OFFSET = 4100;
        const int RANGE = 100;
    } 

    // in ImplantSsdProcessor.cpp
    namespace implantSsd {
        const int OFFSET = 4200;
        const int RANGE = 300;
    }

    // in MtcProcessor.cpp
    namespace mtc {
        const int OFFSET = 4500;
        const int RANGE = 100;
    } 

    //in PulserProcessor.cpp 
    namespace pulser{ //Needs no more than 30 -SVP
        const int OFFSET = 4600;
        const int RANGE = 50;
    } 

    namespace teenyvandle{ //Same story here. -SVP
       const int OFFSET = 4650;
       const int RANGE  = 50;
    }

    //in IonChamberProcessor.cpp 
    namespace ionChamber{ 
        const int OFFSET = 4700;
        const int RANGE = 100;
    } 

    namespace position {
        const int OFFSET = 5000;
        const int RANGE = 600;
    }

    // in Correlator.cpp
    namespace correlator {
        const int OFFSET = 6000;
        const int RANGE = 100;
    } 

    // in DssdProcessor.cpp
    namespace dssd {
        const int OFFSET = 7000;
        const int RANGE = 500;
    } 

    namespace trace {
        const int OFFSET = 7500;
        const int RANGE = 200;

        const int DD_TRACE = 0;
        const int DD_FILTER1          = 1;
        const int DD_FILTER2          = 2;
        const int DD_FILTER3          = 3;
        const int DD_AVERAGE_TRACE    = 4;
        const int DD_REJECTED_TRACE   = 5;
        const int D_ENERGY1           = 15;
        const int D_ENERGY2           = 16;	
        const int DD_DOUBLE_TRACE     = 20;
        const int DD_ENERGY2__TDIFF   = 21;
        const int DD_ENERGY2__ENERGY1 = 22;
        const int DD_TRIPLE_TRACE         = 30;
        const int DD_TRIPLE_TRACE_FILTER1 = 31;
        const int DD_TRIPLE_TRACE_FILTER2 = 32;
        const int DD_TRIPLE_TRACE_FILTER3 = 33;
        // 1D-traces from the extracter
        const int D_TRACE = 50;
    } 
}

const int S1 = 2, S2 = 4, S3 = 8, S4 = 16, S5 = 32, S6 = 64, S7 = 128,
    S8 = 256, S9 = 512, SA = 1024, SB = 2048, SC = 4096,
    SD = 8192, SE = 16384, SF = 32768;


#endif // __DAMM_PLOTIDS_HPP_
