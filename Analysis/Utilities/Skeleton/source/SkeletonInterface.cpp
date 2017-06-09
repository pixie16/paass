///@file SkeletonInterface.cpp
///@brief
///@author C. R. Thornsberry, S. V. Paulauskas
///@date May 14, 2017
#include <iostream>

#include "SkeletonInterface.hpp"
#include "XiaData.hpp"

using namespace std;

SkeletonInterface::SkeletonInterface() : ScanInterface() {
    init = false;

    auxillaryKnownArgumentMap_.insert(make_pair("mycmd1", "A useful terminal command."));
    auxillaryKnownArgumentMap_.insert(make_pair("mycmd2",
                                                "Usage : mycmd2 [param] | A useful terminal command with an optional argument."));
    auxillaryKnownArgumentMap_.insert(make_pair("mycmd3",
                                                "Usage : mycmd3 <param> | A useful terminal command with one argument."));
    auxillaryKnownArgumentMap_.insert(make_pair("mycmd4", "Usage : mycmd4 <param1> <param2> | A useful terminal command with two"
            " arguments."));
}

/** ExtraCommands is used to send command strings to classes derived
  * from ScanInterface. If ScanInterface receives an unrecognized
  * command from the user, it will pass it on to the derived class.
  * \param[in]  cmd_ The command to interpret.
  * \param[out] arg_ Vector or arguments to the user command.
  * \return True if the command was recognized and false otherwise.
  */
bool SkeletonInterface::ExtraCommands(const string &cmd_,
                                      vector<string> &args_) {
    if (cmd_ == "mycmd1") { // Handle the command.
    } else if (cmd_ == "mycmd2") {
        if (args_.size() >= 1) { // Do something with the argument.
        } else { // No argument, do something else.
        }
    } else if (cmd_ == "mycmd3") {
        if (args_.size() >= 1) { // Do something with the single argument.
        }
    } else if (cmd_ == "mycmd4") {
        if (args_.size() >= 2) { // Do something with the two arguments.
        }
    } else { return false; } // Unrecognized command.

    return true;
}

/** ExtraArguments is used to send command line arguments to classes derived
  * from ScanInterface. This method should loop over the optionExt elements
  * in the vector userOpts and check for those options which have been flagged
  * as active by ::Setup(). This should be overloaded in the derived class.
  * \return Nothing.
  */
void SkeletonInterface::ExtraArguments() {
    if (userOpts.at(0).active)
        cout << msgHeader << "Using option --myarg1 (-x).\n";
    if (userOpts.at(1).active)
        cout << msgHeader << "Using option --myarg2 (-y): arg=\""
                  << userOpts.at(1).argument << "\"\n";
    if (userOpts.at(2).active)
        cout << msgHeader << "Using option --myarg3 (-z): arg=\""
                  << userOpts.at(2).argument << "\"\n";
    if (userOpts.at(3).active)
        cout << msgHeader << "Using option --myarg4.\n";
}

/** ArgHelp is used to allow a derived class to add a command line option
  * to the main list of options. This method is called at the end of
  * from the ::Setup method.
  * Does nothing useful by default.
  * \return Nothing.
  */
void SkeletonInterface::ArgHelp() {
    AddOption(optionExt("myarg1", no_argument, NULL, 'x', "",
                        "A useful command line argument."));
    AddOption(optionExt("myarg2", required_argument, NULL, 'y', "<arg>",
                        "A useful command line argument with a required argument."));
    AddOption(optionExt("myarg3", optional_argument, NULL, 'z', "[arg]",
                        "A useful command line argument with an optional argument."));
    AddOption(optionExt("myarg4", no_argument, NULL, 0, "",
                        "A long only command line argument."));

    // Note that the following single character options are reserved by ScanInterface
    //  b, h, i, o, q, s, and v
}

/** SyntaxStr is used to print a linux style usage message to the screen.
  * \param[in]  name_ The name of the program.
  * \return Nothing.
  */
void SkeletonInterface::SyntaxStr(char *name_) {
    cout << " usage: " << string(name_) << " [options]\n";
}

/** Initialize the map file, the config file, the processor handler, 
  * and add all of the required processors.
  * \param[in]  prefix_ String to append to the beginning of system output.
  * \return True upon successfully initializing and false otherwise.
  */
bool SkeletonInterface::Initialize(string prefix_) {
    if (init)
        return false;
    // Do some initialization.
    return init = true;
}

/** Perform any last minute initialization before processing data.
  * /return Nothing.
  */
void SkeletonInterface::FinalInitialization() {
    // Do some last minute initialization before the run starts.
}

/** Receive various status notifications from the scan.
  * \param[in] code_ The notification code passed from ScanInterface methods.
  * \return Nothing.
  */
void SkeletonInterface::Notify(const string &code_/*=""*/) {
    if (code_ == "START_SCAN") {}
    else if (code_ == "STOP_SCAN") {}
    else if (code_ == "SCAN_COMPLETE") {
        cout << msgHeader << "Scan complete.\n";
    } else if (code_ == "LOAD_FILE") {
        cout << msgHeader << "File loaded.\n";
    } else if (code_ == "REWIND_FILE") {}
    else {
        cout << msgHeader << "Unknown notification code '" << code_
                  << "'!\n";
    }
}

