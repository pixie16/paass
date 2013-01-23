#ifndef MESSENGER_HPP
#define MESSENGER_HPP

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "Exceptions.hpp"

/** This class outputs nicely formated messages during configuration loading.*/
class Messenger {
    public:
        /** Default constructors sets output to std::cout*/
        Messenger () {
            file_ = false;
            out_ = &std::cout;
        }

        /** Sets output to file.*/
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

        /** Start of some main category.*/
        void start(std::string msg) {
            if (!endline_)
                *out_ << std::endl;

            *out_ << std::setfill('.') << std::setw(66) 
                  << std::left << msg;
            endline_ = false;
        }

        /** Detail under main category, higer level parameter
         *  imposes more indentation of the message. */
        void detail(std::string msg, short level = 0, std::string symbol = "*");

        /** Warning message */
        void warning(std::string msg);

        /** Message shown during scanning*/
        void run_message(std::string msg);

        /** At the end of main category, [Done] message.*/
        void done() {
            *out_ << std::setfill(' ');
            if (endline_) {
                *out_ << std::right << std::setw(72);
            }
            *out_ << "[Done]" << std::endl;
            endline_ = true;
        }

        /** At the end of main category, [FAIL] message.*/
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
