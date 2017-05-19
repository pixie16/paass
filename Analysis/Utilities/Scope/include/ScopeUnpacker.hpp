///@file ScopeUnpacker.hpp
///@brief
///@author S. V. Paulauskas
///@date May 19, 2017
#ifndef PIXIESUITE_SCOPEUNPACKER_HPP
#define PIXIESUITE_SCOPEUNPACKER_HPP

#include "ProcessedXiaData.hpp"
#include "Unpacker.hpp"
#include "VandleTimingFunction.hpp"

class TGraph;
class TH2F;
class TF1;
class TLine;
class TProfile;

class ScopeUnpacker : public Unpacker {
public:
    /// Default constructor.
    ScopeUnpacker(const unsigned int &mod = 0, const unsigned int &chan = 0);

    /// Destructor.
    ~ScopeUnpacker();

    int SetMod(const unsigned int &mod) {
        return (mod >= 0 ? (mod_ = mod) : (mod_ = 0));
    }

    int SetChan(const unsigned int &chan) {
        return (chan >= 0 ? (chan_ = chan) : (chan_ = 0));
    }

    void SetThreshLow(const int &threshLow) { threshLow_ = threshLow; }

    void SetThreshHigh(const int &threshHigh) { threshHigh_ = threshHigh; }

    unsigned int GetMod() { return mod_; }

    unsigned int GetChan() { return chan_; }

    int GetThreshLow() { return threshLow_; }

    int GetThreshHigh() { return threshHigh_; }

    void SetResetGraph(const bool &a) { resetGraph_ = a; }

    /** Clear the event deque.
  * \return Nothing.
  */
    void ClearEvents();

private:
    unsigned int mod_; ///< The module of the signal of interest.
    unsigned int chan_; ///< The channel of the signal of interest.
    int threshLow_;
    int threshHigh_;
    bool resetGraph_;
    unsigned int numAvgWaveforms_;
    unsigned int num_displayed; ///< The number of displayed traces.

    size_t numEvents; /// The number of waveforms to store.

    float cfdF_;
    int cfdD_;
    int cfdL_;
    int fitLow_;
    int fitHigh_;
    int delay_; /// The number of seconds to wait between drawing traces.

    bool need_graph_update; /// Set to true if the graph range needs updated.
    bool acqRun_;
    bool singleCapture_;
    bool init;
    bool running;
    bool performFit_;
    bool performCfd_;

    time_t last_trace; ///< The time of the last trace.
    TF1 *SetupFunc();

    TGraph *graph; ///< The TGraph for plotting traces.
    TLine *cfdLine;
    TF1 *cfdPol3;
    TF1 *cfdPol2;
    TH2F *hist; ///<The histogram containing the waveform frequencies.
    TProfile *prof; ///<The profile of the average histogram.

    TF1 *fittingFunction_;
    VandleTimingFunction *vandleTimingFunction_;

    std::vector<int> x_vals;
    std::deque<ProcessedXiaData *> chanEvents_; ///<The buffer of waveforms to be plotted.

    void ResetGraph(unsigned int size_);

    /** Process all events in the event list.
      * \param[in]  addr_ Pointer to a ScanInterface object.
      * \return Nothing.
      */
    void ProcessRawEvent();

    /** Add a channel event to the deque of events to send to the processors.
      * This method should only be called from skeletonUnpacker::ProcessRawEvent().
      * \param[in]  event_ The raw XiaData to add to the channel event deque.
      * \return True if events are ready to be processed and false otherwise.
      */
    bool AddEvent(XiaData *event_);

    /** Process all channel events read in from the rawEvent.
      * This method should only be called from skeletonUnpacker::ProcessRawEvent().
      * \return True if events were processed and false otherwise.
      */
    bool ProcessEvents();



    /// Plot the current event.
    void Plot();
};


#endif //PIXIESUITE_SCOPEUNPACKER_HPP
