#include <iostream>

#ifdef USE_HRIBF
#include "GetArguments.hpp"
#include "Scanor.hpp"
#include "ScanorInterface.hpp"
#endif

#include "EventReaderInterface.hpp"
#include "EventReaderUnpacker.hpp"

#ifndef USE_HRIBF
int main(int argc, char *argv[]){
	// Define a new unpacker object.
    EventReaderUnpacker unpacker;
	EventReaderInterface scanner(&unpacker);
	
	// Set the output message prefix.
	scanner.SetProgramName("EventReader");
	
	// Initialize the scanner.
	if(!scanner.Setup(argc, argv, &unpacker))
		return 1;

	// Run the main loop.
	int retval = scanner.Execute();
	
	scanner.Close();
	
	return retval;
}
#else
EventReaderInterface *scanner = NULL;
EventReaderUnpacker *unpacker = NULL;


// Do some startup stuff.
extern "C" void startup_()
{
    unpacker = new EventReaderUnpacker();
	scanner = new EventReaderInterface(unpacker);

	// Handle command line arguments.
	scanner->Setup(GetNumberArguments(), GetArguments(), unpacker);
	
	// Get a pointer to a class derived from Unpacker.
	ScanorInterface::get()->SetUnpacker(unpacker);
}

///@brief Defines the main interface with the SCANOR library, the program
/// essentially starts here.
///@param [in] iexist : unused paramter from SCANOR call
extern "C" void drrsub_(uint32_t &iexist) {
	drrmake_();
	hd1d_(8000, 2, 256, 256, 0, 255, "Run DAMM you!", strlen("Run DAMM you!"));
	endrr_();
}

// Catch the exit call from scanor and clean up c++ objects CRT
extern "C" void cleanup_()
{
	// Do some cleanup.
	std::cout << "\nCleaning up..\n";
	scanner->Close();
	delete scanner;
}
#endif
