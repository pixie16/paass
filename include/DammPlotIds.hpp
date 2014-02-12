#ifndef __DAMM_PLOTIDS_HPP_
#define __DAMM_PLOTIDS_HPP_ 1

namespace dammIds {
    const int GENERIC_CHANNEL = 10;

    namespace raw {
        /** Notice offset 1, so all
         * the ids are effectively +1 in the his file*/
        const int OFFSET = 1;
        const int RANGE = 1899;

        /** Notice that there is a space for 300 channels,
         * one 13 modules crate has 208 channels */
        const int D_RAW_ENERGY = 0;
        const int D_FILTER_ENERGY = 300;
        const int D_SCALAR = 600;
        const int D_TIME = 900;
        const int D_CAL_ENERGY = 1200;

        const int D_HIT_SPECTRUM = 1801; 
        const int D_SUBEVENT_GAP = 1802;
        const int D_EVENT_LENGTH = 1803;
        const int D_EVENT_GAP = 1804;
        const int D_EVENT_MULTIPLICITY = 1805;
        const int D_BUFFER_END_TIME = 1806;
        const int DD_RUNTIME_SEC = 1807;
        const int DD_DEAD_TIME_CUMUL = 1808; 
        const int DD_BUFFER_START_TIME = 1809;
        const int DD_RUNTIME_MSEC = 1810;
        const int D_NUMBER_OF_EVENTS = 1811;
        const int D_HAS_TRACE = 1812;
    }

    namespace mcp {	
        const int OFFSET = 2000;
        const int RANGE = 10;
    }

    namespace beta_scint {
        const int OFFSET = 2050;
        const int RANGE = 50;
    } 

    namespace neutron_scint {
        const int OFFSET = 2100;
        const int RANGE = 50;
    } 

    namespace liquid_scint {
        const int OFFSET = 2150;
        const int RANGE = 50;
    } 

    namespace hen3 {
        const int OFFSET = 2200;
        const int RANGE = 50;
    }

    namespace ge {
        const int OFFSET = 2500;
        const int RANGE = 500;
    } 

    namespace logic {
        const int OFFSET = 3000;
        const int RANGE = 100;
        const int MAX_LOGIC = 10; /*< maximum number of logic signals */
    }

    namespace triggerlogic {
        const int OFFSET = 3100;
        const int RANGE = 100;
    }

    namespace vandle{ //The RANGE can be greatly reduced. -SVP
        const int OFFSET = 3200;
        const int RANGE = 200;
    }

    //in PulserProcessor.cpp 
    namespace pulser{ 
        const int OFFSET = 3400;
        const int RANGE = 20;
    } 
   
    namespace waveformanalyzer{ 
        const int OFFSET = 3420;
        const int RANGE = 20;
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
        const int RANGE = 10;
    } 

    // in DssdProcessor.cpp
    namespace dssd {
        const int OFFSET = 7000;
        const int RANGE = 100;
    } 

    // in DssdProcessor.cpp
    namespace dssd4she {
        const int OFFSET = 7100;
        const int RANGE = 100;
    } 

    namespace trace {
        const int OFFSET = 7500;
        const int RANGE = 150;

        const int DD_TRACE = 0;
        const int DD_FILTER1 = 1;
        const int DD_FILTER2 = 2;
        const int DD_FILTER3 = 3;
        const int DD_AVERAGE_TRACE = 4;
        const int DD_REJECTED_TRACE = 5;
        const int D_ENERGY1 = 15;
        const int D_ENERGY2 = 16;	
        const int DD_DOUBLE_TRACE = 20;
        const int DD_ENERGY2__TDIFF = 21;
        const int DD_ENERGY2__ENERGY1 = 22;
        const int DD_TRIPLE_TRACE = 30;
        const int DD_TRIPLE_TRACE_FILTER1 = 31;
        const int DD_TRIPLE_TRACE_FILTER2 = 32;
        const int DD_TRIPLE_TRACE_FILTER3 = 33;

        namespace waveformanalyzer {
            const int DD_TRACES     = 40;
            const int D_CHISQPERDOF = 41;
            const int D_PHASE       = 42;
            const int DD_AMP        = 43;
        }
        
        // 1D-traces from the extracter
        const int D_TRACE = 50;
        const int maxSingleTraces = 99;
    } 
}

const int S1 = 2, S2 = 4, S3 = 8, S4 = 16, S5 = 32, S6 = 64, S7 = 128,
    S8 = 256, S9 = 512, SA = 1024, SB = 2048, SC = 4096,
    SD = 8192, SE = 16384, SF = 32768;


#endif // __DAMM_PLOTIDS_HPP_
