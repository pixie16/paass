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

class GGate {
    public:
        GGate() {
            g1min = -1;
            g1max = -1;
            g2min = -1;
            g2max = -1;
        }

        bool check() {
            if (g1min > 0 && g1max > g1min &&
                g2min > 0 && g2max > g2min)
                return true;
            else
                return false;
        }

        double g1min;
        double g1max;
        double g2min;
        double g2max;
};


class GeProcessor : public EventProcessor
{
private:
    static const unsigned int chansPerClover = 4; /*!< number of channels per clover */
    
    std::map<int, int> leafToClover;   /*!< Translate a leaf location to a clover number */
    std::vector<float> timeResolution; /*!< Contatin time resolutions used */
    unsigned int numClovers;           /*!< number of clovers in map */

    double WalkCorrection(double e);
    vector<GGate> gGates;
    vector<ChanEvent*> geEvents_; /*!< Preprocessed good ge events, filled in PreProcess, removed in Process*/

    void DeclareHistogramGranY(int dammId, int xsize, int ysize, 
			       const char *title, int halfWordsPerChan,
			       const std::vector<float> &granularity, const char *units );
    void granploty(int dammId, double x, double y, const std::vector<float> &granularity);
    void symplot(int dammID, double bin1, double bin2);

public:
    GeProcessor(); // no virtual c'tors
    virtual bool Init(DetectorDriver &driver);
    virtual bool PreProcess(RawEvent &event);
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void);
};

/**
 * A class to hold all events which happen within a given clover
 *   maintaing a record of the total energy deposited
 *   NOT CURRENTLY USED as we need to have an associated TIME as well
 */
class CloverEvent : public EventContainer
{
  private:
    double totalEnergy;
  public:
    CloverEvent();
    virtual void push_back(const ChanEvent* &x);
    virtual void clear(void);
};

#endif // __GEPROCESSOR_HPP_
