#include <iostream>

// Local files
#include "ScanorInterface.hpp"
#include "UtkScanInterface.hpp"
#include "UtkUnpacker.hpp"

// Define the name of the program.
#ifndef PROGRAM_NAME
#define PROGRAM_NAME "utkscanor"
#endif

using std::cout;
using std::endl;

Unpacker *pixieUnpacker = NULL;
UtkScanInterface *scanner = NULL;

// Do some startup stuff.
extern "C" void startup_()
{
    cout << "utkscanor.cpp : Instancing the UtkScanInterface" << endl;
    scanner = new UtkScanInterface();

    // Set the output message prefix.
    cout << "utkscan.cpp : Setting the Program Name" << endl;
    scanner->SetProgramName(std::string(PROGRAM_NAME));

    // Initialize the scanner and handle command line arguments from SCANOR
    cout << "utkscan.cpp : Performing the setup routine" << endl;
    scanner->Setup(fortargc, fortargv);

    // Get a pointer to a class derived from Unpacker.
    pixieUnpacker = scanner->GetCore();
}

// Catch the exit call from scanor and clean up c++ objects CRT
extern "C" void cleanup_()
{
    // Do some cleanup.
    std::cout << "\nCleaning up..\n";
    scanner->Close();
    delete scanner;
}