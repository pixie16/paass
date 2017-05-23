#ifndef HISTUNPACKER_H
#define HISTUNPACKER_H

#include "Unpacker.hpp"

class ScanInterface;

class HistUnpacker : public Unpacker {
public:
    HistUnpacker();

private:
    /// Process all events in the event list.
    /// \param[in]  addr_ Pointer to a location in memory.
    /// \return Nothing.
    ///
    void ProcessRawEvent(ScanInterface *addr_ = NULL);

    /// @brief Create a HistScannerChanData from the provided XiaData.
    /// @param[in] event An XIA event to process.
    bool AddEvent(XiaData *event);
};


#endif //HISTUNPACKER_H
