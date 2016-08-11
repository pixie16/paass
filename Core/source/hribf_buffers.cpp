/** \file hribf_buffers.cpp
  * 
  * \brief Handles poll2 output data files
  * 
  * The classes within this file are used to open, format,
  * and properly write data to an hribf style ldf file so
  * that it may be read by programs which read legacy ldf
  * files. Each individual buffer class (inheriting from the
  * BufferType class) is responsible for writting a different
  * type of buffer to the data file. PollOutputFile is the
  * class used to stitch all of the individual buffers together
  * into a correctly formatted output ldf file which may be
  * read by SCANOR from the hhirf upak library.
  *
  * \author Cory R. Thornsberry
  * 
  * \date Aug. 9th, 2016
  * 
  * \version 1.2.15
*/

#include <sstream>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <iomanip>
#include <vector>

#include "hribf_buffers.h"
#include "poll2_socket.h"

#define HEAD 1145128264 /// Run begin buffer
#define DATA 1096040772 /// Physics data buffer
#define SCAL 1279345491 /// Scaler type buffer
#define DEAD 1145128260 /// Deadtime buffer
#define DIR 542263620   /// "DIR "
#define PAC 541278544   /// "PAC "
#define ENDFILE 541478725 /// End of file buffer
#define ENDBUFF 0xFFFFFFFF /// End of buffer marker

const unsigned int end_spill_size = 20; /// The size of the end of spill "event" (5 words).
const unsigned int pacman_word1 = 2; /// Words to signify the end of a spill. The scan code searches for these words.
const unsigned int pacman_word2 = 9999; /// End of spill vln. The scan code searches for these words.

///
void set_char_array(const std::string &input_, char *arr_, const unsigned int &size_){
	size_t size_to_copy = size_;
	if(size_ > input_.size()){ size_to_copy = input_.size(); }
	for(unsigned int i = 0; i < size_; i++){
		if(i < size_to_copy){ arr_[i] = input_[i]; }
		else{ arr_[i] = ' '; }
	}
	arr_[size_] = '\0';
}

/// Return true if the input word corresponds to the header of a ldf style buffer.
bool is_hribf_buffer(const unsigned int &input_){
	return (input_==HEAD || input_==DATA || input_==SCAL || input_==DEAD || input_==DIR || input_==PAC || input_==ENDFILE);
}

/// Generic BufferType constructor.
BufferType::BufferType(unsigned int bufftype_, unsigned int buffsize_, unsigned int buffend_/*=0xFFFFFFFF*/){
	bufftype = bufftype_; 
	buffsize = buffsize_; 
	buffend = buffend_; 
	zero = 0;
	debug_mode = false;
	Reset();
}

/// Returns only false if not overwritten
bool BufferType::Write(std::ofstream *file_){
	return false;
}

/// Returns only false if not overwritten
bool BufferType::Read(std::ifstream *file_){
	return false;
}

/// Return true if the first word of the current buffer is equal to this buffer type
bool BufferType::ReadHeader(std::ifstream *file_){
	unsigned int check_bufftype;	
	file_->read((char*)&check_bufftype, 4);
	if(check_bufftype != bufftype){ // Not a valid buffer
		return false;
	}
	return true;
}

/// Default constructor.
PLD_header::PLD_header() : BufferType(HEAD, 0){ // 0x44414548 "HEAD"
	Reset();
}

/// Destructor.
PLD_header::~PLD_header(){
	if(run_title){ delete[] run_title; }
}

/// Get the length of the header buffer.
unsigned int PLD_header::GetBufferLength(){
	unsigned int buffer_len = 100;
	buffer_len += strlen(run_title);
	while(buffer_len % 4 != 0){ buffer_len++; }
	return buffer_len;
}

/// Set the date and tiem of when the file is opened.
void PLD_header::SetStartDateTime(){
	time_t rawtime;
	time (&rawtime);
	
	char *date_holder = ctime(&rawtime);
	set_char_array(std::string(date_holder), start_date, 24); // Strip the trailing newline character
}

/// Set the date and time of when the file is closed.
void PLD_header::SetEndDateTime(){
	time_t rawtime;
	time (&rawtime);
	
	char *date_holder = ctime(&rawtime);
	set_char_array(std::string(date_holder), end_date, 24); // Strip the trailing newline character
}

/// Set the facility of the output pld file (max length 16).
void PLD_header::SetFacility(std::string input_){
	set_char_array(input_, facility, 16);
}
	
/// Set the title of the output pld file (unlimited length).
void PLD_header::SetTitle(std::string input_){
	if(run_title){ delete[] run_title; }
	run_title = new char[input_.size()+1];
	for(unsigned int i = 0; i < input_.size(); i++){
		run_title[i] = input_[i];
	}
	run_title[input_.size()] = '\0';
}

/// Write a pld style header to a file.
bool PLD_header::Write(std::ofstream *file_){
	if(!file_ || !file_->is_open() || !file_->good()){ return false; }
	
	unsigned int len_of_title = strlen(run_title);
	unsigned int padding_bytes = 0;
	while((len_of_title + padding_bytes) % 4 != 0){ 
		padding_bytes++; 
	}
	unsigned int total_title_len = len_of_title + padding_bytes;
	
	if(debug_mode){ std::cout << "debug: writing " << 100 + total_title_len << " byte HEAD buffer\n"; }
	
	file_->write((char*)&bufftype, 4);
	file_->write((char*)&run_num, 4);
	file_->write((char*)&max_spill_size, 4);
	file_->write((char*)&run_time, 4);
	file_->write(format, 16);
	file_->write(facility, 16);
	file_->write(start_date, 24);
	file_->write(end_date, 24);
	file_->write((char*)&total_title_len, 4);
	file_->write(run_title, len_of_title);
	
	char padding = ' ';
	for(unsigned int i = 0; i < padding_bytes; i++){
		file_->write(&padding, 1);
	}
	
	file_->write((char*)&buffend, 4); // Close the buffer
	
	return true;
}

/// Read a pld style header from a file.
bool PLD_header::Read(std::ifstream *file_){
	if(!file_ || !file_->is_open() || !file_->good()){ return false; }
	
	unsigned int check_bufftype;	
	file_->read((char*)&check_bufftype, 4);
	if(check_bufftype != bufftype){ // Not a valid HEAD buffer
		if(debug_mode){ std::cout << "debug: not a valid HEAD buffer\n"; }
		file_->seekg(-4, file_->cur); // Rewind to the beginning of this buffer
		return false; 
	}

	unsigned int end_buff_check;
	unsigned int len_of_title;
	file_->read((char*)&run_num, 4);
	file_->read((char*)&max_spill_size, 4);
	file_->read((char*)&run_time, 4);
	file_->read(format, 16); format[16] = '\0';
	file_->read(facility, 16); facility[16] = '\0';
	file_->read(start_date, 24); start_date[24] = '\0';
	file_->read(end_date, 24); end_date[24] = '\0';
	file_->read((char*)&len_of_title, 4);
	
	if(run_title){ delete[] run_title; }
	run_title = new char[len_of_title+1];
	
	file_->read(run_title, len_of_title); run_title[len_of_title] = '\0';
	file_->read((char*)&end_buff_check, 4);
	
	if(end_buff_check != buffend){ // Buffer was not terminated properly
		if(debug_mode){ std::cout << "debug: buffer not terminated properly\n"; }
		return false;
	}

	return true;
}

/// Set initial values.
void PLD_header::Reset(){
	set_char_array("U OF TENNESSEE  ", facility, 16);
	set_char_array("PIXIE LIST DATA ", format, 16);
	set_char_array("Mon Jan 01 00:00:00 2000", start_date, 24);
	set_char_array("Mon Jan 01 00:00:00 2000", end_date, 24);
	max_spill_size = 0;
	run_title = NULL;
	run_time = 0.0;
	run_num = 0;
}

/// Print header information.
void PLD_header::Print(){
	std::cout << " 'HEAD' buffer-\n";
	std::cout << "  Facility: " << std::string(facility) << "\n";
	std::cout << "  Format: " << std::string(format) << "\n";
	std::cout << "  Start: " << std::string(start_date) << "\n";
	std::cout << "  Stop: " << std::string(end_date) << "\n";
	std::cout << "  Title: " << std::string(run_title) << "\n";
	std::cout << "  Run number: " << run_num << "\n";
	std::cout << "  Max spill: " << max_spill_size << " words\n";
	std::cout << "  ACQ time: " << run_time << " seconds\n";	
}

/// Print header information in a delimited list.
void PLD_header::PrintDelimited(const char &delimiter_/*='\t'*/){
	std::cout << std::string(facility) << delimiter_;
	std::cout << std::string(format) << delimiter_;
	std::cout << std::string(start_date) << delimiter_;
	std::cout << std::string(end_date) << delimiter_;
	std::cout << std::string(run_title) << delimiter_;
	std::cout << run_num << delimiter_;
	std::cout << max_spill_size << delimiter_;
	std::cout << run_time;
}

/// Default constructor.
PLD_data::PLD_data() : BufferType(DATA, 0){ // 0x41544144 "DATA"
	Reset();
}

/// Write a pld style data buffer to file.
bool PLD_data::Write(std::ofstream *file_, char *data_, unsigned int nWords_){
	if(!file_ || !file_->is_open() || !file_->good() || nWords_ == 0){ return false; }
	
	if(debug_mode){ std::cout << "debug: writing spill of " << nWords_ << " words\n"; }
	
	file_->write((char*)&bufftype, 4);
	file_->write((char*)&nWords_, 4);
	file_->write(data_, 4*nWords_);
	
	file_->write((char*)&buffend, 4); // Close the buffer
	
	return true;
}

/// Read a pld style data buffer from file.
bool PLD_data::Read(std::ifstream *file_, char *data_, unsigned int &nBytes, unsigned int max_bytes_, bool dry_run_mode/*=false*/){
	if(!file_ || !file_->is_open() || !file_->good()){ return false; }

	unsigned int check_bufftype;	
	file_->read((char*)&check_bufftype, 4);
	if(check_bufftype != bufftype){ // Not a valid DATA buffer
		if(debug_mode){ std::cout << "debug: not a valid DATA buffer\n"; }

		unsigned int countw = 0;
		while(check_bufftype != bufftype){
			file_->read((char*)&check_bufftype, 4);
			if(file_->eof()){
				if(debug_mode){ std::cout << "debug: encountered physical end-of-file before start of spill!\n"; }
				return false;
			}
			countw++;
		}
		
		if(debug_mode){ std::cout << "debug: read an extra " << countw << " words to get to first DATA buffer!\n"; }
	}
	
	file_->read((char*)&nBytes, 4);
	nBytes = nBytes * 4;
	
	if(debug_mode){ std::cout << "debug: reading spill of " << nBytes << " bytes\n"; }
	
	if(nBytes > max_bytes_){
		if(debug_mode){ std::cout << "debug: spill size is greater than size of data array!\n"; }
		return false;
	}
	
	unsigned int end_buff_check;
	if(!dry_run_mode){ file_->read(data_, nBytes); }
	else{ file_->seekg(nBytes, std::ios::cur); }
	file_->read((char*)&end_buff_check, 4);
	
	if(end_buff_check != buffend){ // Buffer was not terminated properly
		if(debug_mode){ std::cout << "debug: buffer not terminated properly\n"; }
		return false;
	}

	return true;
}

/// Default constructor.
DIR_buffer::DIR_buffer() : BufferType(DIR, ACTUAL_BUFF_SIZE-2){ // 0x20524944 "DIR "
	Reset();
}
	
/** DIR buffer (1 word buffer type, 1 word buffer size, 1 word for total buffer length,
  * 1 word for total number of buffers, 2 unknown words, 1 word for run number, 1 unknown word,
  * and 8186 zeros).
  */
bool DIR_buffer::Write(std::ofstream *file_){
	if(!file_ || !file_->is_open() || !file_->good()){ return false; }
	
	if(debug_mode){ std::cout << "debug: writing " << ACTUAL_BUFF_SIZE*4 << " byte DIR buffer\n"; }
	
	file_->write((char*)&bufftype, 4);
	file_->write((char*)&buffsize, 4);
	file_->write((char*)&total_buff_size, 4);
	file_->write((char*)&zero, 4); // Will be overwritten later
	file_->write((char*)unknown, 8);
	file_->write((char*)&run_num, 4);
	file_->write((char*)&unknown[2], 4);
	
	// Fill the rest of the buffer with 0
	for(unsigned int i = 0; i < ACTUAL_BUFF_SIZE-8; i++){
		file_->write((char*)&zero, 4);
	}
	
	return true;
}

/// Read a ldf DIR buffer.
bool DIR_buffer::Read(std::ifstream *file_){
	if(!file_ || !file_->is_open() || !file_->good()){ return false; }
	
	unsigned int check_bufftype, check_buffsize;	
	file_->read((char*)&check_bufftype, 4);
	file_->read((char*)&check_buffsize, 4);
	if(check_bufftype != bufftype || check_buffsize != buffsize){ // Not a valid DIR buffer
		if(debug_mode){ std::cout << "debug: not a valid DIR buffer\n"; }
		file_->seekg(-8, file_->cur); // Rewind to the beginning of this buffer
		return false; 
	}
	
	file_->read((char*)&total_buff_size, 4);
	file_->read((char*)&total_buff_size, 4);
	file_->read((char*)unknown, 8);
	file_->read((char*)&run_num, 4);
	file_->seekg((buffsize*4 - 20), file_->cur); // Skip the rest of the buffer

	return true;
}

/// Set initial values.
void DIR_buffer::Reset(){
	total_buff_size = ACTUAL_BUFF_SIZE;
	run_num = 0;
	unknown[0] = 0;
	unknown[1] = 1;
	unknown[2] = 2;
}

/// Print dir buffer information.
void DIR_buffer::Print(){
	std::cout << " 'DIR ' buffer-\n";
	std::cout << "  Run number: " << run_num << "\n";
	std::cout << "  Number buffers: " << total_buff_size << "\n";
}

/// Print dir buffer information in a delimited list.
void DIR_buffer::PrintDelimited(const char &delimiter_/*='\t'*/){
	std::cout << run_num << delimiter_;
	std::cout << total_buff_size;
}

/// Default constructor.
HEAD_buffer::HEAD_buffer() : BufferType(HEAD, 64){ // 0x44414548 "HEAD"
	Reset();
}

/// Set the date and time of the ldf file.
bool HEAD_buffer::SetDateTime(){
	struct tm * local;
	time_t temp_time;
	time(&temp_time);
	local = localtime(&temp_time);
	
	int month = local->tm_mon+1;
	int day = local->tm_mday;
	int year = local->tm_year;
	int hour = local->tm_hour;
	int minute = local->tm_min;
	//int second = local->tm_sec;
	
	std::stringstream stream;
	(month<10) ? stream << "0" << month << "/" : stream << month << "/";
	(day<10) ? stream << "0" << day << "/" : stream << day << "/";
	(year<110) ? stream << "0" << year-100 << " " : stream << year-100 << " ";
	
	(hour<10) ? stream << "0" << hour << ":" : stream << hour << ":";
	(minute<10) ? stream << "0" << minute << "  " : stream << minute << "  ";
	//stream << ":"
	//(second<10) ? stream << "0" << second : stream << second;
	
	std::string dtime_str = stream.str();
	if(dtime_str.size() > 16){ return false; }
	for(unsigned int i = 0; i < 16; i++){
		if(i >= dtime_str.size()){ date[i] = ' '; }
		else{ date[i] = dtime_str[i]; }
	}
	return true;
}
	
/// Set the title of the ldf file.
bool HEAD_buffer::SetTitle(std::string input_){
	for(unsigned int i = 0; i < 80; i++){
		if(i >= input_.size()){ run_title[i] = ' '; }
		else{ run_title[i] = input_[i]; }
	}
	return true;
}

/** ldf style HEAD buffer (1 word buffer type, 1 word buffer size, 2 words for facility, 2 for format, 
  * 3 for type, 1 word separator, 4 word date, 20 word title [80 character], 1 word run number,
  * 30 words of padding, and 8129 end of buffer words).
  */
bool HEAD_buffer::Write(std::ofstream *file_){
	if(!file_ || !file_->is_open() || !file_->good()){ return false; }
	
	if(debug_mode){ std::cout << "debug: writing " << ACTUAL_BUFF_SIZE*4 << " byte HEAD buffer\n"; }
	
	// write 140 bytes (35 words)
	file_->write((char*)&bufftype, 4);
	file_->write((char*)&buffsize, 4);
	file_->write(facility, 8);
	file_->write(format, 8);
	file_->write(type, 16);
	file_->write(date, 16);
	file_->write(run_title, 80);
	file_->write((char*)&run_num, 4);
		
	// Get the buffer length up to 256 bytes (add 29 words)
	char temp[116];
	for(unsigned int i = 0; i < 116; i++){ temp[i] = 0x0; }
	file_->write(temp, 116);
		
	// Fill the rest of the buffer with 0xFFFFFFFF (end of buffer)
	for(unsigned int i = 0; i < ACTUAL_BUFF_SIZE-64; i++){
		file_->write((char*)&buffend, 4);
	}
	
	return true;
}

/// Read a ldf HEAD buffer.
bool HEAD_buffer::Read(std::ifstream *file_){
	if(!file_ || !file_->is_open() || !file_->good()){ return false; }
	
	unsigned int check_bufftype, check_buffsize;	
	file_->read((char*)&check_bufftype, 4);
	file_->read((char*)&check_buffsize, 4);
	if(check_bufftype != bufftype || check_buffsize != buffsize){ // Not a valid HEAD buffer
		if(debug_mode){ std::cout << "debug: not a valid HEAD buffer\n"; }
		file_->seekg(-8, file_->cur); // Rewind to the beginning of this buffer
		return false; 
	}
	
	file_->read(facility, 8); facility[8] = '\0';
	file_->read(format, 8); format[8] = '\0';
	file_->read(type, 16); type[16] = '\0';
	file_->read(date, 16); date[16] = '\0';
	file_->read(run_title, 80); run_title[80] = '\0';
	file_->read((char*)&run_num, 4);
	file_->seekg((ACTUAL_BUFF_SIZE*4 - 140), file_->cur); // Skip the rest of the buffer

	return true;
}

/// Set initial values.
void HEAD_buffer::Reset(){
	set_char_array("HHIRF   ", facility, 8);
	set_char_array("L003    ", format, 8);
	set_char_array("LIST DATA       ", type, 16);
	set_char_array("01/01/01 00:00  ", date, 16);
	run_num = 0;
}

/// Print header information.
void HEAD_buffer::Print(){
	std::cout << " 'HEAD' buffer-\n";
	std::cout << "  Facility: " << std::string(facility) << "\n";
	std::cout << "  Format: " << std::string(format) << "\n";
	std::cout << "  Type: " << std::string(type) << "\n";
	std::cout << "  Date: " << std::string(date) << "\n";
	std::cout << "  Title: " << std::string(run_title) << "\n";
	std::cout << "  Run number: " << run_num << "\n";	
}

/// Print header information in a delimited list.
void HEAD_buffer::PrintDelimited(const char &delimiter_/*='\t'*/){
	std::cout << std::string(facility) << delimiter_;
	std::cout << std::string(format) << delimiter_;
	std::cout << std::string(type) << delimiter_;
	std::cout << std::string(date) << delimiter_;
	std::cout << std::string(run_title) << delimiter_;
	std::cout << run_num;
}

/// Write a ldf data buffer header (2 words).
bool DATA_buffer::open_(std::ofstream *file_){
	if(!file_ || !file_->is_open() || !file_->good()){ return false; }

	if(debug_mode){ std::cout << "debug: writing 2 word DATA header\n"; }
	file_->write((char*)&bufftype, 4); // write buffer header type
	file_->write((char*)&buffsize, 4); // write buffer size
	buff_pos = 2;

	return true;
}

/// 
bool DATA_buffer::read_next_buffer(std::ifstream *f_, bool force_/*=false*/){
	if(!f_ || !f_->good() || f_->eof()){ return false; }

	if(bcount == 0){
		f_->read((char *)buffer1, ACTUAL_BUFF_SIZE*4);
	}
	else if(buff_pos + 3 <= ACTUAL_BUFF_SIZE-1 && !force_){
		// Don't need to scan a new buffer yet. There are still
		// words remaining in the one currently in memory.
			
		// Skip end of event delimiters.
		while(curr_buffer[buff_pos] == ENDBUFF && buff_pos < ACTUAL_BUFF_SIZE-1){
			buff_pos++;
		}
			
		// If we have more good words in this buffer, keep reading it.
		if(buff_pos + 3 < ACTUAL_BUFF_SIZE-1){ 
			return true; 
		}
	}
	
	// Read the buffer into memory.
	if(bcount % 2 == 0){
		f_->read((char *)buffer2, ACTUAL_BUFF_SIZE*4);
		curr_buffer = buffer1;
		next_buffer = buffer2;
	}
	else{
		f_->read((char *)buffer1, ACTUAL_BUFF_SIZE*4);
		curr_buffer = buffer2;
		next_buffer = buffer1;
	}
	
	// Reset the buffer index.
	buff_pos = 0;
	
	// Increment the number of buffers read.
	bcount++;
	
	// Read the buffer header and length.
	buff_head = curr_buffer[buff_pos++];
	buff_size = curr_buffer[buff_pos++];

	if(!f_->good()){ return false; }
	else if(f_->eof()){ retval = 2; }
	
	return true; 
}

/// Default constructor.
DATA_buffer::DATA_buffer() : BufferType(DATA, ACTUAL_BUFF_SIZE-2){ // 0x41544144 "DATA"
	Reset();
}

/// Close a ldf data buffer by padding with 0xFFFFFFFF.
bool DATA_buffer::Close(std::ofstream *file_){
	if(!file_ || !file_->is_open() || !file_->good()){ return false; }

	if(buff_pos < ACTUAL_BUFF_SIZE){
		if(debug_mode)
			std::cout << "debug: closing buffer with " << ACTUAL_BUFF_SIZE - buff_pos << " 0xFFFFFFFF words\n";
		for(unsigned int i = buff_pos; i < ACTUAL_BUFF_SIZE; i++){
			file_->write((char*)&buffend, 4); 
		}
	}
	buff_pos = 0;

	return true;
}

/// Write a ldf data buffer to disk.
bool DATA_buffer::Write(std::ofstream *file_, char *data_, unsigned int nWords_, int &buffs_written){
	if(!file_ || !file_->is_open() || !file_->good() || !data_ || nWords_ == 0){ 
		if(debug_mode){ std::cout << "debug: !file_ || !file_->is_open() || !data_ || nWords_ == 0\n"; }	
		return false; 
	}

	buffs_written = 0;

	// If this is a new buffer, write a buffer header.
	if(buff_pos == 0)
		open_(file_);

	unsigned int spillpos = 0;
	unsigned int chunkPayload;
	
	unsigned int oldBuffPos = buff_pos;
	unsigned int chunkSizeB;
	unsigned int totalNumChunks = 0;
	unsigned int currentNumChunk = 0;
	
	char *arrptr = data_;

	// Calculate the total number of spill chunks.
	while(spillpos < nWords_){
		if(oldBuffPos + 4 > ACTUAL_BUFF_SIZE)
			oldBuffPos = 2;
			
		if(nWords_ - spillpos + 4 > ACTUAL_BUFF_SIZE - oldBuffPos)
			chunkPayload = ACTUAL_BUFF_SIZE - oldBuffPos - 4;
		else
			chunkPayload = nWords_ - spillpos;
		
		spillpos += chunkPayload;
		oldBuffPos += chunkPayload + 4;
		
		totalNumChunks++;
		
		if(oldBuffPos >= ACTUAL_BUFF_SIZE)
			oldBuffPos = 2;
	}
	
	// Account for the spill footer.
	totalNumChunks++;
	
	// Reset the spill position.
	spillpos = 0;
	
	// Write the spill.
	while(spillpos < nWords_){
		if(buff_pos + 4 > ACTUAL_BUFF_SIZE){
			buffs_written++;
			Close(file_);
			open_(file_);
		}
		
		if(nWords_ - spillpos + 4 > ACTUAL_BUFF_SIZE - buff_pos)
			chunkPayload = ACTUAL_BUFF_SIZE - buff_pos - 4;
		else
			chunkPayload = nWords_ - spillpos;
		
		spillpos += chunkPayload;
		chunkSizeB = 4*(chunkPayload + 3);
		
		if(debug_mode) 
			std::cout << "debug: writing " << 1+chunkSizeB/4 << " word spill chunk " << currentNumChunk << " of " << totalNumChunks << ".\n";
		
		file_->write((char*)&chunkSizeB, 4);
		file_->write((char*)&totalNumChunks, 4);
		file_->write((char*)&currentNumChunk, 4);
		file_->write((char*)arrptr, 4*chunkPayload);
		file_->write((char*)&buffend, 4);
		
		currentNumChunk++;
		
		buff_pos += chunkPayload + 4;
		arrptr += 4*chunkPayload;
		
		if(buff_pos > ACTUAL_BUFF_SIZE)
			std::cout << "WARNING: Current ldf buffer overfilled by " << buff_pos - ACTUAL_BUFF_SIZE << " words!\n";
		
		if(buff_pos >= ACTUAL_BUFF_SIZE){
			buffs_written++;
			open_(file_);
		}
	}

	if(buff_pos > ACTUAL_BUFF_SIZE)
		std::cout << "WARNING: Final ldf buffer overfilled by " << buff_pos - ACTUAL_BUFF_SIZE << " words!\n";

	// Write the spill footer.
	if(buff_pos + 6 > ACTUAL_BUFF_SIZE){
		buffs_written++;
		Close(file_);
		open_(file_);
	}
	
	if(debug_mode)
		std::cout << "debug: writing final spill chunk " << currentNumChunk << " of " << 1+end_spill_size/4 << " words.\n";
		
	file_->write((char*)&end_spill_size, 4);
	file_->write((char*)&totalNumChunks, 4);
	file_->write((char*)&currentNumChunk, 4);
	file_->write((char*)&pacman_word1, 4);
	file_->write((char*)&pacman_word2, 4);
	file_->write((char*)&buffend, 4);

	// Update the buffer position.
	buff_pos += 6;

	return true;
}

/// Read a ldf data spill from a file.
bool DATA_buffer::Read(std::ifstream *file_, char *data_, unsigned int &nBytes, unsigned int max_bytes_, bool &full_spill, bool &bad_spill, bool dry_run_mode/*=false*/){
	if(!file_ || !file_->is_open() || !file_->good()){ 
		retval = 6;
		return false; 
	}
	
	bad_spill = false;

	bool first_chunk = true;
	unsigned int this_chunk_sizeB;
	unsigned int total_num_chunks = 0;
	unsigned int current_chunk_num = 0;
	unsigned int prev_chunk_num;
	unsigned int prev_num_chunks;
	nBytes = 0; // Set the number of output bytes to zero

	while(true){
		if(!read_next_buffer(file_)){ 
			if(debug_mode){ std::cout << "debug: failed to read from input data file\n"; }
			retval = 6;
			return false;
		}
	
		if(buff_head == DATA){
			prev_chunk_num = current_chunk_num;
			prev_num_chunks = total_num_chunks;
			
			this_chunk_sizeB = curr_buffer[buff_pos++];
			total_num_chunks = curr_buffer[buff_pos++];
			current_chunk_num = curr_buffer[buff_pos++];
			
			if(debug_mode){ std::cout << "debug: scanning spill chunk " << current_chunk_num << " of " << total_num_chunks << "\n"; }
			
			// Check if this is a spill fragment.
			if(first_chunk){ // Check for starting read in middle of spill.
				if(current_chunk_num != 0){
					if(debug_mode){ std::cout << "debug: starting read in middle of spill (chunk " << current_chunk_num << " of " << total_num_chunks << ")\n"; }		
					
					// Update the number of dropped chunks.
					missing_chunks += current_chunk_num;
						
					full_spill = false;
				}
				else{ full_spill = true; }
				first_chunk = false;
			}
			else if(total_num_chunks != prev_num_chunks){
				if(debug_mode){ std::cout << "debug: skipped to new spill with " << total_num_chunks << " spill chunks without reading footer of old spill\n"; }
				
				// We are likely out of position in the spill. Scrap this current buffer and skip to the next one.
				read_next_buffer(file_, true);
				
				// Update the number of dropped chunks.
				missing_chunks += (prev_num_chunks-1) - prev_chunk_num;
				
				retval = 4;
				return false;
			}
			else if(current_chunk_num != prev_chunk_num + 1){ // Missing a spill chunk.
				if(debug_mode){ 
					if(current_chunk_num == prev_chunk_num + 2){ std::cout << "debug: missing single spill chunk (" << prev_chunk_num+1 << ")\n"; }
					else{ std::cout << "debug: missing multiple spill chunks (" << prev_chunk_num+1 << " to " << current_chunk_num-1 << ")\n"; }
				}
				full_spill = false;
				
				// We are likely out of position in the spill. Scrap this current buffer and skip to the next one.
				read_next_buffer(file_, true);
				
				// Update the number of dropped chunks.
				missing_chunks += (current_chunk_num-1) - prev_chunk_num;
				
				retval = 4;
				return false;
			}

			// Construct the spill.	
			if(current_chunk_num == total_num_chunks - 1){ // Spill footer
				if(this_chunk_sizeB != end_spill_size){
					if(debug_mode){ std::cout << "debug: spill footer (chunk " << current_chunk_num << " of " << total_num_chunks << ") has size " << this_chunk_sizeB << " != 5\n"; }
					
					// We are likely out of position in the spill. Scrap this current buffer and skip to the next one.
					read_next_buffer(file_, true);
					
					// Update the number of dropped chunks.
					//missing_chunks += 1;
					
					retval = 5;
					return false;
				}
				else if(debug_mode){ 
					if(full_spill){ std::cout << "debug: finished scanning spill of " << nBytes << " bytes\n"; }
					else{ std::cout << "debug: finished scanning spill fragment of " << nBytes << " bytes\n"; }
				}
			
				// Copy data into the output array.
				if(!dry_run_mode){ memcpy(&data_[nBytes], &curr_buffer[buff_pos], 8); }
				if(debug_mode){ std::cout << "debug: spill footer words are " << curr_buffer[buff_pos] << " and " << curr_buffer[buff_pos+1] << std::endl; }
				nBytes += 8;
				buff_pos += 2;

				retval = 0;
				return true;
			}
			else{ // Normal spill chunk
				unsigned int copied_bytes;
				if(this_chunk_sizeB <= 12){
					if(debug_mode){ std::cout << "debug: invalid number of bytes in chunk " << current_chunk_num+1 << " of " << total_num_chunks << ", " <<  this_chunk_sizeB << " B!\n"; }
					
					// Update the number of dropped chunks.
					missing_chunks++;
					
					retval = 4;
					return false;
				}
			
				// Update the number of good spill chunks.
				good_chunks++;
			
				copied_bytes = this_chunk_sizeB - 12;
				if(!dry_run_mode){ memcpy(&data_[nBytes], &curr_buffer[buff_pos], copied_bytes); }
				nBytes += copied_bytes;
				buff_pos += copied_bytes/4;
			}
		}
		else if(buff_head == ENDFILE){ // Catch the start of the end-of-file buffer and return
			if(next_buffer[0] == ENDFILE){
				if(debug_mode){ std::cout << "debug: encountered double EOF buffer marking end of file\n"; }
				retval = 2;
			}
			else{
				if(debug_mode){ std::cout << "debug: encountered EOF buffer marking end of run\n"; }
				
				// We need to skip this buffer.
				read_next_buffer(file_, true);
				
				retval = 1;
			}
			return false;
		}
		else{ // Skip the entire buffer
			if(debug_mode){ 
				std::cout << "debug: encountered non DATA type buffer 0x" << std::hex << buff_head << std::dec << "\n";
				std::cout << "debug: skipping " << ACTUAL_BUFF_SIZE << " words of buffer!\n";
			}
			
			// This is not a data buffer. We need to force a scan of the next buffer.
			// read_next_buffer will not scan the next buffer by default because it
			// thinks there are still words left in this buffer to read.
			read_next_buffer(file_, true);
			
			retval = 3;
			continue;
		}
	}

	return false;
}

/// Set initial values.
void DATA_buffer::Reset(){
	curr_buffer = buffer1;
	next_buffer = buffer2;
	buff_pos = 0;
	bcount = 0;
	retval = 0;
	good_chunks = 0;
	missing_chunks = 0;
}

EOF_buffer::EOF_buffer() : BufferType(ENDFILE, ACTUAL_BUFF_SIZE-2){ // 0x20464F45 "EOF "
	Reset();
}

/// Write an end-of-file buffer (1 word buffer type, 1 word buffer size, and 8192 end of file words).
bool EOF_buffer::Write(std::ofstream *file_){
	if(!file_ || !file_->is_open() || !file_->good()){ return false; }
	
	if(debug_mode){ std::cout << "debug: writing " << ACTUAL_BUFF_SIZE*4 << " byte EOF buffer\n"; }
	
	// write 8 bytes (2 words)
	file_->write((char*)&bufftype, 4);
	file_->write((char*)&buffsize, 4);
	
	// Fill the rest of the buffer with 0xFFFFFFFF (end of buffer)
	for(unsigned int i = 0; i < ACTUAL_BUFF_SIZE-2; i++){
		file_->write((char*)&buffend, 4);
	}
	
	return true;
}

/// Read an end-of-file buffer.
bool EOF_buffer::Read(std::ifstream *file_){
	if(!file_ || !file_->is_open() || !file_->good()){ return false; }

	unsigned int check_bufftype, check_buffsize;	
	file_->read((char*)&check_bufftype, 4);
	file_->read((char*)&check_buffsize, 4);
	if(check_bufftype != bufftype || check_buffsize != buffsize){ // Not a valid EOF buffer
		if(debug_mode){ std::cout << "debug: not a valid EOF buffer\n"; }
		file_->seekg(-8, file_->cur); // Rewind to the beginning of this buffer
		return false; 
	}
	
	file_->seekg((buffsize*4), file_->cur); // Skip the rest of the buffer
	
	return true;
}

/// Get the formatted filename of the current file.
std::string PollOutputFile::get_filename(){
	std::stringstream stream; stream << current_file_num;
	std::string run_num_str = stream.str();
	std::string output;
	
	if(current_file_num == 0){ output = fname_prefix; }
	else if(current_file_num < 10){ output = fname_prefix + "_0" + run_num_str; }
	else{ output = fname_prefix + "_" + run_num_str; }
	
	if(output_format == 0){ output += ".ldf"; }
	else if(output_format == 1){ output += ".pld"; }
	else{ output += ".root"; } // PLACEHOLDER!!!
	return output;
}

/// Get the full filename of the current file including path.
bool PollOutputFile::get_full_filename(std::string &output){
	unsigned int depth = (int)directories.size();
	
	// Get the depth of the current filename
	std::string formatted_filename = "";
	unsigned int index = 0;
	while(index < current_filename.size()){
		if(current_filename[index] == '.'){
			if(current_filename[index+1] == '.'){ // '../'
				if(depth > 0){ depth--; }
				else{ // user supplied a stupid filename
					output = current_filename;
					return false;
				} 
				index += 3;
			}
			else if(current_filename[index+1] == '/'){ // './'
				index += 2;
			}
			else{ 
				formatted_filename += current_filename[index]; 
				index++;
			}
		}
		else{ 
			formatted_filename += current_filename[index]; 
			index++;
		}
	}

	if(depth > 0){
		output = "/";
		for(unsigned int i = 0; i < depth; i++){
			output += directories.at(i) + "/";
		}
		output += formatted_filename;
	}
	else{ output = formatted_filename; }
	
	return true;
}

/** Overwrite the fourth word of the file with the total number of buffers and close the file.
  * Returns false if no output file is open or if the number of 4 byte words in the file is not 
  * evenly divisible by the number of words in a buffer.
  */
bool PollOutputFile::overwrite_dir(int total_buffers_/*=-1*/){
	if(!output_file.is_open() || !output_file.good()){ return false; }
	
	// Set the buffer count in the "DIR " buffer
	if(total_buffers_ == -1){ // Set with the internal buffer count
		unsigned int size = output_file.tellp(); // Get the length of the file (in bytes)
		output_file.seekp(12); // Set position to just after the third word
	
		// Calculate the number of buffers in this file
		unsigned int total_num_buffer = size / (4 * ACTUAL_BUFF_SIZE);
		unsigned int overflow = size % (4 * ACTUAL_BUFF_SIZE);
		output_file.write((char*)&total_num_buffer, 4); 
		
		if(debug_mode){ 
			std::cout << "debug: file size is " << size << " bytes (" << size/4 << " 4 byte words)\n";
			std::cout << "debug: file contains " << total_num_buffer << " buffers of " << ACTUAL_BUFF_SIZE << " words\n";
			if(overflow != 0){ std::cout << "debug: file has an overflow of " << overflow << " 4 byte words!\n"; }
			std::cout << "debug: set .ldf directory buffer number to " << total_num_buffer << std::endl; 
		}
		
		if(overflow != 0){ 
			output_file.close();
			return false; 
		}
	} 
	else{ // Set with an external buffer count
		output_file.write((char*)&total_buffers_, 4); 
		if(debug_mode){ std::cout << "debug: set .ldf directory buffer number to " << total_buffers_ << std::endl; }	
	}
	
	output_file.close();
	return true;
}

/// Initialize the output file with initial parameters
void PollOutputFile::initialize(){
	max_spill_size = -9999;
	current_file_num = 0; 
	output_format = 0;
	number_spills = 0;
	fname_prefix = "poll_data";
	current_filename = "unknown";
	current_full_filename = "unknown";
	debug_mode = false;
	
	// Get the current working directory
	// current_directory DOES NOT include a trailing '/'
	char ch_cwd[1024];
	current_directory = std::string(getcwd(ch_cwd, 1024));
	
	// Find the depth of the current directory
	std::string temp = "";
	for(unsigned int i = 0; i < current_directory.size(); i++){
		if(current_directory[i] == '/'){
			if(temp != ""){
				directories.push_back(temp);
				temp = "";
			}
		}
		else{ temp += current_directory[i]; }
	}
	
	if(temp != ""){ directories.push_back(temp); }
	
	current_depth = directories.size();
}

/// Default constructor.
PollOutputFile::PollOutputFile(){ 
	initialize();
}

/// Constructor to set the output filename prefix.
PollOutputFile::PollOutputFile(std::string filename_){
	initialize();
	fname_prefix = filename_;
}

/// Toggle debug mode.
void PollOutputFile::SetDebugMode(bool debug_/*=true*/){
	debug_mode = debug_;
	pldHead.SetDebugMode(debug_);
	pldData.SetDebugMode(debug_);
	dirBuff.SetDebugMode(debug_);
	headBuff.SetDebugMode(debug_);
	dataBuff.SetDebugMode(debug_);
	eofBuff.SetDebugMode(debug_);
}

/// Set the output file data format.
bool PollOutputFile::SetFileFormat(unsigned int format_){
	if(format_ <= 2){
		output_format = format_;
		return true;
	}
	return false;
}

/// Set the output filename prefix.
void PollOutputFile::SetFilenamePrefix(std::string filename_){ 
	fname_prefix = filename_; 
	current_file_num = 0;
}

/// Write nWords_ words of pixie data to disk.
int PollOutputFile::Write(char *data_, unsigned int nWords_){
	if(!data_ || nWords_ == 0){ return -1; }

	if(!output_file.is_open() || !output_file.good()){ return -1; }

	if(nWords_ > max_spill_size){ max_spill_size = nWords_; }
	
	// Write data to disk
	int buffs_written;
	if(output_format == 0){
		if(!dataBuff.Write(&output_file, data_, nWords_, buffs_written)){ return -1; }
	}
	else if(output_format == 1){
		if(!pldData.Write(&output_file, data_, nWords_)){ return -1; }
		buffs_written = 1;
	}
	else{
		if(debug_mode){ std::cout << "debug: invalid output format for PollOutputFile::Write!\n"; }
		return -1;
	}
	number_spills++;
	
	return buffs_written;
}

/** Broadcast a data spill notification message onto the network.
  * Return the total number of bytes in the packet upon success, and -1 otherwise.
  */
int PollOutputFile::SendPacket(Client *cli_){
	if(!cli_){ return -1; }

	unsigned int end_packet = ENDBUFF;
	unsigned int buff_size = ACTUAL_BUFF_SIZE;
	std::streampos file_size = output_file.tellp();

	int bytes = -1; // size of char array in bytes
	
	// Size of basic types on this machine. Probably overly cautious, but it only
	// amounts to sending two extra bytes over the network per packet
	char size_of_int = sizeof(int); // Size of integer on this machine
	char size_of_spos = sizeof(std::streampos); // Size of streampos on this machine

	char *packet = NULL;

	if(!output_file.is_open() || !output_file.good()){
		// Below is the packet packet structure
		// ------------------------------------
		// 1 byte size of integer (may not be the same on a different machine)
		// 1 byte size of streampos (may not be the same on a different machine)
		// 4 byte packet length (inclusive, also includes the end packet flag)
		// 4 byte begin packet flag (0xFFFFFFFF)
		bytes = 2 + 2 * sizeof(int); // Total size of the packet (in bytes)
		packet = new char[bytes];
		
		unsigned int index = 0;
		memcpy(&packet[index], (char *)&size_of_int, 1); index += 1;
		memcpy(&packet[index], (char *)&size_of_spos, 1); index += 1;
		memcpy(&packet[index], (char *)&bytes, sizeof(int)); index += sizeof(int);
		memcpy(&packet[index], (char *)&end_packet, sizeof(int)); index += sizeof(int);
	}
	else{
		// Below is the packet packet structure
		// ------------------------------------
		// 1 byte size of integer (may not be the same on a different machine)
		// 1 byte size of streampos (may not be the same on a different machine)
		// 4 byte packet length (inclusive, also includes the end packet flag)
		// x byte file path (no size limit)
		// 8 byte file size streampos (long long)
		// 4 byte spill number ID (unsigned int)
		// 4 byte buffer size (unsigned int)
		// 4 byte begin packet flag (0xFFFFFFFF)
		// length of the file path.
		bytes = (2 + 4 * sizeof(int)) + sizeof(std::streampos) + current_full_filename.size(); // Total size of the packet (in bytes)
		packet = new char[bytes];
		const char *str = current_full_filename.c_str();
	
		unsigned int index = 0;
		memcpy(&packet[index], (char *)&size_of_int, 1); index += 1;
		memcpy(&packet[index], (char *)&size_of_spos, 1); index += 1;
		memcpy(&packet[index], (char *)&bytes, sizeof(int)); index += sizeof(int);
		memcpy(&packet[index], (char *)str, (unsigned int)current_full_filename.size()); index += current_full_filename.size();
		memcpy(&packet[index], (char *)&file_size, sizeof(std::streampos)); index += sizeof(std::streampos);
		memcpy(&packet[index], (char *)&number_spills, sizeof(int)); index += sizeof(int);
		memcpy(&packet[index], (char *)&buff_size, sizeof(int)); index += sizeof(int);
		memcpy(&packet[index], (char *)&end_packet, sizeof(int));
	}
	cli_->SendMessage(packet, bytes);
	
	delete[] packet;
	
	return bytes;
}

/// Close the current file, if one is open, and open a new file for data output
bool PollOutputFile::OpenNewFile(std::string title_, unsigned int &run_num_, std::string prefix, std::string output_directory/*="./"*/, bool continueRun /*= false*/){
	CloseFile();

	// Restart the spill counter for the new file
	number_spills = 0;

	std::string filename = GetNextFileName(run_num_,prefix,output_directory,continueRun);
	output_file.open(filename.c_str(), std::ios::binary);
	if(!output_file.is_open() || !output_file.good()){
		output_file.close();
		return false;
	}

	current_filename = filename;
	get_full_filename(current_full_filename);		

	if(output_format == 0){	
		dirBuff.SetRunNumber(run_num_);
		dirBuff.Write(&output_file); // Every .ldf file gets a DIR header

		if(title_.size() > 80){
			std::cout << "WARNING: Title length " << title_.size() - 80 << " characters too long for ldf format!\n";
			std::cout << " Setting title to \"" << title_.substr(0, 80) << "\".\n";
		}

		headBuff.SetTitle(title_);
		headBuff.SetDateTime();
		headBuff.SetRunNumber(run_num_);
		headBuff.Write(&output_file); // Every .ldf file gets a HEAD file header
	}
	else if(output_format == 1){
		pldHead.SetTitle(title_);
		pldHead.SetRunNumber(run_num_);
		pldHead.SetStartDateTime();
		
		// Write a blank header for now and overwrite it later
		unsigned int temp = 0;
		for(unsigned int i = 0; i < pldHead.GetBufferLength()/4; i++){
			output_file.write((char*)&temp, 4);
		}
		temp = -1;
		output_file.write((char*)&temp, 4); // Close the buffer
	}
	else{
		if(debug_mode){ std::cout << "debug: invalid output format for PollOutputFile::OpenNewFile!\n"; }
		return false;
	}

	return true;
}

/// Return the filename of the next output file.
std::string PollOutputFile::GetNextFileName(unsigned int &run_num_, std::string prefix, std::string output_directory, bool continueRun /*=false*/) {
	std::stringstream filename;
	filename << output_directory << prefix << "_" << std::setfill('0') << std::setw(3) << run_num_;

	if(output_format == 0){ filename << ".ldf"; }
	else if(output_format == 1){ filename << ".pld"; }
	
	std::ifstream dummy_file(filename.str().c_str());
	unsigned int suffix = 0;
	while (dummy_file.is_open()) {
		dummy_file.close();
		filename.str("");
		
		if(continueRun){ filename << output_directory << prefix << "_" << std::setfill('0') << std::setw(3) << run_num_ << "-" << ++suffix; }
		else{ filename << output_directory << prefix << "_" << std::setfill('0') << std::setw(3) << ++run_num_; }
		
		if(output_format == 0){ filename << ".ldf"; }
		else if(output_format == 1){ filename << ".pld"; }
		
		dummy_file.open(filename.str().c_str());
	}
	dummy_file.close();
	return filename.str();
}

/// Write the footer and close the file.
void PollOutputFile::CloseFile(float total_run_time_/*=0.0*/){
	if(!output_file.is_open() || !output_file.good()){ return; }
	
	if(output_format == 0){
		dataBuff.Close(&output_file); // Pad the final data buffer with 0xFFFFFFFF
	
		eofBuff.Write(&output_file); // First EOF buffer signals end of run
		eofBuff.Write(&output_file); // Second EOF buffer signals physical end of file
	
		overwrite_dir(); // Overwrite the total buffer number word and close the file
	}
	else if(output_format == 1){
		unsigned int temp = ENDFILE; // Write an EOF buffer
		output_file.write((char*)&temp, 4);
		
		temp = ENDBUFF; // Signal the end of the file
		output_file.write((char*)&temp, 4);
		
		// Overwrite the blank pld header at the beginning of the file and close it
		output_file.seekp(0);
		pldHead.SetEndDateTime();
		pldHead.SetRunTime(total_run_time_);
		pldHead.SetMaxSpillSize(max_spill_size);
		pldHead.Write(&output_file);
		output_file.close();
	}
	else if(debug_mode){ std::cout << "debug: invalid output format for PollOutputFile::CloseFile!\n"; }
}
