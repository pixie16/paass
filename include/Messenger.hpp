#ifndef MESSENGER_HPP
#define MESSENGER_HPP

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "Exceptions.hpp"

class Messenger {
    public:
        Messenger () {
            file_ = false;
            out_ = &std::cout;
        }

        Messenger (const std::string& name) {
            file_ = true;
            fout_.open(name.c_str());
            if (!fout_.good()) {
                std::string msg =
                    "Messenger::Messenger: Could not open file " + name;
                throw IOException(msg);
            } 
            out_ = &fout_;
        }

        void start(std::string msg) {
            if (!endline_)
                *out_ << std::endl;

            *out_ << std::setfill('.') << std::setw(66) 
                  << std::left << msg;
            endline_ = false;
        }

        void detail(std::string msg);

        void done() {
            *out_ << std::setfill(' ');
            if (endline_) {
                *out_ << std::right << std::setw(72);
            }
            *out_ << "[Done]" << std::endl;
            endline_ = true;
        }

        void fail() {
            *out_ << std::setfill(' ');
            if (endline_) {
                *out_ << std::right << std::setw(72);
            }
            *out_ << "[FAIL]" << std::endl;
            endline_ = true;
        }

        ~Messenger() {
            if (file_)
                fout_.close();
        }

    private:
        std::ostream* out_;
        std::ofstream fout_;
        bool file_;
        static bool endline_;
};
#endif
