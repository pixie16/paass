///@file EventReaderUnpacker.cpp
///@brief Derived class to handle unpacking of events for EventReader
///@authors S. V. Paulauskas and C. R. Thornsberry
///@date June 8, 2017

#include <iostream>

#include "unistd.h"

#include "EventReaderUnpacker.hpp"

/** Process all events in the event list.
  * \param[in]  addr_ Pointer to a location in memory.
  * \return Nothing.
  */
void EventReaderUnpacker::ProcessRawEvent(){
    XiaData *current_event = NULL;

    // Fill the processor event deques with events
    while(!rawEvent.empty()){
        if(!running) break;

        current_event = rawEvent.front();
        rawEvent.pop_front(); // Remove this event from the raw event deque.

#ifdef USE_HRIBF
        // If using scanor, output to the generic histogram so we know that something is happening.
		count1cc_(8000, (current_event->modNum*16+current_event->chanNum), 1);
#endif

        // Check that this channel event exists.
        if(!current_event)
            continue;

        // Send the event to the scan interface object for processing.
        AddEvent(current_event);
    }
}

void EventReaderUnpacker::displayBool(const char *msg_, const bool &val_){
    if(val_) std::cout << msg_ << "YES\n";
    else std::cout << msg_ << "NO\n";
}

/** Add a channel event to the deque of events to send to the processors.
  * This method should only be called from readerUnpacker::ProcessRawEvent().
  * \param[in]  event_ The raw XiaData to add to the channel event deque.
  * \return False.
  */
bool EventReaderUnpacker::AddEvent(XiaData *event_){
    if(!event_)
        return false;

    if(numSkip_ == 0){
        std::cout << "*************************************************\n";
        std::cout << "** Raw Event no. " << eventsRead_ << std::endl;
        std::cout << "*************************************************\n";
        std::cout << " Filter Energy: " << event_->GetEnergy() << std::endl;
        std::cout << " Trigger Time:  " << (unsigned long long)event_->GetTime() << std::endl;
        std::cout << " Module:        " << event_->GetModuleNumber() << std::endl;
        std::cout << " Channel:       " << event_->GetChannelNumber() << std::endl;
        std::cout << " CFD Time:      " << event_->GetCfdFractionalTime() << std::endl;
        std::cout << " Trace Length:  " << event_->GetTrace().size() << std::endl;

        if(showFlags_){
            displayBool(" Virtual:       ", event_->IsVirtualChannel());
            displayBool(" Pileup:        ", event_->IsPileup());
            displayBool(" Saturated:     ", event_->IsSaturated());
            displayBool(" CFD Force:     ", event_->GetCfdForcedTriggerBit());
            displayBool(" CFD Trig:      ", event_->GetCfdTriggerSourceBit());
        }

        if(showTrace_ && !event_->GetTrace().empty()){
            int numLine = 0;
            std::cout << " Trace:\n  ";
            for(size_t i = 0; i < event_->GetTrace().size(); i++){
                std::cout << event_->GetTrace().at(i) << "\t";
                if(++numLine % 10 == 0) std::cout << "\n  ";
            }
            std::cout << std::endl;
        }

        std::cout << std::endl;

        ///@TODO This needs to be replaced with a more C++ compatible version. See here
        /// https://stackoverflow.com/questions/4184468/sleep-for-milliseconds#10613664
        sleep(1);
    }
    else{ numSkip_--; }

    eventsRead_++;
    delete event_;

    return false;
}