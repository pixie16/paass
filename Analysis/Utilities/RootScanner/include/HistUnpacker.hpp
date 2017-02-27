#ifndef HISTUNPACKER_H
#define HISTUNPACKER_H

#include "Unpacker.hpp"

class ScanInterface;

class HistUnpacker : public Unpacker {
	public:
		HistUnpacker();

		/// Process all events in the event list.
		/// \param[in]  addr_ Pointer to a location in memory. 
		/// \return Nothing.
		///
		virtual void ProcessRawEvent(ScanInterface *addr_=NULL);


};



#endif //HISTUNPACKER_H
