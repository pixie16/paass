///@file EventReaderUnpacker.hpp
///@brief Derived class to handle unpacking of events for EventReader
///@authors S. V. Paulauskas and C. R. Thornsberry
///@date June 8, 2017

#ifndef PAASS_EVENTREADERUNPACKER_HPP
#define PAASS_EVENTREADERUNPACKER_HPP

#include "Unpacker.hpp"
#include "XiaData.hpp"

class EventReaderUnpacker : public Unpacker {
public:
    /// Default constructor.
    EventReaderUnpacker() : Unpacker() {}

    /// Destructor.
    ~EventReaderUnpacker() {}

    unsigned int GetNumberToSkip() const { return numSkip_; }

    bool GetShowFlags() const { return showFlags_; }

    bool GetShowTrace() const { return showTrace_; }

    void SetNumberToSkip(const unsigned int &a) { numSkip_ = a; }

    void SetShowFlags(const bool &a) { showFlags_ = a; }

    void SetShowTrace(const bool &a) { showTrace_ = a; }

private:
    /** Process all events in the event list.
      * \param[in]  addr_ Pointer to a ScanInterface object.
      * \return Nothing.
      */
    void ProcessRawEvent();

    /** Add a channel event to the deque of events to send to the processors.
  * This method should only be called from EventReaderUnpacker::ProcessRawEvent().
  * \param[in]  event_ The raw XiaData to add to the channel event deque.
  * \return False.
  */
    bool AddEvent(XiaData *event_);

    void displayBool(const char *msg_, const bool &val_);

    unsigned int numSkip_;
    unsigned int eventsRead_;
    bool showFlags_;
    bool showTrace_;
};


#endif //PAASS_EVENTREADERUNPACKER_HPP
