/** \file GeProcessor.hpp
 * \brief Processor for germanium detectors
 */

#ifndef __GEPROCESSOR_HPP_
#define __GEPROCESSOR_HPP_

#include <map>
#include <vector>
#include <utility>
#include <cmath>

#include "EventProcessor.hpp"
#include "RawEvent.hpp"

namespace dammIds {
    namespace ge {
        /*
        * Naming conventions:
        * Beta offset = 10
        * Decay offset = 20
        * Addback offset = 50
        */

        const unsigned int MAX_CLOVERS = 4; //!< for *_DETX spectra
        const unsigned int MAX_TIMEX = 5; //!< for *_TIMEX spectra

        const int D_ENERGY = 0;//!< Energy
        const int D_ENERGY_CLOVERX = 2; //!< Energy Full Clover
        const int D_ENERGY_MOVE = 6;//!< Energy during move
        const int D_MULT = 9;//!< Multiplicity
        const int D_ADD_ENERGY = 50; //!< Addback energy
        const int D_ADD_ENERGY_CLOVERX = 55;//!< Add back energy per clover
        const int D_ADD_ENERGY_TOTAL = 59;//!< Addback energy total

        const int DD_ENERGY = 100;//!< Gamma-Gamma Energy
        const int DD_ENERGY_PROMPT = 101;//!< Gamma-Gamma Spectra - Prompt
        const int DD_ENERGY_CGATE1 = 102;//!< Gamma-Gamma - Cycle Gate 1
        const int DD_ENERGY_CGATE2 = 103;//!< Gamma-Gamma - Cycle Gate 2

        const int DD_TDIFF__GATEX = 105;//!< Tdiff Gamma - Beta - Gated
        const int DD_ENERGY__GATEX = 106;//!< Energy - Gated
        const int DD_ANGLE__GATEX = 107;//!< Gamma Angle - Gated

        const int DD_ENERGY__TIMEX = 120;//!< Energy vs. Time

        const int DD_ADD_ENERGY = 150;//!< Addback Energy

        const int DD_TDIFF__GAMMA_GAMMA_ENERGY = 155;//!< Tdiff vs Gamma-Gamma Energy
        const int DD_TDIFF__GAMMA_GAMMA_ENERGY_SUM = 156;//!< Tdiff vs. Gamma-Gamma Energy sum

        const int DD_ADD_ENERGY__TIMEX = 170;//!< Addback Energy vs. Time

        namespace betaGated {
            const int D_ENERGY = 10;//!< Beta Gated Energy
            const int D_ENERGY_PROMPT = 11;//!< Beta Gated Prompt Energy
            const int D_ENERGY_CLOVERX = 12;//!< Beta Gated Clover Energy
            const int D_ENERGY_MOVE = 16;//!< Beta Gated Energy during Move
            const int D_ENERGY_BETA0 = 17;//!< Energy Beta 0
            const int D_ENERGY_BETA1 = 18;//!< Energy Beta 1
            const int DD_ENERGY__BETAGAMMALOC = 19;//!< Energy vs Beta-Gamma Loc

            const int D_ADD_ENERGY = 60;//!< Beta Gated Addback Energy
            const int D_ADD_ENERGY_PROMPT = 61;//!< Beta Gated Add Back Prompt Energy
            const int D_ADD_ENERGY_CLOVERX = 65;//!< Beta Gated Addback Clover Energy
            const int D_ADD_ENERGY_TOTAL = 69;//!< Beta Gated Addback Total Energy

            const int DD_ENERGY = 110;//!< Beta Gated Gamma-Gamma
            const int DD_ENERGY_PROMPT = 111;//!< Beta Gated Gamma-Gamma Prompt
            const int DD_ENERGY_CGATE1 = 112;//!< Beta Gated Gamma-Gamma Cycle 1
            const int DD_ENERGY_CGATE2 = 113;//!< Beta Gated Gamma-Gamma Cycle 2
            const int DD_ENERGY_BDELAYED = 114;//!< Beta Gated Gamma-Gamma Beta Delayed

            const int DD_TDIFF__GATEX = 115;//!< Beta Gated Time Diff Gated
            const int DD_ENERGY__GATEX = 116;//!< Beta Gated Energy Gated
            const int DD_ANGLE__GATEX = 117;//!< Beta Gated Angle Gated

            const int DD_TDIFF__GAMMA_ENERGY = 118; //!< Beta Gated TDiff vs. Gamma Energy
            const int DD_TDIFF__BETA_ENERGY = 119;//!< Beta Gated Tdiff vs. Beta Energy

            const int DD_ENERGY__TIMEX = 130;//!< Beta Gated Energy vs. Time
            const int DD_ENERGY__TIMEX_GROW = 135;//!< Beta Gated Energy vs. Grow Cycle
            const int DD_ENERGY__TIMEX_DECAY = 140;//!< Beta Gated Energy vs. Decay Cycle

            const int DD_ADD_ENERGY = 160;//!< Beta Gated Gamma-Gamma Addback
            const int DD_ADD_ENERGY_PROMPT = 161;//!< Beta Gated Gamma-Gamma Prompt addback
            const int DD_ADD_ENERGY__TIMEX = 180;//!< Beta Gated Addback Energy vs. Time
        }

        namespace multi {
            const int D_ADD_ENERGY = 52;//!< multi gated addback energy
            const int D_ADD_ENERGY_TOTAL = 54;//!< multi gated addback total energy
            const int DD_ADD_ENERGY = 152;//!< multi gated addback energy
            namespace betaGated {
                const int D_ADD_ENERGY = 62;//!< beta/multi gated addback energy
                const int D_ADD_ENERGY_PROMPT = 63;//!< beta/multi gated prompt addback energy
                const int D_ADD_ENERGY_TOTAL = 64;//!< beta/multi gated total addback energy
                const int DD_ADD_ENERGY = 162;//!< beta/multi gated addback energy
                const int DD_ADD_ENERGY_PROMPT = 163;//!< beta/multi gated addback energy
            }
        }
    } // end namespace ge
}

#ifdef GGATES
/** Class to store line gate needed for gamma-gamma gates.
 * Used if GGATES flag is defined in the makefile. */
class LineGate {
    public:
        LineGate() {
            min = -1;
            max = -1;
        }

        LineGate(double emin, double emax) {
            min = emin;
            max = emax;
        }

        /** Sanity check of gate definition. */
        bool Check() {
            if (min > 0 && max > min)
                return true;
            else
                return false;
        }

        /** Check if x is inside gate (borders included) */
        bool IsWithin(double x) {
            if (x >= min && x <= max)
                return true;
            else
                return false;
        }

        bool operator< (const LineGate& other) const {
            return (min < other.min);
        }

        double min;
        double max;
};
#endif


/** Simple structure-like class to store info on addback reconstructed
 * event.*/
class AddBackEvent {
    public:
        AddBackEvent() {
            energy = 0;
            time = 0;
            multiplicity = 0;
        }

        AddBackEvent(double ienergy, double itime, unsigned imultiplicity) {
            energy = ienergy;
            time = itime;
            multiplicity = imultiplicity;
        }

        double energy;
        double time;
        unsigned multiplicity;
};


class GeProcessor : public EventProcessor
{
protected:
    static const unsigned int chansPerClover = 4; /*!< number of channels per clover */

    std::map<int, int> leafToClover;   /*!< Translate a leaf location to a clover number */
    std::vector<float> timeResolution; /*!< Contatin time resolutions used */
    unsigned int numClovers;           /*!< number of clovers in map */

    /** Returns lowest difference between gamma and beta times and EventData of the beta event.
     * Takes gTime in pixie clock units, returns value in seconds. */
    EventData BestBetaForGamma(double gTime);

    /** Returns true if gamma-beta correlation time within good limits.
     * Browses through all beta events in Beta correlation place to find
     * the lowest difference. Takes gTime in pixie clock, limit in
     * seconds. */
    bool GoodGammaBeta(double gTime);

    /** Preprocessed good ge events, filled in PreProcess.*/
    std::vector<ChanEvent*> geEvents_;

    void DeclareHistogramGranY(int dammId, int xsize, int ysize,
			       const char *title, int halfWordsPerChan,
			       const std::vector<float> &granularity, const char *units );
    void granploty(int dammId, double x, double y,
                   const std::vector<float> &granularity);
    void symplot(int dammID, double bin1, double bin2);

    /** addbackEvents vector of vectors, where first vector
     * enumerates cloves, second events
     */
    std::vector< std::vector<AddBackEvent> > addbackEvents_;
    /** tas vector for total energy absorbed, similar structure as addback
     * but there is only one "super-clover" (sum of all detectors)*/
    std::vector<AddBackEvent> tas_;
#ifdef GGATES
    std::vector< std::vector<LineGate> > gGates;
#endif

    /* Gamma low threshold in keV */
    double gammaThreshold_;

    /** Low and high ratio between low and high gain to
     * be accepted (for data without saturation flag */
    double lowRatio_;
    double highRatio_;

    /** Addback subevent window in seconds */
    double subEventWindow_;

    /** Prompt Gamma-beta limit in seconds */
    double gammaBetaLimit_;

    /** Prompt Gamma-gamma limit in seconds */
    double gammaGammaLimit_;

    /** Cycle gates replace early/high limits. Gate set on cycle in time
     * allows to check the gamma-gamma coincidences within the chosen
     * range of cycle (e.g 1 - 1.5 s) */
    double cycle_gate1_min_;
    double cycle_gate1_max_;
    double cycle_gate2_min_;
    double cycle_gate2_max_;

public:
    GeProcessor(double gammaThreshold, double lowRatio,
                double highRatio, double subEventWindow,
                double gammaBetaLimit, double gammaGammaLimit,
                double cycle_gate1_min, double cycle_gate1_max,
                double cycle_gate2_min, double cycle_gate2_max);
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
};

#endif // __GEPROCESSOR_HPP_
