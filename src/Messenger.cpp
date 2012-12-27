#include "Messenger.hpp"

bool Messenger::endline_ = true;

void Messenger::detail(std::string msg, short level /*= 0*/) {
    std::vector<std::string> lines;
    int width = 60 - 4 * level;
    std::stringstream indention;
    for (unsigned i = 0; i < level; ++i)
        indention << "    ";
    indention << "   ";

    while(msg.length() > 0) {
        std::string line;
        if (msg.length() > width) {
            line = msg.substr(0, width);
            msg = msg.substr(width, msg.length());
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
            << indention.str() << "* " << std::setw(width) << (*it++) << std::endl;
    for (; it != lines.end(); ++it) 
        *out_ << indention.str() << "  " << std::setw(width) << *it << std::endl;
}

