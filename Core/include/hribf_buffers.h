/** \file hribf_buffers.h
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
  * \date Sept. 29th, 2015
  * 
  * \version 1.2.04
*/

#ifndef HRIBF_BUFFERS_H
#define HRIBF_BUFFERS_H

#include <fstream>
#include <vector>

#define HRIBF_BUFFERS_VERSION "1.2.10"
#define HRIBF_BUFFERS_DATE "May 28th, 2016"

#define ACTUAL_BUFF_SIZE 8194 /// HRIBF .ldf file format

class Client;

class BufferType{
  protected:
	int bufftype;
	int buffsize;
	int buffend;
	int zero;
	bool debug_mode;
	
	BufferType(int bufftype_, int buffsize_, int buffend_=-1);
	
	/// Returns only false if not overloaded
	virtual bool Write(std::ofstream *file_);

	/// Returns only false if not overloaded
	virtual bool Read(std::ifstream *file_);
	
	/// Does nothing if not overloaded.
	virtual void Reset(){  }
	
  public:
	int GetBufferType(){ return bufftype; }
	
	int GetBufferSize(){ return buffsize; }
	
	int GetBufferEndFlag(){ return buffend; }
	
	bool DebugMode(){ return debug_mode; }
  
	void SetDebugMode(bool debug_=true){ debug_mode = debug_; }

	/// Return true if the first word of the current buffer is equal to this buffer type
	bool ReadHeader(std::ifstream *file_);
};

/// The pld header contains information about the run including the date/time, the title, and the run number.
class PLD_header : public BufferType{
  private:
	float run_time; // Total length of run (time acquisition is running in seconds)
	int run_num; // Run number
	int max_spill_size; // Maximum size of spill in file (in words)
	char format[17]; // 'PIXIE LIST DATA ' (16 bytes)
	char facility[17]; // 'U OF TENNESSEE  ' (16 bytes)
	char start_date[25]; // Wed Feb 13 16:06:10 2013 (24 bytes)
	char end_date[25]; // Wed Feb 13 16:06:10 2013 (24 bytes)
	char *run_title; // Unlimited length

  public:
	PLD_header();
	~PLD_header();
	
	int GetBufferLength(); /// Get the total length of the buffer (in bytes)
	
	char *GetFacility(){ return facility; }
	
	char *GetFormat(){ return format; }
		
	char *GetStartDate(){ return start_date; }
	
	char *GetEndDate(){ return end_date; }
	
	char *GetRunTitle(){ return run_title; }
		
	int GetRunNumber(){ return run_num; }
	
	int GetMaxSpillSize(){ return max_spill_size; }
	
	float GetRunTime(){ return run_time; }
		
	void SetStartDateTime();
	
	void SetEndDateTime();

	void SetFacility(std::string input_);
	
	void SetTitle(std::string input_);
	
	void SetRunNumber(int input_){ run_num = input_; }
	
	void SetMaxSpillSize(int max_spill_size_){ max_spill_size = max_spill_size_; }
	
	void SetRunTime(float time_){ run_time = time_; }
	
	/** HEAD buffer (1 word buffer type, 1 word run number, 1 word maximum spill size, 4 word format, 
	  * 2 word facility, 6 word date, 1 word title length (x in bytes), x/4 word title, 1 word end of buffer*/
	virtual bool Write(std::ofstream *file_);

	/// Read a HEAD buffer from a pld format file. Return false if buffer has the wrong header and return true otherwise
	virtual bool Read(std::ifstream *file_);

	/// Set initial values.
	virtual void Reset();
};

/// The DATA buffer contains all physics data within the .pld file
class PLD_data : public BufferType{
  private:
	
  public:
	PLD_data(); /// 0x41544144 "DATA"

	/// Write a data spill to file
	virtual bool Write(std::ofstream *file_, char *data_, int nWords_);
	
	/// Read a data spill from a file
	virtual bool Read(std::ifstream *file_, char *data_, int &nBytes, int max_bytes_, bool dry_run_mode=false);

	/// Set initial values.
	virtual void Reset(){ }
};

/* The DIR buffer is written at the beginning of each .ldf file. When the file is ready
   to be closed, the data within the DIR buffer is re-written with run information. */
class DIR_buffer : public BufferType{
  private:
  	int total_buff_size;
	int run_num;
	int unknown[3];
	
  public:
	DIR_buffer();
	
	int GetTotalBufferSize(){ return total_buff_size; }
	
	int GetRunNumber(){ return run_num; }
	
	void SetRunNumber(int input_){ run_num = input_; }
	
	/* DIR buffer (1 word buffer type, 1 word buffer size, 1 word for total buffer length,
	   1 word for total number of buffers, 2 unknown words, 1 word for run number, 1 unknown word,
	   and 8186 zeros) */
	virtual bool Write(std::ofstream *file_);

	/// Read a DIR buffer from a file. Return false if buffer has the wrong header and return true otherwise
	virtual bool Read(std::ifstream *file_, int &number_buffers);

	/// Set initial values.
	virtual void Reset();
};

/* The HEAD buffer is written after the DIR buffer for each .ldf file. HEAD contains information
   about the run including the date/time, the title, and the run number. */
class HEAD_buffer : public BufferType{
  private:
	char facility[9];
	char format[9];
	char type[17];
	char date[17];
	char run_title[81];
	int run_num;

  public:
	HEAD_buffer();
		
	char *GetFacility(){ return facility; }
	
	char *GetFormat(){ return format; }
		
	char *GetType(){ return type; }
	
	char *GetDate(){ return date; }
	
	char *GetRunTitle(){ return run_title; }
		
	int GetRunNumber(){ return run_num; }
		
	bool SetDateTime();
	
	bool SetTitle(std::string input_);
	
	void SetRunNumber(int input_){ run_num = input_; }

	/** HEAD buffer (1 word buffer type, 1 word buffer size, 2 words for facility, 2 for format, 
	  * 3 for type, 1 word separator, 4 word date, 20 word title [80 character], 1 word run number,
	  * 30 words of padding, and 8129 end of buffer words) */
	virtual bool Write(std::ofstream *file_);

	/// Read a HEAD buffer from a file. Return false if buffer has the wrong header and return true otherwise
	virtual bool Read(std::ifstream *file_);

	/// Set initial values.
	virtual void Reset();
};

/// The DATA buffer contains all physics data within the .ldf file
class DATA_buffer : public BufferType{
  private:
	int current_buff_pos; /// Absolute buffer position
	int buff_words_remaining; /// Absolute number of buffer words remaining
	int good_words_remaining; /// Good buffer words remaining (not counting header or footer words)

	int retval; /// The error code for the read method.

	unsigned int buffer1[ACTUAL_BUFF_SIZE]; /// Container for a ldf buffer.
	unsigned int buffer2[ACTUAL_BUFF_SIZE]; /// Container for a second ldf buffer.

	unsigned int *curr_buffer; /// Pointer to the current ldf buffer.
	unsigned int *next_buffer; /// Pointer to the next ldf buffer.
	
	unsigned int bcount; /// The total number of ldf buffers read from file.
	unsigned int buff_head; /// The ldf buffer header ID.
	unsigned int buff_size; /// Total size of ldf buffer (in 4 byte words).
	unsigned int good_chunks; /// Count of the number of good spill chunks which were read.
	unsigned int missing_chunks; /// Count of the number of missing spill chunks which were dropped.

	size_t buff_pos; /// The actual position in the current ldf buffer.

	/// DATA buffer (1 word buffer type, 1 word buffer size)
	bool open_(std::ofstream *file_);

	bool read_next_buffer(std::ifstream *f_, bool force_=false);
	
  public:
	DATA_buffer(); /// 0x41544144 "DATA"

	/// Close a data buffer by padding with 0xFFFFFFFF
	bool Close(std::ofstream *file_);

	/** Get the standard data spill size for a given data file. This number is set at runtime by poll
	  * and should be the same for each and every spill in the file. Returns the spill size in words 
	  * or -1 in the event of an error. This method should be called whenever a new file is opened. */
	int GetSpillSize(std::ifstream *file_);

	/** Get the return value from the read method.
	  *  0 - Success
	  *  1 - Encountered single EOF buffer (end of run)
	  *  2 - Encountered double EOF buffer (end of file)
	  *  3 - Encountered unknown ldf buffer type
	  *  4 - Encountered invalid spill chunk
	  *  5 - Received bad spill footer size
	  *  6 - Failed to read buffer from file
	  */
	int GetRetval(){ return retval; }
	
	/// Return the number of good spill chunks which were read.
	unsigned int GetNumChunks(){ return good_chunks; }
	
	/// Return the number of missing or dropped spill chunks.
	unsigned int GetNumMissing(){ return missing_chunks; }
	
	/// Write a data spill to file
	virtual bool Write(std::ofstream *file_, char *data_, int nWords_, int &buffs_written);
	
	/// Read a data spill from a file
	virtual bool Read(std::ifstream *file_, char *data_, unsigned int &nBytes_, unsigned int max_bytes_, bool &full_spill, bool &bad_spill, bool dry_run_mode=false);

	/// Set initial values.
	virtual void Reset();
};

/// A single EOF buffer signals the end of a run (pacman .ldf format). A double EOF signals the end of the .ldf file.
class EOF_buffer : public BufferType{	
  public:
	EOF_buffer(); /// 0x20464F45 "EOF "
	
	/// EOF buffer (1 word buffer type, 1 word buffer size, and 8192 end of buffer words)
	virtual bool Write(std::ofstream *file_);

	/// Read an EOF buffer from a file. Return false if buffer has the wrong header and return true otherwise
	virtual bool Read(std::ifstream *file_);
	
	/// Set initial values.
	virtual void Reset(){ }
};

class PollOutputFile{
  private:
	std::ofstream output_file;
	std::string fname_prefix;
	std::string current_filename;
	std::string current_full_filename;
	PLD_header pldHead;
	PLD_data pldData;
	DIR_buffer dirBuff;
	HEAD_buffer headBuff;
	DATA_buffer dataBuff;
	EOF_buffer eofBuff;
	int max_spill_size;
	int current_file_num;
	int output_format;
	int number_spills;
	bool debug_mode;
	int run_num;
	
	int current_depth;
	std::string current_directory;
	std::vector<std::string> directories;

	/// Get the formatted filename of the current file
	std::string get_filename();
	
	/// Get the full path of the current file
	bool get_full_filename(std::string &output);

	/** Overwrite the fourth word of the file with the total number of buffers and close the file
	  * Returns false if no output file is open or if the number of 4 byte words in the file is not 
	  * evenly divisible by the number of words in a buffer */
	bool overwrite_dir(int total_buffers_=-1);

	/// Initialize the output file with initial parameters
	void initialize();

  public:
	PollOutputFile();

	PollOutputFile(std::string filename_);
	
	~PollOutputFile(){ CloseFile(); }
	
	/// Get the size of the current file, in bytes.
	std::streampos GetFilesize(){ return output_file.tellp(); }
	
	/// Get the name of the current output file
	std::string GetCurrentFilename(){ return current_filename; }
	
	/// Return the total number of spills written since the current file was opened
	int GetNumberSpills(){ return number_spills; }

	/// Return a pointer to the PLD header object
	PLD_header *GetPLDheader(){ return &pldHead; }

	/// Return a pointer to the PLD data object
	PLD_data *GetPLDdata(){ return &pldData; }
	
	/// Return a pointer to the DIR buffer object
	DIR_buffer *GetDIRbuffer(){ return &dirBuff; }
	
	/// Return a pointer to the HEAD buffer object
	HEAD_buffer *GetHEADbuffer(){ return &headBuff; }

	/// Return a pointer to the DATA buffer object
	DATA_buffer *GetDATAbuffer(){ return &dataBuff; }
	
	/// Return a pointer to the EOF buffer object
	EOF_buffer *GetEOFbuffer(){ return &eofBuff; }
	
	/// Toggle debug mode
	void SetDebugMode(bool debug_=true);
	
	/// Set the output file format
	bool SetFileFormat(int format_);

	/// Set the output filename prefix
	void SetFilenamePrefix(std::string filename_);

	/// Return true if an output file is open and writable and false otherwise
	bool IsOpen(){ return (output_file.is_open() && output_file.good()); }
	
	/// Write nWords_ of data to the file
	int Write(char *data_, int nWords_);

	/** Build a data spill notification message for broadcast onto the network
	  * Return the total number of bytes in the packet upon success, and -1 otherwise */
	int SendPacket(Client *cli_);

	/// Close the current file, if one is open, and open a new file for data output
	bool OpenNewFile(std::string title_, int &run_num_, std::string prefix, std::string output_dir="./", bool continueRun = false);

	std::string GetNextFileName(int &run_num_, std::string prefix, std::string output_dir, bool continueRun = false);
	
	int GetRunNumber() {return dirBuff.GetRunNumber();}

	/// Write the footer and close the file
	void CloseFile(float total_run_time_=0.0);
};

#endif
