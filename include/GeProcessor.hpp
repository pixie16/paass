/*! \file GeProcessor.hpp
 *
 * Processor for germanium detectors
 */

#ifndef __GEPROCESSOR_HPP_
#define __GEPROCESSOR_HPP_

#include <map>
#include <vector>

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
        const int D_ENERGY_CLOVERX = 1; 

        const int D_ENERGY_LOWGAIN = 7;
        const int D_ENERGY_HIGHGAIN = 8;
        const int D_MULT = 9;

        const int D_ADD_ENERGY = 50; 
        const int D_ADD_ENERGY_CLOVERX = 51;
        const int D_ADD_ENERGY_TOTAL = 55;

        // 2D spectra
        const int DD_ENERGY = 100;
        const int DD_ENERGY_PROMPT = 101;
        const int DD_TDIFF__GAMMA_GAMMA_ENERGY = 102;
        const int DD_TDIFF__GAMMA_GAMMA_ENERGY_SUM = 103;
        const int DD_CLOVER_ENERGY_RATIO = 107;
        const int DD_ADD_ENERGY = 150;

        const int DD_ANGLE__GATEX = 155;
        const int DD_ENERGY__GATEX = 156;
        const int DD_TDIFF__GATEX = 157;

        const int DD_ENERGY__TIMEX = 120;
        const int DD_ADD_ENERGY__TIMEX = 170;

        namespace betaGated {
            const int D_ENERGY = 10;
            const int D_ENERGY_CLOVERX = 11;
            const int D_ENERGY_BETA0 = 15; 
            const int D_ENERGY_BETA1 = 16; 
            const int D_ADD_ENERGY = 60; 
            const int D_ADD_ENERGY_CLOVERX = 61; 
            const int D_ADD_ENERGY_TOTAL = 65; 

            const int DD_TDIFF__GAMMA_ENERGY = 105;
            const int DD_TDIFF__BETA_ENERGY = 106;
            const int DD_ENERGY = 110; 
            const int DD_ENERGY_PROMPT = 111; 
            const int DD_ADD_ENERGY = 160; 
            const int DD_ADD_ENERGY_DELAYED = 163;
            const int DD_ANGLE__GATEX = 164; 
            const int DD_ENERGY__GATEX = 165; 
            const int DD_ENERGY__TIMEX = 130; 
            const int DD_ENERGY__TIMEX_GROW = 135; 
            const int DD_ENERGY__TIMEX_DECAY = 140; 
            const int DD_ADD_ENERGY__TIMEX = 180;
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

/** This class is to compare gamma - beta time differences in function
 * GeProcessor::GammaBetaDtime(). */
class CompareTimes {
    public:
        bool operator()(double first, double second) {
            return abs(first) < abs(second);
        }
};

class GeProcessor : public EventProcessor
{
protected:
    static const unsigned int chansPerClover = 4; /*!< number of channels per clover */
    
    std::map<int, int> leafToClover;   /*!< Translate a leaf location to a clover number */
    std::vector<float> timeResolution; /*!< Contatin time resolutions used */
    unsigned int numClovers;           /*!< number of clovers in map */

    double WalkCorrection(double e);

    /** Returns lowest difference between gamma and beta times. Takes gTime in pixie clock units.
     * returns value in seconds. */
    double GammaBetaDtime(double gTime);

    /** Returns true if gamma-beta correlation time within good limits.
     * Browses through all beta events in Beta correlation place to find
     * the lowest difference. Takes gTime in pixie clock, limit in
     * seconds. */
    bool GoodGammaBeta(double gTime, 
                       double limit_in_sec = detectors::gammaBetaLimit);

    std::vector<ChanEvent*> geEvents_; /*!< Preprocessed good ge events, filled in PreProcess*/

    void DeclareHistogramGranY(int dammId, int xsize, int ysize, 
			       const char *title, int halfWordsPerChan,
			       const std::vector<float> &granularity, const char *units );
    void granploty(int dammId, double x, double y, 
                   const std::vector<float> &granularity);
    void symplot(int dammID, double bin1, double bin2);

    /** addbackEvents vector is arranged as:
     * pair, first-> energy, second->time
     */
    std::vector< std::vector< std::pair<double, double> > > addbackEvents_;
    /** tas vector for total energy absorbed, same structure as addback */
    std::vector< std::pair<double, double> > tas_;
#ifdef GGATES
    std::vector< std::vector<LineGate> > gGates;
#endif
public:
    GeProcessor(); // no virtual c'tors
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
};

#endif // __GEPROCESSOR_HPP_
