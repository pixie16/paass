/** \file CTerminal.cpp
  * 
  * \brief Library to handle all aspects of a stand-alone command line interface
  *
  * Library to facilitate the creation of C++ executables with
  * interactive command line interfaces under a linux environment
  *
  * \author Cory R. Thornsberry
  * 
  * \date April 24th, 2015
  * 
  * \version 1.1.04
*/

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <vector>

#ifdef USE_NCURSES

#include <signal.h>
#include <stdexcept>

#endif

#include "CTerminal.h"

#include "TermColors.h"

#ifdef USE_NCURSES

bool SIGNAL_INTERRUPT = false;
bool SIGNAL_RESIZE = false;

#endif

template <typename T>
std::string to_str(const T &input_){
	std::stringstream stream;
	stream << input_;
	return stream.str();
}

// Default target for get_opt
void dummy_help(){}

///////////////////////////////////////////////////////////////////////////////
// CLoption
///////////////////////////////////////////////////////////////////////////////

/// Parse all command line entries and find valid options.
bool get_opt(int argc_, char **argv_, CLoption *options, unsigned int num_valid_opt_, void (*help_)()/*=dummy_help*/){
	unsigned int index = 1;
	unsigned int previous_opt;
	bool need_an_argument = false;
	bool may_have_argument = false;
	bool is_valid_argument = false;
	while(index < argc_){
		if(argv_[index][0] == '-'){
			if(need_an_argument){
				std::cout << "\n Error: --" << options[previous_opt].alias << " [-" << options[previous_opt].opt << "] requires an argument\n";
				help_();
				return false;
			}

			is_valid_argument = false;
			if(argv_[index][1] == '-'){ // Word options
				std::string word_arg = csubstr(argv_[index], 2);
				for(unsigned int i = 0; i < num_valid_opt_; i++){
					if(word_arg == options[i].alias && word_arg != "NULL"){
						options[i].is_active = true; 
						previous_opt = i;
						if(options[i].require_arg){ need_an_argument = true; }
						else{ need_an_argument = false; }
						if(options[i].optional_arg){ may_have_argument = true; }
						else{ may_have_argument = false; }
						is_valid_argument = true;
					}
				}
				if(!is_valid_argument){
					std::cout << "\n Error: encountered unknown option --" << word_arg << std::endl;
					help_();
					return false;
				}
			}
			else{ // Character options
				unsigned int index2 = 1;
				while(argv_[index][index2] != '\0'){
					for(unsigned int i = 0; i < num_valid_opt_; i++){
						if(argv_[index][index2] == options[i].opt && argv_[index][index2] != 0x0){ 
							options[i].is_active = true; 
							previous_opt = i;
							if(options[i].require_arg){ need_an_argument = true; }
							else{ need_an_argument = false; }
							if(options[i].optional_arg){ may_have_argument = true; }
							else{ may_have_argument = false; }
							is_valid_argument = true;
						}
					}
					if(!is_valid_argument){
						std::cout << "\n Error: encountered unknown option -" << argv_[index][index2] << std::endl;
						help_();
						return false;
					}
					index2++;
				}
			}
		}
		else{ // An option argument
			if(need_an_argument || may_have_argument){
				options[previous_opt].value = csubstr(argv_[index]);
				need_an_argument = false;
				may_have_argument = false;
			}
			else{
				std::cout << "\n Error: --" << options[previous_opt].alias << " [-" << options[previous_opt].opt << "] takes no argument\n";
				help_();
				return false;			
			}
		}
		
		// Check for the case where the end option requires an argument, but did not receive it
		if(index == argc_-1 && need_an_argument){
			std::cout << "\n Error: --" << options[previous_opt].alias << " [-" << options[previous_opt].opt << "] requires an argument\n";
			help_();
			return false;	
		}
		
		index++;
	}
	return true;
}

/// Return the length of a character string.
unsigned int cstrlen(const char *str_){
	unsigned int output = 0;
	while(true){
		if(str_[output] == '\0'){ break; }
		output++;
	}
	return output;
}

/// Extract a string from a character array.
std::string csubstr(char *str_, unsigned int start_index_/*=0*/){
	std::string output = "";
	unsigned int index = start_index_;
	while(str_[index] != '\0' && str_[index] != ' '){
		output += str_[index];
		index++;
	}
	return output;
}

///////////////////////////////////////////////////////////////////////////////
// CommandHolder
///////////////////////////////////////////////////////////////////////////////

/// Push a new command into the storage array
void CommandHolder::Push(std::string &input_){
	input_.erase(input_.find_last_not_of(" \n\t\r")+1);
	commands[index] = input_;
	total++;
	index++;
	external_index = 0;
	
	if(index >= max_size){ index = 0; }
}

/// Clear the command array
void CommandHolder::Clear(){
	for(unsigned int i = 0; i < max_size; i++){
		commands[i] = "";
	}
}

/** Convert the external index (relative to the most recent command) to the internal index
  * which is used to actually access the stored commands in the command array. */
unsigned int CommandHolder::wrap_(){
	if(index < external_index){
		unsigned int temp = (external_index - index) % max_size;
		return max_size - temp;
	}
	else if(index >= max_size + external_index){
		unsigned int temp = (index - external_index) % max_size;
		return temp;
	}
	return (index - external_index);
}

/// Get the previous command entry
std::string CommandHolder::GetPrev(){
	if(total == 0){ return "NULL"; }

	external_index++;
	if(external_index >= max_size){ 
		external_index = max_size - 1; 
	}
	else if(external_index >= total){
		external_index = total;
	}

	return commands[wrap_()];
}

/// Get the previous command entry but do not change the internal array index
std::string CommandHolder::PeekPrev(){
	if(total == 0){ return "NULL"; }
	
	if(index == 0){ return commands[max_size-1]; }
	return commands[index-1];
}

/// Get the next command entry
std::string CommandHolder::GetNext(){
	if(total == 0){ return "NULL"; }

	if(external_index >= 1){
		external_index--;
	}

	if(external_index == 0){ return fragment; }
	return commands[wrap_()]; 
}

/// Get the next command entry but do not change the internal array index
std::string CommandHolder::PeekNext(){
	if(total == 0){ return "NULL"; }
	
	if(index == max_size-1){ return commands[0]; }
	return commands[index+1];
}

/// Dump all stored commands to the screen
void CommandHolder::Dump(){
	for(unsigned int i = 0; i < max_size; i++){
		if(commands[i] != ""){ std::cout << " " << i << ": " << commands[i] << std::endl; }
	}
}

///////////////////////////////////////////////////////////////////////////////
// CommandString
///////////////////////////////////////////////////////////////////////////////

/// Put a character into string at specified position.
void CommandString::Put(const char ch_, int index_){
	if(index_ < 0){ return; }
	else if(index_ < command.size()){ // Overwrite or insert a character
		if(!insert_mode) { command.insert(index_, 1, ch_); } // Insert
		else { command.at(index_) = ch_; } // Overwrite
	}
	else{ command.push_back(ch_); } // Appending to the back of the string
}

/// Remove a character from the string.
void CommandString::Pop(int index_){
	if(index_ < 0){ return ; }
	else if(index_ < command.size()){ // Pop a character out of the string
		command.erase(index_, 1);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Terminal
///////////////////////////////////////////////////////////////////////////////

#ifdef USE_NCURSES

void sig_int_handler(int ignore_){
	SIGNAL_INTERRUPT = true;
}

void signalResize(int ignore_) {
	SIGNAL_RESIZE = true;
}


// Setup the interrupt signal intercept
void setup_signal_handlers(){ 
	if(signal(SIGINT, SIG_IGN) != SIG_IGN){	
		if(signal(SIGINT, sig_int_handler) == SIG_ERR){
			throw std::runtime_error(" Error setting up signal handler!");
		}
	}

	//Handle resize signal
	signal(SIGWINCH, signalResize);

}
void Terminal::resize_() {
	//end session and then refresh to get new window sizes.
	endwin();
	refresh();
	//Get new window sizes
	getmaxyx(stdscr, _winSizeY, _winSizeX);

	//Make pad bigger if needed.
	int outputSizeY, outputSizeX;
	getmaxyx(output_window,outputSizeY,outputSizeX);
	if (outputSizeX < _winSizeX) {
		wresize(output_window,_scrollbackBufferSize,_winSizeX);
		wresize(input_window,1,_winSizeX);
	}

	//Mark resize as handled
	SIGNAL_RESIZE = false;
}

void Terminal::pause(bool &flag) {
	endwin();
	std::cout.rdbuf(original); // Restore cout's original streambuf
	setvbuf(stdout,NULL,_IOLBF,0); //Change to line buffering
	while (flag) sleep(1);
	std::cout.rdbuf(pbuf); // Restore cout's original streambuf
	refresh_();
}


void Terminal::refresh_(){
	if(!init){ return; }
	if (SIGNAL_RESIZE) resize_();
	
	pnoutrefresh(output_window, 
		_scrollbackBufferSize - _winSizeY - _scrollPosition, 0,  //Pad corner to be placed in top left 
		 0, 0, _winSizeY - 2, _winSizeX-1); //Size of window
	pnoutrefresh(input_window,0,0, _winSizeY-1, 0,_winSizeY,_winSizeX-1);
	doupdate();
	
}

void Terminal::scroll_(int numLines){
	if (!init){return;}
	//We subtract so that a negative number is scrolled up resulting in a positive position value.
	_scrollPosition -= numLines;
	if(_scrollPosition > _scrollbackBufferSize - (_winSizeY-1)) 
		_scrollPosition = _scrollbackBufferSize - (_winSizeY-1);
	else if (_scrollPosition < 0) _scrollPosition = 0;
	refresh_();

}

void Terminal::update_cursor_(){
	if(!init){ return; }
	move(cursY, cursX); // Move the physical cursor
	wmove(input_window, 0, cursX); // Move the logical cursor in the input window
}

void Terminal::clear_(){
	int start = (int)cmd.GetSize() + offset;
	for(start; start >= offset; start--){
		wmove(input_window, 0, start);
		wdelch(input_window);
	}
	cmd.Clear();
	cursX = offset;
	text_length = 0;
	update_cursor_();
	refresh_();
}

// Force a character to the input screen
void Terminal::in_char_(const char input_){
	cursX++;
	//If in insert mode we overwite the character otherwise insert it.
	if (cmd.GetInsertMode()) waddch(input_window, input_);
	else winsch(input_window, input_);
	update_cursor_();
	refresh_();
}

// Force text to the input screen
void Terminal::in_print_(const char* input_){
	cursX += cstrlen(input_);
	waddstr(input_window, input_);
	update_cursor_();
	refresh_();
}

void Terminal::init_colors_() {
	if(has_colors()) {
		start_color();
		//Use user's terminal colors.
		use_default_colors();

		//Define colors
		init_pair(1,COLOR_GREEN,-1);
		init_pair(2,COLOR_RED,-1);
		init_pair(3,COLOR_BLUE,-1);
		init_pair(4,COLOR_YELLOW,-1);
		init_pair(5,COLOR_MAGENTA,-1);
		init_pair(6,COLOR_CYAN,-1);
		init_pair(7,COLOR_WHITE,-1);
		
		//Assign colors to map
		attrMap[TermColors::DkGreen] = COLOR_PAIR(1);
		attrMap[TermColors::BtGreen] = COLOR_PAIR(1);
		attrMap[TermColors::DkRed] = COLOR_PAIR(2);
		attrMap[TermColors::BtRed] = COLOR_PAIR(2);
		attrMap[TermColors::DkBlue] = COLOR_PAIR(3);
		attrMap[TermColors::BtBlue] = COLOR_PAIR(3);
		attrMap[TermColors::DkYellow] = COLOR_PAIR(4);
		attrMap[TermColors::BtYellow] = COLOR_PAIR(4);
		attrMap[TermColors::DkMagenta] = COLOR_PAIR(5);
		attrMap[TermColors::BtMagenta] = COLOR_PAIR(5);
		attrMap[TermColors::DkCyan] = COLOR_PAIR(6);
		attrMap[TermColors::BtCyan] = COLOR_PAIR(6);
		attrMap[TermColors::DkWhite] = COLOR_PAIR(7);
		attrMap[TermColors::BtWhite] = COLOR_PAIR(7);
		attrMap[TermColors::Flashing] = A_BLINK;
		attrMap[TermColors::Underline] = A_UNDERLINE;
	}
}

bool Terminal::load_commands_(){
	std::ifstream input(cmd_filename.c_str());
	if(!input.good()){ return false; }
	
	size_t index;
	std::string cmd;
	std::vector<std::string> cmds;
	while(true){
		std::getline(input, cmd);
		if(input.eof()){ break; }
		
		// Strip the newline from the end
		index = cmd.find("\n");
		if(index != std::string::npos){
			cmd.erase(index);
		}
		
		// Push the command into the command array
		if(cmd != ""){ // Just to be safe!
			cmds.push_back(cmd);
		}
	}
	
	if(cmds.size() > 0){ // Push commands into the array in reverse order so that the original order is preserved
		std::vector<std::string>::iterator iter = cmds.end()-1;
		while(true){
			commands.Push(*iter);
			if(iter == cmds.begin()){ break; }
			else{ iter--; }
		}
	}
	
	input.close();
	return true;
}

bool Terminal::save_commands_(){
	std::ofstream output(cmd_filename.c_str());
	if(!output.good()){ return false; }
	
	std::string temp;
	unsigned int num_entries;
	if(commands.GetTotal() > commands.GetSize()){ num_entries = commands.GetSize(); }
	else{ num_entries = commands.GetTotal(); }
	
	for(unsigned int i = 0; i < num_entries; i++){
		temp = commands.GetPrev();
		if(temp != "NULL"){ // Again, just to be safe
			output << temp << std::endl;
		}
	}
	
	output.close();
	return true;
}

Terminal::Terminal() :
	_scrollbackBufferSize(SCROLLBACK_SIZE),
	_scrollPosition(0)	
{
	pbuf = NULL; 
	original = NULL;
	main = NULL;
	output_window = NULL;
	input_window = NULL;

	cmd_filename = "";
	init = false;
	save_cmds = false;
	text_length = 0;
	cursX = 0; 
	cursY = 0;
	offset = 0;
}

Terminal::~Terminal(){
	if(init){
		flush(); // Make sure no text is remaining in the buffer
		Close();
	}
}

void Terminal::Initialize(){
	if(init){ return; }
	
	original = std::cout.rdbuf(); // Back-up cout's streambuf
	pbuf = stream.rdbuf(); // Get stream's streambuf
	std::cout.flush();
	std::cout.rdbuf(pbuf); // Assign streambuf to cout
	
	main = initscr();
	
	if(main == NULL ){ // Attempt to initialize ncurses
		std::cout.rdbuf(original); // Restore cout's original streambuf
		fprintf(stderr, " Error: failed to initialize ncurses!\n");
	}
	else{		
   		getmaxyx(stdscr, _winSizeY, _winSizeX);
		output_window = newpad(_scrollbackBufferSize, _winSizeX);
		input_window = newpad(1, _winSizeX);
		wmove(output_window, _scrollbackBufferSize-1, 0); // Set the output cursor at the bottom so that new text will scroll up
		
		halfdelay(5); // Timeout after 5/10 of a second
		keypad(input_window, true); // Capture special keys
		noecho(); // Turn key echoing off
		
		scrollok(output_window, true);
		scrollok(input_window, true);

		if (NCURSES_MOUSE_VERSION > 0) {		
			mousemask(ALL_MOUSE_EVENTS,NULL);
			mouseinterval(0);
		}

		init = true;
		text_length = 0;
		offset = 0;
		
		// Set the position of the physical cursor
		cursX = 0; cursY = _winSizeY-1;
		update_cursor_();
		refresh_();

		init_colors_();
	}
	
	setup_signal_handlers();
}

void Terminal::Initialize(std::string cmd_fname_){
	if(init){ return; }
	
	Initialize();
	cmd_filename = cmd_fname_;
	save_cmds = true;
	load_commands_();
}

/// Set the command filename for storing previous commands
/// This command will clear all current commands from the history if overwrite_ is set to true
void Terminal::SetCommandFilename(std::string input_, bool overwrite_/*=false*/){
	if(save_cmds && !overwrite_){ return; }

	cmd_filename = input_;
	save_cmds = true;
	commands.Clear();
	load_commands_();
}

void Terminal::SetPrompt(const char *input_){
	offset = cstrlen(input_);
	in_print_(input_);
}

// Force a character to the output screen
void Terminal::putch(const char input_){
	waddch(output_window, input_);
	refresh_();
}

// Force text to the output screen
void Terminal::print(std::string input_){
	size_t pos = 0, lastPos = 0;
	//Search for escape sequences
	while ((pos = input_.find("\e[",lastPos)) != std::string::npos) {
		//Output the string from last location to current escape sequence
		waddstr(output_window, input_.substr(lastPos,pos-lastPos).c_str());
		lastPos = pos;

		//Identify which escape code we found.
		//First try reset code then loop through other codes
		if (pos == input_.find(TermColors::Reset,pos)) {
			wattrset(output_window, A_NORMAL);
			lastPos += std::string(TermColors::Reset).length();
		}
		else {
			for(auto it=attrMap.begin(); it!= attrMap.end(); ++it) {
				//If the attribute is at the same position then we found this attribute and we turn it on
				if (pos == input_.find(it->first,pos)) {
					wattron(output_window, it->second);
					//Iterate position to supress printing the escape string
					lastPos += std::string(it->first).length();
					break;
				}
			}
		}	
	}
	//Print the remaining string content
	waddstr(output_window, input_.substr(lastPos).c_str());
	refresh_();
}

// Dump all text in the stream to the output screen
void Terminal::flush(){
	std::string stream_contents = stream.str();
	if(stream_contents.size() > 0){
		print(stream_contents);
		stream.str("");
		stream.clear();
	}
}

std::string Terminal::GetCommand(){
	int keypress;
	std::string output = "";
	while(true){
		if(SIGNAL_INTERRUPT){ // ctrl-c (SIGINT)
			SIGNAL_INTERRUPT = false;
			output = "CTRL_C";
			text_length = 0;
			break;
		}

		flush(); // If there is anything in the stream, dump it to the screen
		
		int keypress = wgetch(input_window);
		if(keypress == ERR){ continue; } // No key was pressed in the interval

		//print((" debug: " + to_str(keypress) + "\n").c_str());
	
		// Check for internal commands
		if(keypress == 10){ // Enter key (10)
			std::string temp_cmd = cmd.Get();
			if(temp_cmd != ""){ 
				if(temp_cmd != commands.PeekPrev()){ // Only save this command if it is different than the previous command
					commands.Push(temp_cmd);
				}
				output = temp_cmd;
				text_length = 0;
				_scrollPosition = 0;
				break;
			}
		} 
		else if(keypress == 4){ // ctrl-d (EOT)
			output = "CTRL_D";
			text_length = 0;
			clear_();
			break;
		}
		else if(keypress == 9){ } // Tab key (9)
		else if(keypress == KEY_UP){ // 259
			if(commands.GetIndex() == 0){ commands.Capture(cmd.Get()); }
			std::string temp_cmd = commands.GetPrev();
			if(temp_cmd != "NULL"){
				clear_();
				cmd.Set(temp_cmd);
				in_print_(cmd.Get().c_str());
				text_length = cmd.GetSize();
			}
		}
		else if(keypress == KEY_DOWN){ // 258
			std::string temp_cmd = commands.GetNext();
			if(temp_cmd != "NULL"){
				clear_();
				cmd.Set(temp_cmd);
				in_print_(cmd.Get().c_str());
				text_length = cmd.GetSize();
			}
		}
		else if(keypress == KEY_LEFT){ cursX--; } // 260
		else if(keypress == KEY_RIGHT){ cursX++; } // 261
		else if(keypress == KEY_PPAGE){ //Page up key
			scroll_(-(_winSizeY-2));
		}
		else if(keypress == KEY_NPAGE){ //Page down key
			scroll_(_winSizeY-2);
		}
		else if(keypress == KEY_BACKSPACE){ // 263
			wmove(input_window, 0, --cursX);
			wdelch(input_window);
			cmd.Pop(cursX - offset);
			text_length = cmd.GetSize();
		}
		else if(keypress == KEY_DC){ // Delete character (330)
			//Remove character from terminal
			wdelch(input_window);
			//Remove character from cmd string
			cmd.Pop(cursX - offset);
			text_length = cmd.GetSize();
		}
		else if(keypress == KEY_IC){ cmd.ToggleInsertMode(); } // Insert key (331)
		else if(keypress == KEY_HOME){ cursX = offset; }
		else if(keypress == KEY_END){ cursX = text_length + offset; }
		else if(keypress == KEY_MOUSE) { //Handle mouse events
			MEVENT mouseEvent;
			//Get information about mouse event.
			getmouse(&mouseEvent);
			
			switch (mouseEvent.bstate) {
				//Scroll up
				case BUTTON4_PRESSED:
					scroll_(-3);
					break;
				//Scroll down. (Yes the name is strange.)
				case REPORT_MOUSE_POSITION:
					scroll_(3);
					break;
			}
		}	
		else if(keypress == KEY_RESIZE) {
			//Get rid of resize key
			continue;
		}
		else{ 
			in_char_((char)keypress); 
			cmd.Put((char)keypress, cursX - offset - 1);
			text_length = cmd.GetSize();
			continue;
		}
		
		// Check for cursor too far to the left
		if(cursX < offset){ cursX = offset; }
	
		// Check for cursor too far to the right
		if(cursX > (text_length + offset)){ cursX = text_length + offset; }
	
		update_cursor_();
		refresh_();
	}
	clear_();
	return output;
}

// Close the window and return control to the terminal
void Terminal::Close(){
	if(init){
		std::cout.rdbuf(original); // Restore cout's original streambuf
		delwin(output_window); // Delete the output window
		delwin(input_window); // Delete the input window
		delwin(main); // Delete the main window
		endwin(); // Restore Terminal settings
		
		if(save_cmds){ save_commands_(); }
		init = false;
	}
}

#endif
