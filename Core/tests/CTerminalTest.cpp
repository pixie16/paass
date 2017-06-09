///@authors K. Smith
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include "CTerminal.h"

int main(int argc, char *argv[]) {
    bool debug = false;
    //Vector of possible commands.
    std::vector<std::string> commandChoices = {"help", "quit", "debug", "tab"};
    //Map of vector possible arguments for a given command.
    std::map<std::string, std::vector<std::string> > argumentChoices;
    argumentChoices["tab"] = std::vector<std::string> {"arg1", "arg2"};


    Terminal term;
    term.Initialize();
    std::cout << "This is test terminal.\n";
    std::cout << "$TERM: " << std::getenv("TERM") << "\n";

    term.SetCommandHistory("CTerminalTest.cmd");
    term.SetPrompt("Test> ");
    term.AddStatusWindow();
    term.EnableTabComplete();

    std::string cmd = "", arg;

    while (true) {
        cmd = term.GetCommand(arg);
        if (debug)
            std::cout << "TEST: cmd='" << cmd << "' arg='" << arg << "'\n";

        if (cmd.find("\t") != std::string::npos) {
            term.TabComplete(cmd, commandChoices);
            continue;
        } else if (arg.find("\t") != std::string::npos) {
            term.TabComplete(arg, argumentChoices[cmd]);
            continue;
        }

        if (cmd == "quit") break;
        else if (cmd == "help") {
            std::cout << "This is a test program to test the terminal.\n";
        } else if (cmd == "debug") {
            debug = !debug;
            std::cout << "Debug mode ";
            if (debug) std::cout << "is on.\n";
            else std::cout << "is off.\n";
            term.SetDebug(debug);
        } else if (cmd == "tab") {
            std::cout << "This command tests the argument tab completion.\n";
            std::cout << "\tTab Completion arguments: '" << arg << "'\n";
        } else {
            std::cout << "Unknown command: '" << cmd << "'\n";
        }

    }

    term.Close();

}
