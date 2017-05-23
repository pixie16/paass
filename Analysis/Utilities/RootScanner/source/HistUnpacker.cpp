#include "HistUnpacker.hpp"

#include "ScanInterface.hpp"

HistUnpacker::HistUnpacker() :
        Unpacker() {
}

void HistUnpacker::ProcessRawEvent() {
    for (auto evt = rawEvent.begin(); evt < rawEvent.end(); ++evt) {
        if (!running)
            break;

        //Store the data in a strack to be analyzied.
        AddEvent(*evt);
    }
    ProcessEvents();

    //Clear the raw events.
    ClearRawEvent();
}


///Processes a built event by filling the tree. After a certain number of
/// events the histograms are replotted. This routine clears the event
/// after filling the tree.
bool HistScanner::ProcessEvents() {
    //Get the lock for tree access.
    std::unique_lock<std::mutex> treeLock(treeMutex_);
    //Fill the tree with the current event.
    tree_->Fill();

    //We've processed the data so we clear the class for the next data.
    eventData_->Clear();

    //Check that we can make changes to histograms.
    // If not we return to handle more data while other thread is busy.
    std::unique_lock<std::mutex> lock(histMutex_, std::try_to_lock);
    if (!lock.owns_lock()) {
        return true;
    }

    static std::chrono::duration<float> timeElapsedSec;

    //Only refresh if the delay is greater than 0 or manual refresh requested.
    if (refreshDelaySec_ > 0 || refreshRequested_) {
        timeElapsedSec = std::chrono::duration_cast<std::chrono::duration<float> >(
                std::chrono::system_clock::now() - lastRefresh_);

        if (timeElapsedSec.count() > refreshDelaySec_) {
            refreshRequested_ = false;

            for (auto padItr = histos_.begin();
                 padItr != histos_.end(); ++padItr) {
                TVirtualPad *pad = padItr->first;
                HistMap_ *map = &padItr->second;
                for (auto itr = map->begin(); itr != map->end(); ++itr) {
                    Plot(itr->first, pad);
                }
                UpdateZoom(pad);
            }
            GetCanvas()->Update();
            lastRefresh_ = std::chrono::system_clock::now();
        }
    }

    return true;
}