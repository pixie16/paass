#include <iostream>

// Local files
#include "UtkScanInterface.hpp"
#include "UtkUnpacker.hpp"

// Define the name of the program.
#if not defined(PROG_NAME)
#define PROG_NAME "utkscan"
#endif

int main(int argc, char *argv[]){
    // Define a new unpacker object.
    UtkScanInterface scanner;
    
    // Set the output message prefix.
    scanner.SetProgramName(std::string(PROG_NAME));	
    
    // Initialize the scanner.
    scanner.Setup(argc, argv);
    
    // Run the main loop.
    int retval = scanner.Execute();
    
    //Cleanup the scanning
    scanner.Close();
    
    //return how things went
    return(retval);
}
