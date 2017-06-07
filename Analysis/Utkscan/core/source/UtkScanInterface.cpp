///@file UtkScanInterface.cpp
///@brief Derived class handling the interface with utkscan.
///@author S. V. Paulauskas
///@date September 23, 2016
#include <iostream>
#include <stdexcept>

#include "DetectorDriver.hpp"
#include "Display.h"
#include "TreeCorrelator.hpp"
#include "UtkScanInterface.hpp"
#include "UtkUnpacker.hpp"
#include "XmlInterface.hpp"

using namespace std;

// Define a pointer to an OutputHisFile for later use.
#ifndef USE_HRIBF
OutputHisFile *output_his = NULL;
#endif

/// Default constructor.
UtkScanInterface::UtkScanInterface() : ScanInterface() {
    init_ = false;
}

/// Destructor.
UtkScanInterface::~UtkScanInterface() {
#ifndef USE_HRIBF
    if (init_)
        delete (output_his);
#endif
}

/** ExtraCommands is used to send command strings to classes derived
 * from ScanInterface. If ScanInterface receives an unrecognized
 * command from the user, it will pass it on to the derived class.
 * \param[in]  cmd_ The command to interpret.
 * \param[out] arg_ Vector or arguments to the user command.
 * \return True if the command was recognized and false otherwise. */
bool UtkScanInterface::ExtraCommands(const string &cmd_,
                                     vector<string> &args_) {
    if (cmd_ == "mycmd") {
        if (args_.size() >= 1) {
            // Handle the command.
        } else {
            cout << msgHeader
                      << "Invalid number of parameters to 'mycmd'\n";
            cout << msgHeader << " -SYNTAX- mycmd <param>\n";
        }
    } else
        return (false); // Unrecognized command.

    return (true);
}

/** CmdHelp is used to allow a derived class to print a help statement about
 * its own commands. This method is called whenever the user enters 'help'
 * or 'h' into the interactive terminal (if available).
 * \param[in]  prefix_ String to append at the start of any output. */
void UtkScanInterface::CmdHelp() {
    cout << "   mycmd <param> - Do something useful.\n";
}

/** SyntaxStr is used to print a linux style usage message to the screen.
 * \param[in]  name_ The name of the program.
 * \return Nothing. */
void UtkScanInterface::SyntaxStr(char *name_) {
    cout << " usage: " << string(name_) << " [input] [options]\n";
}

/** Initialize the map file, the config file, the processor handler, 
 * and add all of the required processors.
 * \param[in]  prefix_ String to append to the beginning of system output.
 * \return True upon successfully initializing and false otherwise. */
bool UtkScanInterface::Initialize(string prefix_) {
    if (init_)
        return false;

    if(GetOutputFilename() == "")
        throw invalid_argument("UtkScaninterface::Initialize : The output file "
                                       "name was not provided.");

    try {
        cout << "UtkScanInterface::Initialize : Now attempting to load and "
                "parse " << GetSetupFilename() << endl;
        XmlInterface::get(GetSetupFilename());
        Globals::get(GetSetupFilename());
        DetectorLibrary::get();
        TreeCorrelator::get()->buildTree();
    }catch (invalid_argument &ex) {
        throw;
    }

    Globals::get()->SetOutputFilename(GetOutputFilename());
    Globals::get()->SetOutputPath(GetOutputPath());
    //set eventWidth from Globals (loaded from config file)
    GetCore()->SetEventWidth(Globals::get()->GetEventLengthInTicks());

    //We remove this whole block in the event that we are using the SCANOR
    //This should be cleaned up!!
#ifndef USE_HRIBF
    try {
        output_his =
                new OutputHisFile(
                        (GetOutputPath()+GetOutputFilename()).c_str());
        output_his->SetDebugMode(false);

        /** The DetectorDriver constructor will load processors
         *  from the xml configuration file upon first call.
         *  The DeclarePlots function will instantiate the DetectorLibrary
         *  class which will read in the "map" of channels.
         *  Subsequently the raw histograms, the diagnostic histograms
         *  and the processors and analyzers plots are declared.
         *
         *  Note that in the PixieStd the Init function of DetectorDriver
         *  is called upon first buffer. This include reading in the
         *  calibration and walk correction factors.
         */
        DetectorDriver::get()->DeclarePlots();
        output_his->Finalize();
    } catch (exception &e) {
        cout << Display::ErrorStr(prefix_ + "Exception caught at UtkScanInterface::Initialize")
                  << endl;
        throw;
    }
#endif
    return (init_ = true);
}

/** Peform any last minute initialization before processing data. 
 * /return Nothing. */
void UtkScanInterface::FinalInitialization() {
    // Do some last minute initialization before the run starts.
}

/** Receive various status notifications from the scan.
 * \param[in] code_ The notification code passed from ScanInterface methods.
 * \return Nothing. */
void UtkScanInterface::Notify(const string &code_/*=""*/) {
    if (code_ == "START_SCAN") {
    } else if (code_ == "STOP_SCAN") {
    } else if (code_ == "SCAN_COMPLETE") {
        cout << msgHeader << "Scan complete.\n";
    } else if (code_ == "LOAD_FILE") {
        cout << msgHeader << "File loaded.\n";
    } else if (code_ == "REWIND_FILE") {
    } else {
        cout << msgHeader << "Unknown notification code '" << code_
                  << "'!\n";
    }
}

/** Return a pointer to the Unpacker object to use for data unpacking.
 * If no object has been initialized, create a new one.
 * \return Pointer to an Unpacker object. */
Unpacker *UtkScanInterface::GetCore() {
    if (!core)
        core = (Unpacker *) (new UtkUnpacker());
    return (core);
}

