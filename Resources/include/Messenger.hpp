/** \file Messenger.hpp
 * \brief A class to handle messages and output them prettily
 * \author K. A. Miernik
 * \date December 14, 2014
 */
#ifndef MESSENGER_HPP
#define MESSENGER_HPP

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "Exceptions.hpp"

//!This class outputs nicely formatted messages during configuration loading.
class Messenger {
public:
    /** Default constructors sets output to std::cout*/
    Messenger() {
        file_ = false;
        out_ = &std::cout;
    }

    /** Sets output to file.
     * \param [in] name : the name of the output file */
    Messenger(const std::string &name) {
        file_ = true;
        fout_.open(name.c_str());
        if (!fout_.good()) {
            std::string msg =
                    "Messenger::Messenger: Could not open file " + name;
            throw IOException(msg);
        }
        out_ = &fout_;
    }

    /** Start of some main category.
     * \param [in] msg : the message to output */
    void start(std::string msg) {
        if (!endline_)
            *out_ << std::endl;

        *out_ << std::setfill('.') << std::setw(66)
              << std::left << msg;
        endline_ = false;
    }

    /** Detail under main category, higer level parameter
     *  imposes more indentation of the message.
     * \param [in] msg : the message to output
     * \param [in] level : the output level
     * \param [in] symbol : the symbol to use in the filler */
    void detail(std::string msg, short level = 0, std::string symbol = "*");

    /** Warning message
    * \param [in] msg : the message to output
    * \param [in] level : the output level */
    void warning(std::string msg, short level = 0);

    /** Message shown during scanning
    * \param [in] msg : the message to output */
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

    /** Default destructor (closes the file) */
    ~Messenger() {
        if (file_)
            fout_.close();
    }

private:
    std::ostream *out_; //!< the output stream
    std::ofstream fout_;//!< the output file stream
    bool file_;//!< true if the file is opened
    static bool endline_;//!< true if we have an endline
};

#endif
