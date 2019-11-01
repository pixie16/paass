/** \file CTerminal.h
  * 
  * \brief Library to handle all aspects of a stand-alone command line interface
  *
  * Library to facilitate the creation of C++ executables with
  * interactive command line interfaces under a linux environment
  *
  * \author Cory R. Thornsberry and Karl Smith
  * 
  * \date Oct. 1st, 2015
  * 
*/

#ifndef CTERMINAL_H
#define CTERMINAL_H

#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <vector>
#include <deque>

///Default size of terminal scroll back buffer in lines.
#define SCROLLBACK_SIZE 1000

#define CTERMINAL_VERSION "1.2.10"
#define CTERMINAL_DATE "Sept. 7th, 2016"

#include <curses.h>

extern std::string CPP_APP_VERSION;

template<typename T>
std::string to_str(const T &input_);

///////////////////////////////////////////////////////////////////////////////
// CommandHolder
///////////////////////////////////////////////////////////////////////////////

class CommandHolder {
private:
    unsigned int max_size;
    unsigned int index, total;
    unsigned int external_index;
    std::string *commands;
    std::string fragment;

    /** Convert the external index (relative to the most recent command) to the internal index
      * which is used to actually access the stored commands in the command array. */
    unsigned int wrap_();

public:
    CommandHolder(unsigned int max_size_ = 1000) {
        max_size = max_size_;
        commands = new std::string[max_size];
        fragment = "";
        index = 0;
        total = 0;
        external_index = 0;
    }

    ~CommandHolder() { delete[] commands; }

    /// Get the maximum size of the command array
    unsigned int GetSize() { return max_size; }

    /// Get the total number of commands
    unsigned int GetTotal() { return total; }

    /// Get the current command index (relative to the most recent command)
    unsigned int GetIndex() { return external_index; }

    /// Push a new command into the storage array
    void Push(std::string &input_);

    /// Capture the current command line text and store it for later use
    void Capture(const std::string &input_) { fragment = input_; }

    /// Clear the command array
    void Clear();

    /// Get the previous command entry
    std::string GetPrev();

    /// Get the next command entry but do not change the internal array index
    std::string PeekPrev();

    /// Get the next command entry
    std::string GetNext();

    /// Get the next command entry but do not change the internal array index
    std::string PeekNext();

    /// Dump all stored commands to the screen
    void Dump();

    /// Reset history to last item
    void Reset();
};

///////////////////////////////////////////////////////////////////////////////
// Terminal
///////////////////////////////////////////////////////////////////////////////

void sig_int_handler(int ignore_);

// Setup the interrupt signal intercept
void setup_signal_handlers();

class Terminal {
private:
    std::map<std::string, int> attrMap;
    std::streambuf *pbuf, *original;
    std::stringstream stream;
    std::string historyFilename_;
    WINDOW *main;
    WINDOW *output_window;
    WINDOW *input_window;
    WINDOW *status_window;
    CommandHolder commands;
    std::string cmd;
    bool init;
    int cursX, cursY;
    int offset;
    int _winSizeX, _winSizeY;
    int _statusWindowSize;
    std::vector<std::string> statusStr;
    std::deque<std::string> cmd_queue; /// The queue of commands read from a command script.
    ///The prompt string.
    std::string prompt;
    ///The tab complete flag
    bool enableTabComplete;
    float commandTimeout_; ///<Time in seconds to wait for command.
    bool insertMode_;

    short tabCount;
    bool debug_; ///<Flag indicating verbose output is enabled.

    std::ofstream logFile;

    bool from_script; /// Set to true if a command is read from a script instead of user input.
    bool prompt_user; /// Set to true if the user should be prompted with a yes/no question.

    /// Size of the scroll back buffer in lines.
    int _scrollbackBufferSize;

    /// Number of lines scrolled back
    int _scrollPosition;

    /// Refresh the terminal
    void refresh_();

    /// Resize the terminal
    void resize_();

    /// Scroll the output by a specified number of lines.
    void scroll_(int numLines);

    /// Update the positions of the physical and logical cursors
    void update_cursor_();

    /// Clear the command prompt output
    void clear_();

    /// Force a character to the input screen
    void in_char_(const char input_);

    /// Force a character string to the input screen
    void in_print_(const char *input_);

    /// Initialize terminal colors
    void init_colors_();

    /// Read commands from a command script.
    bool LoadCommandFile(const char *filename_);

    /// Load a list of previous commands from a file
    bool LoadCommandHistory(bool overwrite);

    /// Save previous commands to a file
    bool SaveCommandHistory();

    /// Force a character string to the output screen
    void print(WINDOW *window, std::string input_);

    /// Split a string into multiple commands separated by a ';'.
    void
    split_commands(const std::string &input_, std::deque<std::string> &cmds);

public:
    Terminal();

    ~Terminal();

    /// Initialize the terminal interface
    void Initialize();

    ///Specify the log file to append.
    bool SetLogFile(std::string logFileName);

    ///Initalize terminal debug mode.
    void SetDebug(bool debug = true) { debug_ = debug; };

    /// Initalizes a status window under the input temrinal.
    void AddStatusWindow(unsigned short numLines = 1);

    ///Set the status message.
    void SetStatus(std::string status, unsigned short line = 0);

    ///Clear the status line.
    void ClearStatus(unsigned short line = 0);

    ///Append some text to the status line.
    void AppendStatus(std::string status, unsigned short line = 0);

    ///Enable tab auto complete functionlity.
    void EnableTabComplete(bool enable = true);

    ///Handle tab complete functionality.
    void TabComplete(const std::string &input_,
                     const std::vector<std::string> &possibilities_);

    ///Enable a timeout while waiting fro a command.
    void EnableTimeout(float timeout = 0.5);

    /// Set the command filename for storing previous commands
    void SetCommandHistory(std::string filename, bool overwrite = false);

    /// Set the command prompt
    void SetPrompt(const char *input_);

    /// Force a character to the output screen
    void putch(const char input_);

    /// Disrupt ncurses while boolean is true
    void pause(bool &flag);

    /// Dump all text in the stream to the output screen
    void flush();

    /// Print a command to the terminal output.
    void PrintCommand(const std::string &cmd_);

    /// Wait for the user to input a command
    std::string GetCommand(std::string &args, const int &prev_cmd_return_ = 0);

    /// Close the window and restore control to the terminal
    void Close();

    /// Clear's the current cmd line in the poll2 prompt. (This is a wrapper of the internal clear_() command, we probably should reimplement it rather than just wrapping a private member function. )
    void ClearCmd();
};

/// Split a string about some delimiter.
unsigned int split_str(std::string str, std::vector<std::string> &args,
                       char delimiter = ' ');


#endif
