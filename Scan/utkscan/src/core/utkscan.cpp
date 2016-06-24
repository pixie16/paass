#include <iostream>

// Local files
#include "UtkScanInterface.hpp"
#include "UtkUnpacker.hpp"

// Define the name of the program.
#if not defined(PROG_NAME)
#define PROG_NAME "utkscan"
#endif

using std::cout;
using std::endl;

int main(int argc, char *argv[]){
    // Define a new unpacker object.
    cout << "utkscan.cpp : Instancing the UtkScanInterface" << endl;
    UtkScanInterface scanner;
    
    // Set the output message prefix.
    cout << "utkscan.cpp : Setting the Program Name" << endl;
    scanner.SetProgramName(std::string(PROG_NAME));	
    
    // Initialize the scanner.
    cout << "utkscan.cpp : Performing the setup routine" << endl;
    scanner.Setup(argc, argv);

    // Run the main loop.
    cout << "utkscan.cpp : Performing Execute method" << endl;
    int retval = scanner.Execute();
    
    //Cleanup the scanning
    cout << "utkscan.cpp : Closing things out" << endl;
    scanner.Close();

    //return how things went
    return(retval);
}
