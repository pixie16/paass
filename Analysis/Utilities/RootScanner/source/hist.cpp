#include "HistScanner.hpp"

int main(int argc, char *argv[]){
	// Define a new unpacker object.
	HistScanner scanner;
	
	// Set the output message prefix.
	scanner.SetProgramName("hist");	
	
	// Initialize the scanner.
	if(!scanner.Setup(argc, argv))
		return 1;

	// Run the main loop.
	int retval = scanner.Execute();
	
	scanner.Close();
	
	return retval;
}
