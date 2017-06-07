#include <iostream>

#include <cstring>

// Local files
#include "DetectorDriver.hpp"
#include "GetArguments.hpp"
#include "ScanorInterface.hpp"
#include "UtkScanInterface.hpp"

// Define the name of the program.
#ifndef PROGRAM_NAME
#define PROGRAM_NAME "utkscanor"
#endif

using std::cout;
using std::endl;

UtkScanInterface *scanner = NULL;

///@brief Begins setups the interface between SCANOR and the C++ and the
/// Unpacker. It also handles the processing of command line arguments.
extern "C" void startup_() {
    cout << "utkscanor.cpp : Instancing the UtkScanInterface" << endl;
    scanner = new UtkScanInterface();

    // Set the output message prefix.
    cout << "utkscan.cpp : Setting the Program Name" << endl;
    scanner->SetProgramName(std::string(PROGRAM_NAME));

    // Initialize the scanner and handle command line arguments from SCANOR
    cout << "utkscan.cpp : Performing the setup routine" << endl;
    scanner->Setup(GetNumberArguments(), GetArguments());

    //Set the unpacker object based off what the ScanInterface object created
    ScanorInterface::get()->SetUnpacker(scanner->GetCore());
}

///@brief Defines the main interface with the SCANOR library, the program
/// essentially starts here.
///@param [in] iexist : unused paramter from SCANOR call
extern "C" void drrsub_(uint32_t &iexist) {
    try {
        drrmake_();
        DetectorDriver::get()->DeclarePlots();
        endrr_();
    } catch (std::exception &e) {
        // Any exceptions will be intercepted here
        cout << "Exception caught at Initialize:" << endl;
        cout << "\t" << e.what() << endl;
    }
}

// Catch the exit call from scanor and clean up c++ objects CRT
extern "C" void cleanup_() {
    // Do some cleanup.
    cout << "\nCleaning up..\n";
    scanner->Close();
    delete scanner;
}