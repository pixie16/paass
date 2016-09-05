#ifndef __UTKUNPACKER_HPP__
#define __UTKUNPACKER_HPP__
#include <iostream>

#include <Unpacker.hpp>

///Derived Unpacker class to handle unpacking of the data
class UtkUnpacker : public Unpacker {
public:
    /// Default constructor.
    UtkUnpacker() : Unpacker() {}
    /// Destructor.
    ~UtkUnpacker();
    
private:
    /** Process all events in the event list.
     * \param[in]  addr_ Pointer to a ScanInterface object.
     * \return Nothing. */
    virtual void ProcessRawEvent(ScanInterface *addr_=NULL);
    
    /** Add an event to generic statistics output.
     * \param[in]  event_ Pointer to the current XIA event.
     * \param[in]  addr_  Pointer to a ScanInterface object.
     * \return Nothing. */
    virtual void RawStats(XiaData *event_, ScanInterface *addr_=NULL){
	    //Does nothing for now, we will fill in things to plot later
    }
};
#endif //__UTKUNPACKER_HPP__
