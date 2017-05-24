#include <chrono>
#include <iostream>

#include "HistUnpacker.hpp"
#include "HistScanner.hpp"

using namespace std;

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
    else if (cmd == "help")
        HelpCommand(args);
    else
        return false;
    return true;
}

void HistScanner::CmdHelp(const string &prefix) {
    HelpCommand(vector<string>());
}

void HistScanner::HelpCommand(const vector<string> &args) {
    if (args.size() == 1) {
        if (args[0] == "plot") {
            cout << "Usage: plot <mod> <chan> <expr> [pad]\n";
            cout << "       plot <expr> [pad]\n";
            cout
                    << " Plots a new histogram for module, chan and expr specified.\n";
            cout
                    << " If no pad is specified the plot is added to the currently selected pad.\n";
            return;
        } else if (args[0] == "refresh") {
            cout << "Usage: refresh [delayInSec]\n";
            cout
                    << " Refreshes the histograms after the delay specified has elapsed. If no delay is specified a force refresh occurs.\n";
            return;
        } else if (args[0] == "zero") {
            cout << "Usage: zero\n";
            cout << " Zeros all histograms and stored data.\n";
            return;
        } else if (args[0] == "clear") {
            cout << "Usage: clear [padNum]\n";
            cout
                    << " Removes all histograms on the specified pad. If none specified the canvas is cleared.\n";
            return;
        } else if (args[0] == "divide") {
            cout << "Usage: divide <numPads>\n";
            cout << "       divide <numXPads> <numYPads>\n";
            cout
                    << " Divides the canvas in the selected number of pads.\n";
            return;
        }
    }
    cout << "Specific Commands:	\n";
    cout << " plot    - Creates a plot.\n";
    cout << " refresh - Refresh histograms.\n";
    cout << " zero    - Zeros all plots and associated data.\n";
    cout << " clear   - Removes the plots on a given pad.\n";
    cout << " divide  - Divides the canvas into multiple pads.\n";
    return;
}