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
  * \version 1.2.03
*/

#ifndef CTERMINAL_H
#define CTERMINAL_H

#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <vector>

///Default size of terminal scroll back buffer in lines.
#define SCROLLBACK_SIZE 1000

#define CTERMINAL_VERSION "1.2.04"
#define CTERMINAL_DATE "Dec. 1st, 2015"

#include <curses.h>

extern std::string CPP_APP_VERSION;

template <typename T>
std::string to_str(const T &input_);

// Default target for get_opt
void dummy_help();

///////////////////////////////////////////////////////////////////////////////
// CLoption
///////////////////////////////////////////////////////////////////////////////

struct CLoption{
	char opt;
	std::string alias;
	std::string value;
	bool require_arg;
	bool optional_arg;
	bool is_active;
	
	CLoption(){
		Set("NULL", false, false);
		opt = 0x0;
		value = "";
		is_active = false;
	}
	
	CLoption(std::string name_, bool require_arg_, bool optional_arg_){
		Set(name_, require_arg_, optional_arg_);
		value = "";
		is_active = false;
	}
	
	void Set(std::string name_, bool require_arg_, bool optional_arg_){
		opt = name_[0]; alias = name_; require_arg = require_arg_; optional_arg = optional_arg_;
	}
};

/// Parse all command line entries and find valid options.
bool get_opt(unsigned int argc_, char **argv_, CLoption *options, unsigned int num_valid_opt_, void (*help_)()=dummy_help);

/// Return the length of a character string.
unsigned int cstrlen(char *str_);

/// Extract a string from a character array.
std::string csubstr(char *str_, unsigned int start_index_=0);

///////////////////////////////////////////////////////////////////////////////
// CommandHolder
///////////////////////////////////////////////////////////////////////////////

class CommandHolder{
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
	CommandHolder(unsigned int max_size_=1000){
		max_size = max_size_;
		commands = new std::string[max_size];
		fragment = "";
		index = 0; total = 0;
		external_index = 0;
	}
	
	~CommandHolder(){ delete[] commands; }
	
	/// Get the maximum size of the command array
	unsigned int GetSize(){ return max_size; }
	
	/// Get the total number of commands
	unsigned int GetTotal(){ return total; }
	
	/// Get the current command index (relative to the most recent command)
	unsigned int GetIndex(){ return external_index; }
	
	/// Push a new command into the storage array
	void Push(std::string &input_);
	
	/// Capture the current command line text and store it for later use
	void Capture(const std::string &input_){ fragment = input_; }
		
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
// CommandString
///////////////////////////////////////////////////////////////////////////////

class CommandString{
  private:
	std::string command;
	bool insert_mode;
	
  public:
	CommandString(){ 
		command = ""; 
		insert_mode = false;
	}
	
	/// Return the current "insert character" mode
	bool GetInsertMode(){ return insert_mode; }
	
	/// Toggle "insert character" mode on or off
	void ToggleInsertMode(){
		if(insert_mode){ insert_mode = false; }
		else{ insert_mode = true; }
	}
	
	/// Return the size of the command string
	unsigned int GetSize(){ return command.size(); }
	
	/// Return the command string
	std::string Get(size_t size_=std::string::npos){ return command.substr(0, size_); }
	
	/// Set the string the the specified input.
	void Set(std::string input_){ command = input_; }	
	
	/// Put a character into string at specified position.
	void Put(const char ch_, unsigned int index_);

	void Insert(size_t pos, const char* str);
	
	/// Remove a character from the string.
	void Pop(unsigned int index_);
	
	/// Clear the string.
	void Clear(){ command = ""; }
};

///////////////////////////////////////////////////////////////////////////////
// Terminal
///////////////////////////////////////////////////////////////////////////////

void sig_int_handler(int ignore_);

// Setup the interrupt signal intercept
void setup_signal_handlers();

class Terminal{
  private:
	std::map< std::string, int > attrMap;
	std::streambuf *pbuf, *original;
	std::stringstream stream;
	std::string historyFilename_;
	WINDOW *main;
	WINDOW *output_window;
	WINDOW *input_window;
	WINDOW *status_window;
	CommandHolder commands;
	CommandString cmd;
	bool init;
	int cursX, cursY;
	int offset;
	int _winSizeX,_winSizeY;
	int _statusWindowSize;
	std::vector<std::string> statusStr;
	///The prompt string.
	std::string prompt;
	///The tab complete flag
	bool enableTabComplete;
	float commandTimeout_; ///<Time in seconds to wait for command.

	short tabCount;

	std::ofstream logFile;
	
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
	
	/// Load a list of previous commands from a file
	bool LoadCommandHistory(bool overwrite);
	
	/// Save previous commands to a file
	bool SaveCommandHistory();

	/// Force a character string to the output screen
	void print(WINDOW *window, std::string input_);
			
  public:
	Terminal();
	
	~Terminal();
		
	/// Initialize the terminal interface
	void Initialize();
	
	///Specify the log file to append.
	bool SetLogFile(std::string logFileName);

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
	void TabComplete(std::vector<std::string> matches);

	///Enable a timeout while waiting fro a command.
	void EnableTimeout(float timeout = 0.5);

	/// Set the command filename for storing previous commands
	void SetCommandHistory(std::string filename, bool overwrite=false);
		
	/// Set the command prompt
	void SetPrompt(const char *input_);
	
	/// Force a character to the output screen
	void putch(const char input_);

	/// Disrupt ncurses while boolean is true
	void pause(bool &flag);

	/// Dump all text in the stream to the output screen
	void flush();

	/// Wait for the user to input a command
	std::string GetCommand();
	
	/// Close the window and restore control to the terminal
	void Close();
};

#endif
