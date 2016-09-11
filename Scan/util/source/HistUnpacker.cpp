#include "HistUnpacker.hpp"

#include "ScanInterface.hpp"

HistUnpacker::HistUnpacker() :
	Unpacker()
{

}

void HistUnpacker::ProcessRawEvent(ScanInterface *interface) {
	if(!interface){ return; }

	for (auto evt=rawEvent.begin(); evt < rawEvent.end(); ++evt) {
		if(!running)
			break;

		//Store the data in a strack to be analyzied.
		interface->AddEvent(*evt);
	}
	interface->ProcessEvents();
	
	//Clear the raw events.
	ClearRawEvent();
}


