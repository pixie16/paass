///@file SkeletonUnpacker.cpp
///@brief
///@author C. R. Thornsberry, S. V. Paulauskas
///@date May 14, 2017

#include "SkeletonUnpacker.hpp"
#include "XiaData.hpp"

#ifdef USE_HRIBF
#include "Scanor.hpp"
#endif


///Process all events in the event list.
void SkeletonUnpacker::ProcessRawEvent() {
    XiaData *current_event = NULL;

    // Fill the processor event deques with events
    while (!rawEvent.empty()) {
        current_event = rawEvent.front();
        rawEvent.pop_front(); // Remove this event from the raw event deque.

#ifdef USE_HRIBF
        // If using scanor, output to the generic histogram so we know that something is happening.
        count1cc_(8000, (current_event->GetId()), 1);
#endif

        // Check that this channel event exists.
        if (!current_event)
            continue;

        //Adds the event to some sort of tracking.
        AddEvent(current_event);
    }
    //Process the event now that the event deque is built
    ProcessEvents();
}

/** Add a channel event to the deque of events to send to the processors.
  * This method should only be called from skeletonUnpacker::ProcessRawEvent().
  * \param[in]  event The raw XiaData to add to the channel event deque.
  * \return False if the event pointer was invalid.
  */
bool SkeletonUnpacker::AddEvent(XiaData *event) {
    if (!event)
        return false;

    // Handle the individual XiaData. Maybe add it to a detector's event list or something.
    // Do nothing with it for now.
    delete event;

    return false;
}

///Process all channel events read in from the rawEvent.
///@return False.
bool SkeletonUnpacker::ProcessEvents() {
    // Really we're going to do nothing for processing.
    return false;
}