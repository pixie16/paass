#include "Messenger.hpp"

bool Messenger::endline_ = true;

void Messenger::detail(std::string msg) {
    std::vector<std::string> lines;
    while(msg.length() > 0) {
        std::string line;
        if (msg.length() > 60) {
            line = msg.substr(0, 60);
            msg = msg.substr(60, msg.length());
        } else {
            line = msg;
            msg = "";
        }
        lines.push_back(line);
    }

    if (!endline_) {
        *out_ << std::endl;
        endline_ = true;
    }
    
    std::vector<std::string>::iterator it = lines.begin();
    *out_ << std::left << std::setfill(' ')
            << "    * " << std::setw(60) << (*it++) << std::endl;
    for (; it != lines.end(); ++it) 
        *out_ << "      " << std::setw(60) << *it << std::endl;
}

