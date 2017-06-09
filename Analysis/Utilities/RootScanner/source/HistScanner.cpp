/// @authors K. Smith, S. V. Paulauskas

#include <chrono>
#include <iostream>

#include "HistUnpacker.hpp"
#include "HistScanner.hpp"

using namespace std;

HistScanner::HistScanner(HistUnpacker *unpacker) {
    unpacker_ = unpacker;

    auxillaryKnownArgumentMap_.insert(make_pair("plot", "Usage1 : plot <mod> <chan> <val> [pad] | Usage 2 : plot <val> [pad] | "
            "Plots a new histogram for module, chan, and value specified. If no pad is specified the plot is added to the "
            "currently selected pad."));
    auxillaryKnownArgumentMap_.insert(make_pair("refresh", "Usage : refresh [delayInSec] | Refreshes histograms after the "
            "delay specified has elapsed. If no delay is specified a force refresh occurs."));
    auxillaryKnownArgumentMap_.insert(make_pair("zero", "Usage : zero | Zeros all histograms and stored data."));
    auxillaryKnownArgumentMap_.insert(make_pair("clear", "Usage : clear [padNum] | Removes all histograms on the specified pad"
            ". If none are specified the canvas is cleared."));
    auxillaryKnownArgumentMap_.insert(make_pair("divide", "Usage: divide <numPads> | Usage : divide <numXPads> <numYpads> | "
            "Divides the canvas into the selected number of pads."));
}

/** Receive various status notifications from the scan.
  * \param[in] code_ The notification code passed from ScanInterface methods.
  * \return Nothing.
  */
void HistScanner::Notify(const string &code_/*=""*/) {
    if (code_ == "START_SCAN") {
        chrono::system_clock::now();
    } else if (code_ == "STOP_SCAN") {

    } else if (code_ == "SCAN_COMPLETE") {
        cout << msgHeader << "Scan complete.\n";
    } else if (code_ == "LOAD_FILE") {
        cout << msgHeader << "File loaded.\n";
    } else if (code_ == "REWIND_FILE") {

    } else {
        cout << msgHeader << "Unknown notification code '" << code_ << "'!\n";
    }
}

/** ExtraCommands is used to send command strings to classes derived
 * from ScanInterface. If ScanInterface receives an unrecognized
 * command from the user, it will pass it on to the derived class.
 * \param[in]  cmd_ The command to interpret.
 * \param[out] arg_ Vector or arguments to the user command.
 * \return True if the command was recognized and false otherwise.
 */
bool HistScanner::ExtraCommands(const string &cmd,
                                vector<string> &args) {
    if (cmd == "plot")
        unpacker_->PlotCommand(args);
    else if (cmd == "refresh")
        unpacker_->RefreshCommand(args);
    else if (cmd == "zero")
        unpacker_->ZeroCommand(args);
    else if (cmd == "clear")
        unpacker_->ClearCommand(args);
    else if (cmd == "divide")
        unpacker_->DivideCommand(args);
    else
        return false;
    return true;
}