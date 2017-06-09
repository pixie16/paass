///@author S. V. Paulauskas
#include <exception>
#include <iostream>
#include <stdexcept>

// Local files
#include "Display.h"
#include "UtkScanInterface.hpp"
#include "UtkUnpacker.hpp"

// Define the name of the program.
#ifndef PROGRAM_NAME
#define PROGRAM_NAME "utkscan"
#endif

using namespace std;

int main(int argc, char *argv[]) {
    // Define the unpacker and scan objects.
    cout << "utkscan.cpp : Instancing the UtkScanInterface" << endl;
    UtkScanInterface scanner;
    cout << "utkscan.cpp : Instancing the UtkUnpacker" << endl;
    UtkUnpacker unpacker;

    // Set the output message prefix.
    cout << "utkscan.cpp : Setting the Program Name" << endl;
    scanner.SetProgramName(std::string(PROGRAM_NAME));

    // Initialize the scanner.
    cout << "utkscan.cpp : Performing the setup routine" << endl;
    try {
        scanner.Setup(argc, argv, &unpacker);
    } catch(invalid_argument &invalidArgument){
        cout << invalidArgument.what() << endl;
        return 1;
    }

    try {
        // Run the main loop.
        cout << "utkscan.cpp : Performing Execute method" << endl;
        scanner.Execute();
    } catch (std::exception &ex) {
        cout << Display::ErrorStr(ex.what()) << endl;
    }

    //Cleanup the scanning
    cout << "utkscan.cpp : Closing things out" << endl;
    scanner.Close();

    //return how things went
    return 0;
}
