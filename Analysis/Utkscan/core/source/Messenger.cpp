/** \file Messenger.cpp
 * \brief A class to handle messages and output them prettily
 * \author K. A. Miernik
 * \date December 14, 2014
 */
#include "Messenger.hpp"

bool Messenger::endline_ = true;

void Messenger::detail(std::string msg, short level /*= 0*/,
                       std::string symbol /* = "*" */) {
    std::vector<std::string> lines;
    int width = 61 - 4 * level - symbol.length();
    std::stringstream indention;
    for (short i = 0; i < level; ++i)
        indention << "    ";
    indention << "   ";

    while((int)msg.length() > 0) {
        std::string line;
        if ((int)msg.length() > width) {
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
          << indention.str() << symbol << " "
          << std::setw(width) << (*it++) << std::endl;
    for (; it != lines.end(); ++it)
        *out_ << indention.str() << "  " << std::setw(width)
              << *it << std::endl;
}

void Messenger::warning(std::string msg, short level) {
    detail(msg + " !", level, "!");
}

void Messenger::run_message(std::string msg) {
    detail(msg, 0, ">>>");
}
