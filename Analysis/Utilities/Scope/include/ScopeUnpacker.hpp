///@file ScopeUnpacker.hpp
///@brief
///@author C. R. Thornsberry, S. V. Paulauskas
///@date May 19, 2017
#ifndef PIXIESUITE_SCOPEUNPACKER_HPP
#define PIXIESUITE_SCOPEUNPACKER_HPP

#include "CrystalBallFunction.hpp"
#include "CsiFunction.hpp"
#include "EmCalTimingFunction.hpp"
#include "ProcessedXiaData.hpp"
#include "SiPmtFastTimingFunction.hpp"
#include "Unpacker.hpp"
#include "VandleTimingFunction.hpp"

class TGraph;

class TH2F;

class TF1;

class TLine;

class TProfile;

class TCanvas;

///Class that handles unpacking data and process for use with scope.
class ScopeUnpacker : public Unpacker {
public:
    /// Default constructor.
    ScopeUnpacker(const unsigned int &mod = 0, const unsigned int &chan = 0);

    /// Destructor.
    ~ScopeUnpacker();

    unsigned int GetModuleNumber() { return mod_; }

    unsigned int GetChannelNumber() { return chan_; }

    int GetThreshLow() { return threshLow_; }

    int GetThreshHigh() { return threshHigh_; }

    bool PerformCfd() { return performCfd_; }

    bool PerformFit() { return performFit_; }

    void SetCfdFraction(const double &a) { cfdF_ = a; }

    void SetCfdDelay(const unsigned int &a) { cfdD_ = a; }

    void SetCfdShift(const unsigned int &a) { cfdL_ = a; }

    void SetChannelNumber(const unsigned int &a) { chan_ = a; }

    void SetDelayInSeconds(const unsigned int &a) { delayInSeconds_ = a; }

    void SetFitLow(const unsigned int &a) { fitLow_ = a; }

    void SetFitHigh(const unsigned int &a) { fitHigh_ = a; }

    void SetModuleNumber(const unsigned int &a) { mod_ = a; }

    void SetNumEvents(size_t num_) { numEvents_ = num_; }

    void SetNumberTracesToAverage(const unsigned int &a) { numAvgWaveforms_ = a; }

    void SetPerformCfd(const bool &a) { performCfd_ = a; }

    void SetPerformFit(const bool &a) { performFit_ = a; }

    void SetResetGraph(const bool &a) { resetGraph_ = a; }

    void SetSaveFile(const std::string &a) { saveFile_ = a; }

    void SetThreshLow(const int &a) { threshLow_ = a; }

    void SetThreshHigh(const int &a) { threshHigh_ = a; }

    bool SelectFittingFunction(const std::string &func);

    void ClearEvents();

private:
    unsigned int mod_; ///< The module of the signal of interest.
    unsigned int chan_; ///< The channel of the signal of interest.
    unsigned int threshLow_;
    unsigned int threshHigh_;
    unsigned int numAvgWaveforms_;
    unsigned int delayInSeconds_; /// The number of seconds to wait between drawing traces.
    unsigned int numTracesDisplayed_; ///< The number of displayed traces.
    unsigned int numEvents_; /// The number of waveforms to store.

    //Parameters for the CFD
    bool performCfd_;
    double cfdF_;
    int cfdD_;
    int cfdL_;

    std::string saveFile_;

    //Parameters for the Fitting
    bool performFit_;
    int fitLow_;
    int fitHigh_;

    bool resetGraph_;
    bool need_graph_update; /// Set to true if the graph range needs updated.
    bool acqRun_;
    bool singleCapture_;
    bool init;

    time_t last_trace; ///< The time of the last trace.

    TGraph *graph; ///< The TGraph for plotting traces.
    TLine *cfdLine;
    TF1 *cfdPol3;
    TF1 *cfdPol2;
    TH2F *hist; ///<The histogram containing the waveform frequencies.
    TProfile *prof; ///<The profile of the average histogram.

    TF1 *fittingFunction_;
    CrystalBallFunction *crystalBallFunction_;
    CsiFunction *csiFunction_;
    EmCalTimingFunction *emCalTimingFunction_;
    SiPmtFastTimingFunction *siPmtFastTimingFunction_;
    VandleTimingFunction *vandleTimingFunction_;

    std::vector<int> x_vals;
    std::deque<ProcessedXiaData *> chanEvents_; ///<The buffer of waveforms to be plotted.

    void ResetGraph(const unsigned int &size);

    /** Process all events in the event list.
      * \param[in]  addr_ Pointer to a ScanInterface object.
      * \return Nothing.
      */
    void ProcessRawEvent();

    /** Process all channel events read in from the rawEvent.
      * This method should only be called from skeletonUnpacker::ProcessRawEvent().
      * \return True if events were processed and false otherwise.
      */
    bool ProcessEvents();

    /// Plot the current event.
    void Plot();
};


#endif //PIXIESUITE_SCOPEUNPACKER_HPP
