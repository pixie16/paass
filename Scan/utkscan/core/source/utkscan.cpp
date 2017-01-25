#include <exception>
#include <iostream>

// Local files
#include "Display.h"
#include "UtkScanInterface.hpp"
#include "UtkUnpacker.hpp"

// Define the name of the program.
#ifndef PROGRAM_NAME
#define PROGRAM_NAME "utkscan"
#endif

using std::cout;
using std::endl;

int main(int argc, char *argv[]){
    // Define a new unpacker object.
    cout << "utkscan.cpp : Instancing the UtkScanInterface" << endl;
    UtkScanInterface scanner;
    
    // Set the output message prefix.
    cout << "utkscan.cpp : Setting the Program Name" << endl;
    scanner.SetProgramName(std::string(PROGRAM_NAME));
    
    // Initialize the scanner.
    cout << "utkscan.cpp : Performing the setup routine" << endl;
    scanner.Setup(argc, argv);

    // Run the main loop.
    cout << "utkscan.cpp : Performing Execute method" << endl;
    int retval = 0;
    try {
        retval = scanner.Execute();
    }catch(std::exception &ex) {
        cout << Display::ErrorStr(ex.what()) << endl;
    }
    
    //Cleanup the scanning
    cout << "utkscan.cpp : Closing things out" << endl;
    scanner.Close();

    //return how things went
    return(retval);
}
