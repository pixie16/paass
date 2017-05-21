///@file ScopeScanner.cpp
///@brief
///@author C. R. Thornsberry and S. V. Paulauskas
///@date May 19, 2017
#include <iostream>
#include <string>

#include "ScopeUnpacker.hpp"
#include "ScopeScanner.hpp"

// Define the name of the program.
#ifndef PROG_NAME
#define PROG_NAME "Scope"
#endif

#ifndef USE_HRIBF
int main(int argc, char *argv[]) {
    // Define a new unpacker object.
    ScopeUnpacker unpacker;
    ScopeScanner scanner(&unpacker);

    // Set the output message prefix.
    scanner.SetProgramName(std::string(PROG_NAME));

    unpacker.SetCanvas(scanner.GetCanvas());

    // Initialize the scanner.
    if (!scanner.Setup(argc, argv, &unpacker))
        return 1;

    // Run the main loop.
    int retval = scanner.Execute();

    scanner.Close();

    return retval;
}
#else
#include "GetArguments.hpp"
#include "Scanor.hpp"
#include "ScanorInterface.hpp"
ScopeScanner *scanner = NULL;

// Do some startup stuff.
extern "C" void startup_()
{
       scanner = new ScopeScanner();   

       // Handle command line arguments from SCANOR
       scanner->Setup(GetNumberArguments(), GetArguments());
       
       // Get a pointer to a class derived from Unpacker.
       ScanorInterface::get()->SetUnpacker(scanner->GetCore());
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

