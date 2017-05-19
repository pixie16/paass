///@file Skeleton.cpp
///@brief The main program for the Skeleton example analysis program
///@authors C. R. Thornsberry and S. V. Paulauskas
///@date May 14, 2017
#include <exception>
#include <iostream>

#include <cstring>

#include "XiaData.hpp"

#include "SkeletonInterface.hpp"
#include "SkeletonUnpacker.hpp"

#ifdef USE_HRIBF
#include "GetArguments.hpp"
#include "Scanor.hpp"
#include "ScanorInterface.hpp"
#endif

// Define the name of the program.
#ifndef PROG_NAME
#define PROG_NAME "Spooky"
#endif

#ifndef USE_HRIBF

using namespace std;

int main(int argc, char *argv[]) {
    // Define a new unpacker object.
    SkeletonUnpacker unpacker;
    SkeletonInterface scanner;

    try {
        // Set the output message prefix.
        scanner.SetProgramName(std::string(PROG_NAME));

        // Initialize the scanner.
        scanner.Setup(argc, argv, &unpacker);
    } catch (invalid_argument &invalidArgument) {
        cout << invalidArgument.what() << endl;
        return 1;
    }

    // Run the main loop.
    int retval = scanner.Execute();

    scanner.Close();

    return retval;
}

#else
SkeletonInterface *scanner = NULL;

extern "C" void startup_() {
    scanner = new SkeletonInterface();

    // Handle command line arguments.
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
extern "C" void cleanup_() {
    // Do some cleanup.
    std::cout << "\nCleaning up..\n";
    scanner->Close();
    delete scanner;
}
#endif