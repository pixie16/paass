#ifndef CTERMINAL_H
#define CTERMINAL_H

#include <string>
#include <sstream>

#ifdef USE_NCURSES

#include <curses.h>

extern bool SIGNAL_INTERRUPT;

#endif

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

/* Parse all command line entries and find valid options. */
bool get_opt(int argc_, char **argv_, CLoption *options, unsigned int num_valid_opt_, void (*help_)()=dummy_help);

/* Return the length of a character string. */
unsigned int cstrlen(char *str_);

/* Extract a string from a character array. */
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

	void inc_ext_index_();
	
	void dec_ext_index_();
	
	unsigned int wrap_();

  public:		
	CommandHolder(unsigned int max_size_=5){
		max_size = max_size_;
		commands = new std::string[max_size];
		fragment = "";
		index = 0; total = 0;
		external_index = 0;
	}
	
	~CommandHolder(){ delete[] commands; }
	
	unsigned int GetSize(){ return max_size; }
	
	unsigned int GetTotal(){ return total; }
	
	unsigned int GetIndex(){ return external_index; }
	
	void Push(const std::string &input_);
	
	void Capture(const std::string &input_){ fragment = input_; }
		
	void Clear();
	
	std::string GetPrev();
	
	std::string GetNext();
	
	void Dump();
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
	
	bool GetInsertMode(){ return insert_mode; }
	
	void ToggleInsertMode(){
		if(insert_mode){ insert_mode = false; }
		else{ insert_mode = true; }
	}
	
	unsigned int GetSize(){ return command.size(); }
	
	std::string Get(int size_=std::string::npos){ return command.substr(0, size_); }
	
	void Set(std::string input_){ command = input_; }
	
	void Put(const char ch_, int index_);
	
	void Pop(int index_);
	
	void Clear(){ command = ""; }
};

///////////////////////////////////////////////////////////////////////////////
// Terminal
///////////////////////////////////////////////////////////////////////////////

#ifdef USE_NCURSES

void sig_int_handler(int ignore_);

// Setup the interrupt signal intercept
void setup_signal_handlers();

class Terminal{
  private:
	std::streambuf *pbuf, *original;
	std::stringstream stream;
	WINDOW *main;
	WINDOW *output_window;
	WINDOW *input_window;
	CommandHolder commands;
	CommandString cmd;
	bool init;
	int text_length;
	int cursX, cursY;
	int offset;
	
	// Refresh the terminal
	void refresh_();
	
	// Update the positions of the physical and logical cursors
	void update_cursor_();
	
	// Clear the command prompt
	void clear_();
	
	// Force a character to the input screen
	void in_char_(const char input_);

	// Force a character string to the input screen
	void in_print_(const char *input_);

	// Dump all text in the stream to the output screen
	void dump_();

  public:
	Terminal();
	
	~Terminal();
		
	// Initialize the terminal interface
	void Initialize();
		
	// Set the command prompt
	void SetPrompt(const char *input_);
	
	// Force a character to the output screen
	void putch(const char input_);

	// Force a character string to the output screen
	void print(const char *input_);
			
	// Wait for the user to input a command
	std::string GetCommand();
	
	// Close the window and restore control to the terminal
	void Close();
};

#endif

#endif
