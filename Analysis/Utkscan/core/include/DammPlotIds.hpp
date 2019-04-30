/** \file DammPlotIds.hpp
 * \brief Lists Offsets and Ranges for various Processors
 * @authors D. Miller, K. Miernik, S. V. Paulauskas
 */
#ifndef __DAMM_PLOTIDS_HPP_
#define __DAMM_PLOTIDS_HPP_

/**
 * Histogram sizes consts
 */
const int S1 = 2 /**< 2**1 */, S2 = 4/**< 2**2 */, S3 = 8/**< 2**3 */,
        S4 = 16/**< 2**4 */, S5 = 32/**< 2**5 */, S6 = 64/**< 2**6 */,
        S7 = 128/**< 2**7 */, S8 = 256/**< 2**8 */, S9 = 512/**< 2**9 */,
        SA = 1024/**< 2**10 */, SB = 2048/**< 2**11 */, SC = 4096/**< 2**12 */,
        SD = 8192/**< 2**13 */, SE = 16384/**< 2**14 */, SF = 32768/**< 2**15 */;

///Namespace containing all the histogram id definitions
namespace dammIds {
    const int GENERIC_CHANNEL = 10; //!< A generic channel

    ///All of the raw histograms
    namespace raw {
        /** Notice offset 1, so all
         * the ids are effectively +1 in the his file*/
        const int OFFSET = 1;//!< offset for raw histograms
        const int RANGE = 1899;//!< Range for raw histograms

        /** Notice that there is a space for 300 channels,
         * one 13 modules crate has 208 channels */
        const int D_RAW_ENERGY = 0;//!< Raw energies
        const int D_FILTER_ENERGY = 300;//!< Trace Filtered energies
        const int D_SCALAR = 600;//!< Rates for the detectors
        const int D_TIME = 900;//!< Arrival times for the channels
        const int D_CAL_ENERGY = 1200;//!< Calibrated energies

        const int D_HIT_SPECTRUM = 1801;//!< Channel hit spectrum
        const int D_SUBEVENT_GAP = 1802;//!< Time difference between sub events
        const int D_EVENT_LENGTH = 1803;//!< Length of the events in ns
        const int D_EVENT_GAP = 1804;//!< Time difference between buffers in ns
        const int D_EVENT_MULTIPLICITY = 1805;//!< Event multiplicity
        const int D_BUFFER_END_TIME = 1806;//!< Last time in Buffer
        const int DD_RUNTIME_SEC = 1807;//!< Run time in seconds
        const int DD_DEAD_TIME_CUMUL = 1808;//!< Cumulative Deadtime
        const int DD_BUFFER_START_TIME = 1809;//!< Buffer Start Time
        const int DD_RUNTIME_MSEC = 1810;//!< Run Time in ms
        const int D_NUMBER_OF_EVENTS = 1811;//!< Number of processed events
        const int D_HAS_TRACE = 1812;//!< Plot for Channels w/ Traces
        const int D_INTERNAL_TS_CHECK = 1813;//!< Plot of time difference between the 2 external time stamps
        const int DD_TRACE_MAX = 1814;//!< Plot for Max Value in Trace
        const int D_HAS_TRACE_2 = 1815;//!< Plot for Channels w/ valid waveform analysis 
        const int D_HAS_TRACE_3 = 1816;//!< Plot for Channels w/ valid fit analysis 
    }

    /// in PspmtProcessor.cpp
    namespace pspmt {
        const int OFFSET = 1900; //!< Offset for PspmtProcessor
        const int RANGE = 100; //!< Range for PspmtProcessor
    }

    ///in TemplateProcessor.cpp
    namespace dettemplate {
        const int OFFSET = 2000;//!< Offset for TemplateProcessor
        const int RANGE = 10;//!< Range for TemplateProcessor
    }

    ///in McpProcessor.cpp
    namespace mcp {
        const int OFFSET = 2000;//!< Offset for McpProcessor
        const int RANGE = 10;//!< Range for McpProcessor
    }

    ///in BetaScintProcessor.hpp
    namespace beta_scint {
        const int OFFSET = 2050;//!< Offset for BetaScintProcessor
        const int RANGE = 50;//!< Range for BetaScintProcessor
    }

       ///in SingleBetaProcessor.hpp
    namespace singlebeta {
        const int OFFSET = 2050;//!< Offset for SingleBetaProcessor
        const int RANGE = 50;//!< Range for SingleBetaProcessor
    }

    ///in NeutronScintProcessor.cpp
    namespace neutron_scint {
        const int OFFSET = 2100;//!< Offset for NeutronScintProcessor
        const int RANGE = 50;//!< Range for NeutronScintProcessor
    }

    ///in LiquidScintProcessor.cpp
    namespace liquid_scint {
        const int OFFSET = 2150;//!< Offset for LiquidScintProcessor
        const int RANGE = 20;//!< Range for LiquidScintProcessor
    }

    ///in DoubleBetaProcessor.cpp
    namespace doublebeta {
        const int OFFSET = 2170;//!< Offset for DoubleBetaProcessor
        const int RANGE = 10;//!< Range for DoubleBetaProcessor
    }

    ///in Hen3Processor.cpp
    namespace hen3 {
        const int OFFSET = 2200;//!< Offset for Hen3Processor
        const int RANGE = 50;//!< Range for Hen3Processor
    }
    ///in GammaScintProcessor.cpp
    namespace gscint {
        const int OFFSET = 2300;//!< Offset for GammaScintProcessor
        const int RANGE = 200;//!< Range for GammaScintProcessor
    }
    ///in CloverProcessor.cpp
    namespace clover {
        const int OFFSET = 2500;//!< Offset for CloverProcessor
        const int RANGE = 499;//!< Range for CloverProcessor
    }

    ///in CloverFragProcessor.cpp
    namespace cloverFrag {
        const int OFFSET = 2500;//!< Offset for CloverFragProcessor
        const int RANGE = 499;//!< Range for CloverFragProcessor
    }

    ///in GeProcessor.hpp
    namespace ge {
        const int OFFSET = 2999;//!< Offset for GeProcessor
        const int RANGE = 1;//!< Range for GeProcessor
    }

    ///in LogicProcessor.cpp
    namespace logic {
        const int OFFSET = 3000;//!< Offset for LogicProcessor
        const int RANGE = 150;//!< Range for the Logic Processor
    }

    ///in VandleProcessor.cpp
    namespace vandle {
        const int OFFSET = 3200;//!< Offset for the VandleProcessor
        const int RANGE = 150;//!< Range for the VandleProcessor
    }

    ///in TeenyVandleProcessor.cpp
    namespace teenyvandle {
        const int OFFSET = 3350;//!<Offset for TeenyVandleProcessor
        const int RANGE = 20;//!<Range for TeenyVandleProcessor
    }

    ///in PulserProcessor.cpp
    namespace pulser {
        const int OFFSET = 3400; //!< Offset for the PulserProcessor Hists
        const int RANGE = 20;//!< Range for the PulserProcessor Hists
    }

    /// in SsdProcessor.cpp
    namespace ssd {
        const int OFFSET = 4100;//!< Offset for SsdProcessor
        const int RANGE = 100;//!< Range for SsdProcessor
    }

    /// in ImplantSsdProcessor.cpp
    namespace implantSsd {
        const int OFFSET = 4200;//!< Offset for ImplantSsdProcessor
        const int RANGE = 300;//!< Range for ImplantSsdProcessor
    }

    ///in IonChamberProcessor.cpp
    namespace ionChamber {
        const int OFFSET = 4700;//!< Offset for IonChamberProcessor
        const int RANGE = 100;//!< Range for IonChamberProcessor
    }

    ///in PositionProcessor.cpp
    namespace position {
        const int OFFSET = 5000;//!< Offset for PositionProcessor
        const int RANGE = 600;//!< Range for PositionProcessor
    }

    /// in Correlator.cpp
    namespace correlator {
        const int OFFSET = 6000;//!< Offset for Correlator
        const int RANGE = 10;//!< Range for Correlator
    }

    /// For Experiment specific processors
    namespace experiment {
        const int OFFSET = 6050; //!< Offset for Experiment processors
        const int RANGE = 250; //!< Range for Experiment Processors
    }

    /// in DssdProcessor.cpp
    namespace dssd {
        const int OFFSET = 7000;//!< Offset for DssdProcessor
        const int RANGE = 100;//!< Range for DssdProcessor
    }

    /// in DssdProcessor.cpp
    namespace dssd4she {
        const int OFFSET = 7100;//!< Offset for Dssd4SheProcessor
        const int RANGE = 200;//!< Range for Dssd4SheProcessor
    }

    /// Definitions for all of the traces
    namespace analyzers {
        const int traceBins = SA; //!< Number of Bins for Traces

        namespace tracefilter {
            const unsigned int OFFSET = 7500;
            const unsigned int RANGE = 10;
        }

        namespace extractor {
            const unsigned int OFFSET = 7550;
            const unsigned int RANGE = 1;
        }

        namespace waa {
            const unsigned int OFFSET = 7510;
            const unsigned int RANGE = 1;
        }

        namespace doubletraceanalyzer {
            const int D_ENERGY2 = 16;//!< distribution of energy 2
            const int DD_DOUBLE_TRACE = 20;//!< double traces
            const int DD_ENERGY2__TDIFF = 21;//!< energy 2 vs tdiff between 1 and 2
            const int DD_ENERGY2__ENERGY1 = 22;//!< energy 1 vs energy 2
            const int DD_TRIPLE_TRACE = 30;//!< results of triple traces
            const int DD_TRIPLE_TRACE_FILTER1 = 31;//!< results of trace filter 1
            const int DD_TRIPLE_TRACE_FILTER2 = 32;//!< results of trace filter 2
            const int DD_TRIPLE_TRACE_FILTER3 = 33; //!< results of trace filter 3
        }
    }
}
#endif // __DAMM_PLOTIDS_HPP_
