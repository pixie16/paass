#ifndef HISTUNPACKER_H
#define HISTUNPACKER_H

// PixieCore libraries
#include "Unpacker.hpp"
#include "ScanInterface.hpp"

class histUnpacker : public Unpacker {
	public:
		histUnpacker();

		/// Process all events in the event list.
		/// \param[in]  addr_ Pointer to a location in memory. 
		/// \return Nothing.
		///
		virtual void ProcessRawEvent(ScanInterface *addr_=NULL);


};



#endif //HISTUNPACKER_H
