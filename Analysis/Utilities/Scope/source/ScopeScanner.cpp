///@file ScopeScanner.cpp
///@brief
///@author S. V. Paulauskas
///@date May 19, 2017

#include "ScopeScanner.hpp"

using namespace std;

/// Default constructor.
ScopeScanner::ScopeScanner() : RootScanner() {
    need_graph_update = false;
    acqRun_ = true;
    singleCapture_ = false;
    init = false;
    running = true;
    performFit_ = false;
    performCfd_ = false;
    numEvents = 20;
    numAvgWaveforms_ = 1;
    cfdF_ = 0.5;
    cfdD_ = 1;
    cfdL_ = 1;
    fitLow_ = 10;
    fitHigh_ = 15;
    delay_ = 2;
    num_displayed = 0;
    time(&last_trace);
}

/** Initialize the map file, the config file, the processor handler,
  * and add all of the required processors.
  * \param[in]  prefix_ String to append to the beginning of system output.
  * \return True upon successfully initializing and false otherwise.
  */
bool ScopeScanner::Initialize(string prefix_) {
    if (init) { return false; }

    // Print a small welcome message.
    cout << "  Displaying traces for mod = " << unpacker_.GetMod()
         << ", chan = " << unpacker_.GetChan() << ".\n";

    return (init = true);
}

/** Receive various status notifications from the scan.
  * \param[in] code_ The notification code passed from ScanInterface methods.
  * \return Nothing.
  */
void ScopeScanner::Notify(const string &code_/*=""*/) {
    if (code_ == "START_SCAN") {
        unpacker_.ClearEvents();
        acqRun_ = true;
    } else if (code_ == "STOP_SCAN") { acqRun_ = false; }
    else if (code_ == "SCAN_COMPLETE") {
        cout << msgHeader << "Scan complete.\n";
    } else if (code_ == "LOAD_FILE") { cout << msgHeader << "File loaded.\n"; }
    else if (code_ == "REWIND_FILE") {}
    else {
        cout << msgHeader << "Unknown notification code '" << code_ << "'!\n";
    }
}

/** CmdHelp is used to allow a derived class to print a help statement about
  * its own commands. This method is called whenever the user enters 'help'
  * or 'h' into the interactive terminal (if available).
  * \param[in]  prefix_ String to append at the start of any output. Not used by default.
  * \return Nothing.
  */
void ScopeScanner::CmdHelp(const string &prefix_/*=""*/) {
    cout << "   set <module> <channel>  - Set the module and channel of signal of interest (default = 0, 0).\n";
    cout << "   stop                    - Stop the acquistion.\n";
    cout << "   run                     - Run the acquistion.\n";
    cout << "   single                  - Perform a single capture.\n";
    cout
            << "   thresh <low> [high]     - Set the plotting window for trace maximum.\n";
    cout
            << "   fit <low> <high>        - Turn on fitting of waveform. Set <low> to \"off\" to disable.\n";
    cout
            << "   cfd [F=0.5] [D=1] [L=1] - Turn on cfd analysis of waveform. Set [F] to \"off\" to disable.\n";
    cout
            << "   avg <numWaveforms>      - Set the number of waveforms to average.\n";
    cout
            << "   save <fileName>         - Save the next trace to the specified file name..\n";
    cout
            << "   delay [time]            - Set the delay between drawing traces (in seconds, default = 1 s).\n";
    cout
            << "   log                     - Toggle log/linear mode on the y-axis.\n";
    cout
            << "   clear                   - Clear all stored traces and start over.\n";
}

/** ArgHelp is used to allow a derived class to add a command line option
  * to the main list of options. This method is called at the end of
  * from the ::Setup method.
  * Does nothing useful by default.
  * \return Nothing.
  */
void ScopeScanner::ArgHelp() {
    AddOption(optionExt("mod", required_argument, NULL, 'M', "<module>",
                        "Module of signal of interest (default=0)"));
    AddOption(optionExt("chan", required_argument, NULL, 'C', "<channel>",
                        "Channel of signal of interest (default=0)"));
}

/** SyntaxStr is used to print a linux style usage message to the screen.
  * \param[in]  name_ The name of the program.
  * \return Nothing.
  */
void ScopeScanner::SyntaxStr(char *name_) {
    cout << " usage: " << string(name_) << " [options]\n";
}

/** ExtraArguments is used to send command line arguments to classes derived
  * from ScanInterface. This method should loop over the optionExt elements
  * in the vector userOpts and check for those options which have been flagged
  * as active by ::Setup(). This should be overloaded in the derived class.
  * \return Nothing.
  */
void ScopeScanner::ExtraArguments() {
    if (userOpts.at(0).active)
        cout << msgHeader << "Set module to ("
             << unpacker_.SetMod(
                     atoi(userOpts.at(0).argument.c_str())) << ").\n";
    if (userOpts.at(1).active)
        cout << msgHeader << "Set channel to ("
             << unpacker_.SetChan(
                     atoi(userOpts.at(1).argument.c_str())) << ").\n";
}

/** ExtraCommands is used to send command strings to classes derived
  * from ScanInterface. If ScanInterface receives an unrecognized
  * command from the user, it will pass it on to the derived class.
  * \param[in]  cmd_ The command to interpret.
  * \param[out] arg_ Vector or arguments to the user command.
  * \return True if the command was recognized and false otherwise.
  */
bool ScopeScanner::ExtraCommands(const string &cmd_, vector<string> &args_) {
    if (cmd_ == "set") { // Toggle debug mode
        if (args_.size() == 2) {
            unpacker_.ClearEvents();
            unpacker_.SetMod(atoi(args_.at(0).c_str()));
            unpacker_.SetChan(atoi(args_.at(1).c_str()));
            unpacker_.SetResetGraph(true);
        } else {
            cout << msgHeader << "Invalid number of parameters to 'set'\n";
            cout << msgHeader << " -SYNTAX- set <module> <channel>\n";
        }
    } else if (cmd_ == "single") {
        singleCapture_ = !singleCapture_;
    } else if (cmd_ == "thresh") {
        if (args_.size() == 1) {
            unpacker_.SetThreshLow(atoi(args_.at(0).c_str()));
            unpacker_.SetThreshHigh(-1);
        } else if (args_.size() == 2) {
            unpacker_.SetThreshLow(atoi(args_.at(0).c_str()));
            unpacker_.SetThreshHigh(atoi(args_.at(1).c_str()));
        } else {
            cout << msgHeader << "Invalid number of parameters to 'thresh'\n";
            cout << msgHeader
                 << " -SYNTAX- thresh <lowerThresh> [upperThresh]\n";
        }
    } else if (cmd_ == "fit") {
        if (args_.size() >= 1 &&
            args_.at(0) == "off") { // Turn root fitting off.
            if (performFit_) {
                cout << msgHeader << "Disabling root fitting.\n";
//                delete graph->GetListOfFunctions()->FindObject(
//                        fittingFunction_->GetName());
                GetCanvas()->Update();
                performFit_ = false;
            } else { cout << msgHeader << "Fitting is not enabled.\n"; }
        } else if (args_.size() == 2) { // Turn root fitting on.
            fitLow_ = atoi(args_.at(0).c_str());
            fitHigh_ = atoi(args_.at(1).c_str());
            cout << msgHeader << "Setting root fitting range to [" << fitLow_
                 << ", " << fitHigh_ << "].\n";
            performFit_ = true;
        } else {
            cout << msgHeader << "Invalid number of parameters to 'fit'\n";
            cout << msgHeader << " -SYNTAX- fit <low> <high>\n";
            cout << msgHeader << " -SYNTAX- fit off\n";
        }
    } else if (cmd_ == "cfd") {
        cfdF_ = 0.5;
        cfdD_ = 1;
        cfdL_ = 1;
        if (args_.empty()) { performCfd_ = true; }
        else if (args_.size() == 1) {
            if (args_.at(0) == "off") { // Turn cfd analysis off.
                if (performCfd_) {
                    cout << msgHeader << "Disabling cfd analysis.\n";
                    performCfd_ = false;
                } else { cout << msgHeader << "Cfd is not enabled.\n"; }
            } else {
                cfdF_ = atof(args_.at(0).c_str());
                performCfd_ = true;
            }
        } else if (args_.size() == 2) {
            cfdF_ = atof(args_.at(0).c_str());
            cfdD_ = atoi(args_.at(1).c_str());
            performCfd_ = true;
        } else if (args_.size() == 3) {
            cfdF_ = atof(args_.at(0).c_str());
            cfdD_ = atoi(args_.at(1).c_str());
            cfdL_ = atoi(args_.at(2).c_str());
            performCfd_ = true;
        }
        if (performCfd_)
            cout << msgHeader << "Enabling cfd analysis with F=" << cfdF_
                 << ", D=" << cfdD_ << ", L=" << cfdL_ << endl;
    } else if (cmd_ == "avg") {
        if (args_.size() == 1) {
            numAvgWaveforms_ = atoi(args_.at(0).c_str());
        } else {
            cout << msgHeader << "Invalid number of parameters to 'avg'\n";
            cout << msgHeader << " -SYNTAX- avg <numWavefroms>\n";
        }
    } else if (cmd_ == "save") {
        if (args_.size() == 1) {
            saveFile_ = args_.at(0);
        } else {
            cout << msgHeader << "Invalid number of parameters to 'save'\n";
            cout << msgHeader << " -SYNTAX- save <fileName>\n";
        }
    } else if (cmd_ == "delay") {
        if (args_.size() == 1) { delay_ = atoi(args_.at(0).c_str()); }
        else {
            cout << msgHeader << "Invalid number of parameters to 'delay'\n";
            cout << msgHeader << " -SYNTAX- delay <time>\n";
        }
    } else if (cmd_ == "log") {
        if (GetCanvas()->GetLogy()) {
            GetCanvas()->SetLogy(0);
            cout << msgHeader << "y-axis set to linear.\n";
        } else {
            GetCanvas()->SetLogy(1);
            cout << msgHeader << "y-axis set to log.\n";
        }
    } else if (cmd_ == "clear") {
        unpacker_.ClearEvents();
        cout << msgHeader << "Event deque cleared.\n";
    } else { return false; }

    return true;
}