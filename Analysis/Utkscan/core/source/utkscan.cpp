#include <exception>
#include <iostream>
#include <stdexcept>

// Local files
#include "Display.h"
#include "UtkScanInterface.hpp"
#include "UtkUnpacker.hpp"
#include "XmlInterface.hpp"
#include "XmlParser.hpp"

// Define the name of the program.
#ifndef PROGRAM_NAME
#define PROGRAM_NAME "utkscan"
#endif

using namespace std;

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

    try {
        ///@TODO This needs to be updated so that the Globals class is not
        /// initialized before this. Otherwise this error handling that we're
        /// intending will not work properly.
        cout << "utkscan.cpp : Initial error checking of Configuration file"
             << endl;
        //Do some initial parsing of the configuration file:
        XmlInterface::get(scanner.GetSetupFilename());
        XmlParser xmlParser;
        xmlParser.ParseRootNode(XmlInterface::get()->GetDocument()->child("Configuration"));

        // Run the main loop.
        cout << "utkscan.cpp : Performing Execute method" << endl;
        scanner.Execute();

    } catch(std::exception &ex) {
        cout << Display::ErrorStr(ex.what()) << endl;
    }

    //Cleanup the scanning
    cout << "utkscan.cpp : Closing things out" << endl;
    scanner.Close();

    //return how things went
    return 0;
}
