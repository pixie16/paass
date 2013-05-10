/*! \file GeProcessor.hpp
 *
 * Processor for germanium detectors
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

        const unsigned int MAX_CLOVERS = 4; // for *_DETX spectra
        const unsigned int MAX_TIMEX = 5; // for *_TIMEX spectra

        const int D_ENERGY = 0;
        const int D_ENERGY_CLOVERX = 2; 

        const int D_ENERGY_MOVE = 6;
        const int D_MULT = 9;

        const int D_ADD_ENERGY = 50; 
        const int D_ADD_ENERGY_CLOVERX = 55;
        const int D_ADD_ENERGY_TOTAL = 59;

        // 2D spectra
        const int DD_ENERGY = 100;
        const int DD_ENERGY_PROMPT = 101;
        const int DD_ENERGY_PROMPT_EARLY = 102; 
        const int DD_ENERGY_PROMPT_LATE = 103; 

        const int DD_TDIFF__GATEX = 105;
        const int DD_ENERGY__GATEX = 106;
        const int DD_ANGLE__GATEX = 107;

        const int DD_ENERGY__TIMEX = 120;

        const int DD_ADD_ENERGY = 150;
        //Addback related
        const int DD_TDIFF__GAMMA_GAMMA_ENERGY = 155;
        const int DD_TDIFF__GAMMA_GAMMA_ENERGY_SUM = 156;

        const int DD_ADD_ENERGY__TIMEX = 170;

        namespace betaGated {
            const int D_ENERGY = 10;
            const int D_ENERGY_PROMPT = 11;
            const int D_ENERGY_CLOVERX = 12;
            const int D_ENERGY_BETA0 = 16; 
            const int D_ENERGY_BETA1 = 17; 
            const int DD_ENERGY__BETAGAMMALOC = 18;

            const int D_ADD_ENERGY = 60; 
            const int D_ADD_ENERGY_PROMPT = 61; 
            const int D_ADD_ENERGY_CLOVERX = 65; 
            const int D_ADD_ENERGY_TOTAL = 69; 

            const int DD_ENERGY = 110; 
            const int DD_ENERGY_PROMPT = 111; 
            const int DD_ENERGY_PROMPT_EARLY = 112; 
            const int DD_ENERGY_PROMPT_LATE = 113; 
            const int DD_ENERGY_BDELAYED = 114;

            const int DD_TDIFF__GATEX = 115;
            const int DD_ENERGY__GATEX = 116;
            const int DD_ANGLE__GATEX = 117;

            const int DD_TDIFF__GAMMA_ENERGY = 118;
            const int DD_TDIFF__BETA_ENERGY = 119;

            const int DD_ENERGY__TIMEX = 130; 
            const int DD_ENERGY__TIMEX_GROW = 135; 
            const int DD_ENERGY__TIMEX_DECAY = 140; 

            const int DD_ADD_ENERGY = 160; 
            const int DD_ADD_ENERGY_PROMPT = 161; 
            const int DD_ADD_ENERGY__TIMEX = 180;
        }

        namespace multi {
            const int D_ADD_ENERGY = 52; 
            const int DD_ADD_ENERGY = 152;
            namespace betaGated {
                const int D_ADD_ENERGY = 62; 
                const int D_ADD_ENERGY_PROMPT = 63; 
                const int DD_ADD_ENERGY = 162; 
                const int DD_ADD_ENERGY_PROMPT = 163; 
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

    /** Early coincidences limit in seconds
     * (early means at the begining of the decay part of cycle, where
     * short-lived activity is present) */
    double earlyLowLimit_;
    double earlyHighLimit_;

public:
    GeProcessor(double gammaThreshold, double lowRatio,
                double highRatio, double subEventWindow,
                double gammaBetaLimit, double gammaGammaLimit,
                double earlyLowLimit, double earlyHighLimit);
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
};

#endif // __GEPROCESSOR_HPP_
