///@file EventReaderInterface.cpp
///@brief Derived class to handle the user interface for EventReader
///@authors S. V. Paulauskas and C. R. Thornsberry
///@date June 8, 2017

#include <iostream>

#include "EventReaderInterface.hpp"

/** ExtraCommands is used to send command strings to classes derived
  * from ScanInterface. If ScanInterface receives an unrecognized
  * command from the user, it will pass it on to the derived class.
  * \param[in]  cmd_ The command to interpret.
  * \param[out] arg_ Vector or arguments to the user command.
  * \return True if the command was recognized and false otherwise.
  */
bool EventReaderInterface::ExtraCommands(const std::string &cmd_, std::vector<std::string> &args_){
    if(cmd_ == "skip"){
        if(args_.size() >= 1){ // Do something with the argument.
            unpacker_->SetNumberToSkip(strtoul(args_.at(0).c_str(), NULL, 0));
            std::cout << msgHeader << "Skipping " << unpacker_->GetNumberToSkip() << " events.\n";
        }
        else {
            std::cout << msgHeader << "Invalid number of parameters to 'skip'\n";
            std::cout << msgHeader << " -SYNTAX- skip <numEvents>\n";
        }
    } else if(cmd_ == "flags"){
        unpacker_->SetShowFlags(!unpacker_->GetShowFlags());
    } else if(cmd_ == "trace"){
        unpacker_->SetShowTrace(!unpacker_->GetShowTrace());
    } else
        return false; // Unrecognized command.

    return true;
}

/** ExtraArguments is used to send command line arguments to classes derived
  * from ScanInterface. This method should loop over the optionExt elements
  * in the vector userOpts and check for those options which have been flagged
  * as active by ::Setup(). This should be overloaded in the derived class.
  * \return Nothing.
  */
void EventReaderInterface::ExtraArguments(){
    if(userOpts.at(0).active){
        unpacker_->SetNumberToSkip(strtoul(userOpts.at(0).argument.c_str(), NULL, 0));
        std::cout << msgHeader << "Skipping " << unpacker_->GetNumberToSkip() << " events.\n";
    }
}

/** CmdHelp is used to allow a derived class to print a help statement about
  * its own commands. This method is called whenever the user enters 'help'
  * or 'h' into the interactive terminal (if available).
  * \param[in]  prefix_ String to append at the start of any output. Not used by default.
  * \return Nothing.
  */
void EventReaderInterface::CmdHelp(const std::string &prefix_/*=""*/){
    std::cout << "   skip <N> - Skip the next N events.\n";
}

/** ArgHelp is used to allow a derived class to add a command line option
  * to the main list of options. This method is called at the end of
  * from the ::Setup method.
  * Does nothing useful by default.
  * \return Nothing.
  */
void EventReaderInterface::ArgHelp(){
    AddOption(optionExt("skip", required_argument, NULL, 'S', "<N>", "Skip the first N events in the input file."));
}

/** Receive various status notifications from the scan.
  * \param[in] code_ The notification code passed from ScanInterface methods.
  * \return Nothing.
  */
void EventReaderInterface::Notify(const std::string &code_/*=""*/){
    if(code_ == "START_SCAN"){  }
    else if(code_ == "STOP_SCAN"){  }
    else if(code_ == "SCAN_COMPLETE"){ std::cout << msgHeader << "Scan complete.\n"; }
    else if(code_ == "LOAD_FILE"){ std::cout << msgHeader << "File loaded.\n"; }
    else if(code_ == "REWIND_FILE"){  }
    else{ std::cout << msgHeader << "Unknown notification code '" << code_ << "'!\n"; }
}