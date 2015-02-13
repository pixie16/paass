// CTerminal.cpp
// Cory R. Thornsberry
// Feb. 2nd, 2015

// Library to facilitate the creation of C++ executables with
// interactive command line interfaces under a linux environment.

#include <iostream>
#include <unistd.h>

#ifdef USE_NCURSES

#include <signal.h>
#include <stdexcept>

#endif

#include "CTerminal.h"

#include "TermColors.h"

#define CTERMINAL_VERSION "1.1.0";

#ifdef USE_NCURSES

bool SIGNAL_INTERRUPT = false;

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

/* Parse all command line entries and find valid options. */
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

/* Return the length of a character string. */
unsigned int cstrlen(const char *str_){
	unsigned int output = 0;
	while(true){
		if(str_[output] == '\0'){ break; }
		output++;
	}
	return output;
}

/* Extract a string from a character array. */
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

void CommandHolder::Push(const std::string &input_){
	commands[index] = input_;
	total++;
	index++;
	external_index = 0;
	
	if(index >= max_size){ index = 0; }
}

void CommandHolder::Clear(){
	for(unsigned int i = 0; i < max_size; i++){
		commands[i] = "";
	}
}

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

// Get the previous command entry
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

// Get the next command entry
std::string CommandHolder::GetNext(){
	if(total == 0){ return "NULL"; }

	if(external_index >= 1){
		external_index--;
	}

	if(external_index == 0){ return fragment; }
	return commands[wrap_()]; 
}

void CommandHolder::Dump(){
	for(unsigned int i = 0; i < max_size; i++){
		std::cout << " " << i << ": " << commands[i] << std::endl;
	}
}

///////////////////////////////////////////////////////////////////////////////
// CommandString
///////////////////////////////////////////////////////////////////////////////

void CommandString::Put(const char ch_, int index_){
	if(index_ < 0){ return; }
	else if(index_ < command.size()){ // Overwrite or insert a character
		if(!insert_mode){ command.at(index_) = ch_; } // Overwrite
		else{ command.insert(index_, 1, ch_); } // Insert
	}
	else{ command.push_back(ch_); } // Appending to the back of the string
}

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

// Setup the interrupt signal intercept
void setup_signal_handlers(){ 
	if(signal(SIGINT, SIG_IGN) != SIG_IGN){	
		if(signal(SIGINT, sig_int_handler) == SIG_ERR){
			throw std::runtime_error(" Error setting up signal handler!");
		}
	}
}

void Terminal::refresh_(){
	if(!init){ return; }
	wrefresh(output_window);
	wrefresh(input_window);
	refresh();
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
	waddch(input_window, input_);
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

Terminal::Terminal(){
	output_window = NULL;
	input_window = NULL;
	init = false;
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
		int height, width;
   		getmaxyx(stdscr, height, width);
		output_window = newwin(height-1, width, 0, 0);
		input_window = newwin(1, width, height-1, 0);
		wmove(output_window, height-2, 0); // Set the output cursor at the bottom so that new text will scroll up
		
		halfdelay(5); // Timeout after 5/10 of a second
		keypad(input_window, true); // Capture special keys
		noecho(); // Turn key echoing off
		
		scrollok(output_window, true);
		scrollok(input_window, true);
		
		init = true;
		text_length = 0;
		offset = 0;
		
		// Set the position of the physical cursor
		cursX = 0; cursY = height-1;
		update_cursor_();
		refresh_();

		init_colors_();
	}
	
	setup_signal_handlers();
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
			commands.Push(cmd.Get());
			output = cmd.Get();
			text_length = 0;
			break;
		} 
		else if(keypress == 4){ // ctrl-d (EOT)
			output = "CTRL_D";
			text_length = 0;
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
		else if(keypress == KEY_BACKSPACE){ // 263
			wmove(input_window, 0, --cursX);
			wdelch(input_window);
			cmd.Pop(cursX - offset);
			text_length = cmd.GetSize();
		}
		else if(keypress == KEY_DC){ // Delete character (330)
			cursX--;
			wdelch(input_window);
		}
		else if(keypress == KEY_IC){ cmd.ToggleInsertMode(); } // Insert key (331)
		else if(keypress == KEY_HOME){ cursX = offset; }
		else if(keypress == KEY_END){ cursX = text_length + offset; }
		else{ 
			in_char_((char)keypress); 
			cmd.Put((char)keypress, cursX - offset - 1);
			if(cursX >= text_length + offset){ text_length++; }
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
		init = false;
	}
}

#endif
