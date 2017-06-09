///@file ScopeScanner.cpp
///@brief
///@author C. R. Thornsberry, S. V. Paulauskas
///@date May 19, 2017
#include <limits>

#include "RootInterface.hpp"
#include "ScopeScanner.hpp"

using namespace std;

/// Default constructor.
ScopeScanner::ScopeScanner(ScopeUnpacker *unpacker) {
    unpacker_ = unpacker;
    need_graph_update = false;
    acqRun_ = true;
    init = false;
    running = true;

    //We setup the commands that ScopeScanner knows about.
    auxillaryKnownArgumentMap_.insert(make_pair("set", "Usage : set <module> <channel> | "
            "Set the module and channel of signal of interest (default = 0, 0)."));
    auxillaryKnownArgumentMap_.insert(make_pair("single", "Perform a single capture."));
    auxillaryKnownArgumentMap_.insert(make_pair("thresh", "Usage: <low> [high] | Set the plotting window for trace maximum. "));
    auxillaryKnownArgumentMap_.insert(make_pair("fit", "Usage : fit <low> <high> | "
            "Turn on fitting of waveform. Set <low> to \"off\" to disable."));
    auxillaryKnownArgumentMap_.insert(make_pair("cfd", "Usage : cfd <F> <D> [L] "
            "Turn on cfd analysis of waveform. Set [F] to \"off\" to disable."));
    auxillaryKnownArgumentMap_.insert(make_pair("avg", "Usage : avg <number> | Set the number of waveforms to average."));
    auxillaryKnownArgumentMap_.insert(make_pair("save", "Usage : save <fileName> | "
            "Save the next trace to the specified file name. Do not provide the extension!"));
    auxillaryKnownArgumentMap_.insert(make_pair("delay", "Usage: delay <val> | "
            "Set the delay between drawing traces in seconds. Default = 1 s)."));
    auxillaryKnownArgumentMap_.insert(make_pair("log", "Toggle log/linear mode on the y-axis."));
    auxillaryKnownArgumentMap_.insert(make_pair("clear", "Clear all stored traces and start over."));
}

/** Initialize the map file, the config file, the processor handler,
  * and add all of the required processors.
  * \param[in]  prefix_ String to append to the beginning of system output.
  * \return True upon successfully initializing and false otherwise.
  */
bool ScopeScanner::Initialize(string prefix_) {
    if (init)
        return false;

    // Print a small welcome message.
    cout << "  Displaying traces for mod = " << unpacker_->GetModuleNumber()
         << ", chan = " << unpacker_->GetChannelNumber() << ".\n";

    return init = true;
}

/** Receive various status notifications from the scan.
  * \param[in] code_ The notification code passed from ScanInterface methods.
  * \return Nothing.
  */
void ScopeScanner::Notify(const string &code_/*=""*/) {
    if (code_ == "START_SCAN") {
        unpacker_->ClearEvents();
        acqRun_ = true;
    } else if (code_ == "STOP_SCAN")
        acqRun_ = false;
    else if (code_ == "SCAN_COMPLETE")
        cout << msgHeader << "Scan complete.\n";
    else if (code_ == "LOAD_FILE")
        cout << msgHeader << "File loaded.\n";
    else
        cout << msgHeader << "Unknown notification code '" << code_ << "'!\n";
}

/** ArgHelp is used to allow a derived class to add a command line option to the main list of options. This method is called at
  * the end of
  * from the ::Setup method.
  * Does nothing useful by default.
  * \return Nothing.
  */
void ScopeScanner::ArgHelp() {
    AddOption(optionExt("mod", required_argument, NULL, 'M', "<module>", "Module of signal of interest (default=0)"));
    AddOption(optionExt("chan", required_argument, NULL, 'C', "<channel>", "Channel of signal of interest (default=0)"));
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
    unpacker_->SetModuleNumber(atoi(userOpts.at(0).argument.c_str()));
    unpacker_->SetChannelNumber(atoi(userOpts.at(1).argument.c_str()));
    if (userOpts.at(0).active)
        cout << msgHeader << "Set module to (" << userOpts.at(0).argument.c_str() << ").\n";
    if (userOpts.at(1).active)
        cout << msgHeader << "Set channel to (" << atoi(userOpts.at(1).argument.c_str()) << ").\n";
}

/** ExtraCommands is used to send command strings to classes derived
  * from ScanInterface. If ScanInterface receives an unrecognized
  * command from the user, it will pass it on to the derived class.
  * \param[in]  cmd_ The command to interpret.
  * \param[out] arg_ Vector or arguments to the user command.
  * \return True if the command was recognized and false otherwise.
  */
bool ScopeScanner::ExtraCommands(const string &cmd_, vector<string> &args_) {
    if (cmd_ == "set") {
        if (args_.size() == 2) {
            unpacker_->ClearEvents();
            unpacker_->SetModuleNumber(atoi(args_.at(0).c_str()));
            unpacker_->SetChannelNumber(atoi(args_.at(1).c_str()));
            unpacker_->SetResetGraph(true);
        } else {
            cout << msgHeader << "Invalid number of parameters to 'set'\n";
            cout << msgHeader << " -SYNTAX- set <module> <channel>\n";
        }
    } else if (cmd_ == "single") {
        unpacker_->SetNumberTracesToAverage(1);
    } else if (cmd_ == "thresh") {
        if (args_.size() == 1) {
            unpacker_->SetThreshLow(atoi(args_.at(0).c_str()));
            unpacker_->SetThreshHigh(numeric_limits<unsigned int>::max());
        } else if (args_.size() == 2) {
            unpacker_->SetThreshLow(atoi(args_.at(0).c_str()));
            unpacker_->SetThreshHigh(atoi(args_.at(1).c_str()));
        } else {
            cout << msgHeader << "Invalid number of parameters to 'thresh'\n";
            cout << msgHeader
                 << " -SYNTAX- thresh <lowerThresh> [upperThresh]\n";
        }
    } else if (cmd_ == "fit") {
        if (args_.size() == 1 && args_.at(0) == "off") {
            // Turn root fitting off.
            if (unpacker_->PerformFit()) {
                cout << msgHeader << "Disabling root fitting.\n";
                unpacker_->SetPerformFit(false);
            } else
                cout << msgHeader << "Fitting is not enabled.\n";
        } else if (args_.size() == 2) { // Turn root fitting on.
            unpacker_->SetFitLow(atoi(args_.at(0).c_str()));
            unpacker_->SetFitHigh(atoi(args_.at(1).c_str()));
            cout << msgHeader << "Setting root fitting range to [" << args_.at(0).c_str() << ", " << args_.at(1).c_str()
                 << "].\n";
            unpacker_->SetPerformFit(true);
        } else {
            cout << msgHeader << "Invalid number of parameters to 'fit'\n";
            cout << msgHeader << " -SYNTAX- fit <low> <high>\n";
            cout << msgHeader << " -SYNTAX- fit off\n";
        }
    } else if (cmd_ == "cfd") {
        if (args_.empty())
            unpacker_->SetPerformCfd(true);
        else if (args_.size() == 1) {
            if (args_.at(0) == "off") { // Turn cfd analysis off.
                if (unpacker_->PerformCfd()) {
                    cout << msgHeader << "Disabling cfd analysis.\n";
                    unpacker_->SetPerformCfd(false);
                } else
                    cout << msgHeader << "Cfd is not enabled.\n";
            }
        } else if (args_.size() == 3) {
            unpacker_->SetCfdFraction(atof(args_.at(0).c_str()));
            unpacker_->SetCfdDelay(atoi(args_.at(1).c_str()));
            unpacker_->SetCfdShift(atoi(args_.at(2).c_str()));
            unpacker_->SetPerformCfd(true);
            cout << msgHeader << "Enabling cfd analysis with F=" << args_.at(0).c_str() << ", D=" << args_.at(1).c_str()
                 << ", L=" << args_.at(2).c_str() << endl;
        } else {
            cout << msgHeader << "Invalid number of parameters to 'cfd'\n";
            cout << msgHeader << " -SYNTAX- cfd <F> <D> <L>\n";
            cout << msgHeader << " -SYNTAX- fit off\n";
        }
    } else if (cmd_ == "avg") {
        if (args_.size() == 1) {
            unpacker_->SetNumberTracesToAverage(atoi(args_.at(0).c_str()));
        } else {
            cout << msgHeader << "Invalid number of parameters to 'avg'\n";
            cout << msgHeader << " -SYNTAX- avg <numWavefroms>\n";
        }
    } else if (cmd_ == "save") {
        if (args_.size() == 1) {
            unpacker_->SetSaveFile(args_.at(0));
        } else {
            cout << msgHeader << "Invalid number of parameters to 'save'\n";
            cout << msgHeader << " -SYNTAX- save <fileName>\n";
        }
    } else if (cmd_ == "delay") {
        if (args_.size() == 1)
            unpacker_->SetDelayInSeconds(atoi(args_.at(0).c_str()));
        else {
            cout << msgHeader << "Invalid number of parameters to 'delay'\n";
            cout << msgHeader << " -SYNTAX- delay <time>\n";
        }
    } else if (cmd_ == "log") {
        if (RootInterface::get()->GetCanvas()->GetLogy()) {
            RootInterface::get()->GetCanvas()->SetLogy(0);
            cout << msgHeader << "y-axis set to linear.\n";
        } else {
            RootInterface::get()->GetCanvas()->SetLogy(1);
            cout << msgHeader << "y-axis set to log.\n";
        }
    } else if (cmd_ == "clear") {
        unpacker_->ClearEvents();
        cout << msgHeader << "Event deque cleared.\n";
    } else if (cmd_ == "func" || cmd_ == "funk") {
        if (args_.size() == 1) {
            if (!unpacker_->SelectFittingFunction(args_.at(0)))
                cout << msgHeader << "Unknown fitting function requested." << endl;
        } else
            cout << msgHeader << "Invalid number of parameters. Usage : func <functionName>" << endl;
    } else
        return false;

    return true;
}