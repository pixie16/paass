/// @authors K. Smith, S. V. Paulauskas

#ifndef HISTUNPACKER_H
#define HISTUNPACKER_H
#include <chrono>
#include <vector>
#include <map>
#include <tuple>
#include <chrono>
#include <mutex>

#include <TTree.h>
#include <TFile.h>
#include <TVirtualPad.h>

#include "Unpacker.hpp"
#include "HistScannerChanData.hpp"

class HistUnpacker : public Unpacker {
public:
    HistUnpacker();

    ~HistUnpacker();

    void SetLastRefresh(const std::chrono::system_clock::time_point &a) { lastRefresh_ = a; }

    void PlotCommand(const std::vector<std::string> &args);

    void RefreshCommand(const std::vector<std::string> &args);

    void ZeroCommand(const std::vector<std::string> &args);

    void ClearCommand(const std::vector<std::string> &args);

    void DivideCommand(const std::vector<std::string> &args);

    void IdleTask();

private:


    TFile *file_; //<ROOT file containing the tree.
    TTree *tree_; //<Tree containing the current plottable data.

    /// Vector containing all the channel data for an event.
    HistScannerChanData *eventData_;

    /// The type for key for the hist map.
    typedef std::tuple<std::string, std::string> HistKey_;
    /// The type for the histogram map.
    typedef std::map<HistKey_, std::string> HistMap_;

    ///A vector of new histograms, containing a HistKey_ and a TVirtualPad*.
    std::vector<std::pair<HistKey_, TVirtualPad *> > newHists_;
    ///A map of plotted histograms.
    std::map<TVirtualPad *, HistMap_> histos_;
    ///A map whose value is the number of times a histogram key was requested
    /// for plotting.
    std::map<HistKey_, int> histCount_;
    ///The number of TTree entries that had been scanned to create the
    /// corresponding histogram.
    std::map<TH1 *, long> treeEntries_;

    std::mutex histMutex_;
    std::mutex treeMutex_;

    ///@brief Initialize and plot newly requested plots.
    void ProcessNewHists();

    ///@brief Plot the histogram corresponding to the provided key.
    ///@param[in] key The key of the histogram to plot.
    ///@param[in] par The corresponding pad that the histogram should be
    /// plotted on
    void Plot(HistKey_ key, TVirtualPad *pad = gPad);

    float refreshDelaySec_;
    bool refreshRequested_;
    std::chrono::system_clock::time_point lastRefresh_;

    /// Process all events in the event list.
    /// \param[in]  addr_ Pointer to a location in memory.
    /// \return Nothing.
    ///
    void ProcessRawEvent();

    /// @brief Create a HistScannerChanData from the provided XiaData.
    /// @param[in] event An XIA event to process.
    bool AddEvent(XiaData *event);

    /// @brief Processes each event by filling the tree and clearing the event.
    bool ProcessEvents();
};


#endif //HISTUNPACKER_H
