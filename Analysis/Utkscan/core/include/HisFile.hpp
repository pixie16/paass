/** \file hisFile.hpp
 * \brief A class to output histograms in a format which HHIRF damm is able to read.
 *
 * This is a special hacked version of the .his file handler which has all of
 * the root dependencies removed. This version is not supported directly. In
 * order to get the most recent version of this file, clone the git repo via
 *  git clone https://github.com/cthornsb/his2root.git
 * CRT
 *
 * \author C. R. Thornsberry, S. V. Paulauskas
 * \date Feb. 12th, 2016
 */
#ifndef HISFILE_H
#define HISFILE_H

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <memory>
#include <thread>
#include <deque>
#include <unistd.h>
#include <mutex>
#ifndef USE_HRIBF

/// Create a DAMM 1D histogram
void
hd1d_(int dammId, int nHalfWords, int rawlen, int histlen, int min, int max,
      const char *title, unsigned int length);

/// Create a DAMM 2D histogram
void hd2d_(int dammId, int nHalfWords, int rawXlen, int histXlen, int xMin,
           int xMax, int rawYlen, int histYlen, int yMin, int yMax,
           const char *title,
           unsigned int length);

/// Do banana gating using ban files
bool bantesti_(const int &id, const double &x, const double &y);

/// Increment histogram dammID at x and y
void count1cc_(const int &dammID, const int &x, const int &y);

/// Unknown
void set2cc_(const int &dammID, const int &x, const int &y, const int &z);

#else
#include "Scanor.hpp"
#endif

/// Histogram data storage object
class HisData {
public:
    /// Default constructor
    HisData();

    /// Destructor
    ~HisData();

    /// Return true if the array is initialized and ready to read data
    bool IsInit() { return init; }

    /// Initialize the int or short array to a specified size_
    void Initialize(size_t size_, bool use_int_);

    /// Read histogram data from an input histogram file
    bool Read(std::ifstream *input_);

    /// Return a pointer to the raw data array
    unsigned int *GetData() { return data; }

    /// Return an element of the array
    unsigned int Get(size_t index_);

    /// Set an element of the array at a given index
    unsigned int Set(size_t index_, unsigned int val_);

    /// Set an element of the array at a given index
    unsigned int Set(size_t index_, unsigned short val_);

    /// Delete the data arrays and reset all variables
    void Delete();

    /// Array subscript overload. No array range checking!
    unsigned int &operator[](const size_t &index_);

private:
    unsigned int *data;
    size_t size;
    bool use_int;
    bool init;
};

/// drr entry information
struct drr_entry {
    unsigned int hisID; /// ID of the histogram
    unsigned short hisDim; /// Number of dimensions
    unsigned short halfWords; /// Number of half-words (2 bytes) per channel
    unsigned short params[4]; /// Parameter id numbers, for each dimension (up to 4)
    unsigned short raw[4]; /// Raw length
    unsigned short scaled[4]; /// Scaled length
    unsigned short comp[4]; /// The compression level of the histogram
    unsigned short minc[4]; /// Min channel number
    unsigned short maxc[4]; /// Max channel number
    unsigned int offset; /// Location in his file (in 2-bytes units)
    char xlabel[13]; /// X axis label
    char ylabel[13]; /// Y axis label
    float calcon[4]; /// Calibration for X axis
    char title[41]; /// Title
    bool use_int; /// True if the size of a cell is 4 bytes
    bool good; /// True if word size is either 2 (short) or 4 bytes (int)

    size_t total_bins; /// Total number of bins (number of elements in array)
    size_t total_size; /// Size of histogram (in bytes) (total size of array)

    float dx; /// Bin width for the x-axis
    float dy; /// Bin width for the y-axis

    unsigned int total_counts; /// Total number of attempted histogram fills
    unsigned int good_counts; /// Total number of actual histogram fills

    /// Default constructor
    drr_entry() {}

    /// Constructor for 1d histogram
    drr_entry(unsigned int hisID_, unsigned short halfWords_,
              unsigned short raw_, unsigned short scaled_, unsigned short min_,
              unsigned short max_, const char *title_);

    /// Constructor for 2d histogram
    drr_entry(unsigned int hisID_, unsigned short halfWords_,
              unsigned short Xraw_,
              unsigned short Xscaled_, unsigned short Xmin_,
              unsigned short Xmax_,
              unsigned short Yraw_, unsigned short Yscaled_,
              unsigned short Ymin_,
              unsigned short Ymax_, const char *title_);

    /// Initialize variables not stored in the .drr entry
    void initialize();

    /// Check that a specified global bin is in range
    bool check_bin(size_t bin_) {
        return (bin_ < total_bins);
    }

    /// Check that a specified x bin is in range.
    bool check_x_bin(unsigned int x_) {
        return (x_ <= scaled[0]);
    }

    /// Check that a specified x value is in range.
    bool check_x_range(unsigned int x_) {
        return (x_ >= minc[0] && x_ <= maxc[0]);
    }

    /// Check that a specified y bin is in range. Always returns true for 1d plots.
    bool check_y_bin(unsigned int y_) {
        return (y_ <= scaled[1] || hisDim < 2);
    }

    /// Check that a specified y value is in range. Always returns true for 1d plots.
    bool check_y_range(unsigned int y_) {
        return ((y_ >= minc[1] && y_ <= maxc[1]) || hisDim < 2);
    }

    /// Return the global array bin for a given x and y bin
    bool get_bin(unsigned int x_, unsigned int y_, unsigned int &bin);

    /// Return the local x and y bins for a given global bin
    bool get_bin_xy(unsigned int global_, unsigned int &x, unsigned int &y);

    /// Return the global array bin for a given x, y coordinate
    bool find_bin(unsigned int x_, unsigned int y_, unsigned int &bin);

    /// Return the global array bin for a given x, y coordinate
    bool find_bin_xy(unsigned int x_, unsigned int y_, unsigned int &x,
                     unsigned int &y);

    /// Print a 128 byte .drr file entry block
    void print_drr(std::ofstream *file_);

    /// Print a formatted .list file entry
    void print_list(std::ofstream *file_);

    unsigned long long calculate_buffer_location(unsigned int bin)
    {
        return (offset * 2) + (bin * halfWords * 2);
    }
};


class HisFile {
protected:
    bool is_good; /// True if a valid drr file is open
    bool is_open; /// True if both the drr and his files are open (also requires is_good == true)
    bool debug_mode; /// True if debug mode is set
    std::ifstream drr; /// The input .drr file
    std::ifstream his; /// The input .his file

    int hists_processed; /// The number of histograms which have been processed
    int err_flag; /// Integer value for storing error information

    HisData data; /// The histogram data storage object
    bool his_ready; /// True if a histogram is loaded and ready to read

    // drr header information
    char initial[13]; /// String HHIRFDIR0001
    int nHis; /// Number of histograms in file
    int nHWords; /// Number of half-words (2 bytes) in file
    int date[6]; /// Date 0 YY MM DD HR MN
    char description[41]; /// Field for text description 

    std::shared_ptr<drr_entry> current_entry; /// Pointer to the current working drr entry
    
    // Storage for all the drr_entry objects in use
    // if a drr_entry at id does not exist, drr_entry_map[id] will be nullptr
    // so you'll need to guard when you use an iterator over this
    // using shared_ptr allows automatic delete on calling clear()
    // unique_ptr would be preferred, but since the API guarantees
    // access to the raw ptr through GetDrrEntry, and unique_ptr.get() is 
    // marked delete, shared_ptr is the better option. 
    std::vector<std::shared_ptr<drr_entry>> drr_entry_map = {};

    /// Read an entry from the drr file
    std::shared_ptr<drr_entry> read_entry();

    /// Set the size of the histogram and allocate memory for data storage
    bool set_hist_size();

    /// Delete all drr entries and clear the entries vector
    void clear_drr_entries();

    /// Initialize all variables
    void initialize();

public:
    HisFile();

    HisFile(const char *prefix_);

    ~HisFile();

    /// Toggle debug mode on or off
    void SetDebugMode(bool input_ = true) { debug_mode = input_; }

    /// Get the error code for a member function call
    int GetError(bool verbose_ = true);

    /// Return true if a valid drr file is open
    bool IsGood() { return is_good; }

    /// Return true if both the drr and his files are open (also requires is_good == true)
    bool IsOpen() { return is_open; }

    /// Return a pointer to the current .drr file entry
    drr_entry *GetDrrEntry() { return current_entry.get(); }

    /// Return the date formatted as mmm dd, yyyy HH:MM
    std::string GetDate();

    /// Get the number of bins in the x-axis
    short GetXbins();

    /// Get the number of bins in the y-axis
    short GetYbins();

    /*
    /// Get the number of bins in the z-axis
    short GetZbins();
      
    /// Get the number of bins in the a-axis
    short GetAbins();
    */

    /// Get the range of a 1d histogram
    bool Get1dRange(short &xmin, short &xmax);

    /// Get the range of a 2d histogram
    bool Get2dRange(short &xmin, short &xmax, short &ymin, short &ymax);

    /*
    /// Get the range of a 3d histogram
    bool Get3dRange(short &xmin, short &xmax, short &ymin, short &ymax, 
                    short &zmin, short &zmax);
      
      /// Get the range of a 4d histogram
    bool Get4dRange(short &xmin, short &xmax, short &ymin, short &ymax,
                    short &zmin, short &zmax, short &amin, short &amax);
    */

    /// Get the size of the .his file
    std::streampos GetHisFilesize();

    /// Get the ID of the histogram
    int GetHisID();

    /// Get the dimension of the histogram
    short GetDimension();

    /// Return pointer to the histogram data array
    /*unsigned int *GetIntegerData(){ return idata; }
      
      unsigned short *GetShortData(){ return sdata; }*/

    /// Return a pointer to the histogram data storage object
    HisData *GetData() { return &data; }

    /// Get a pointer to a root TH1I
    //TH1I *GetTH1(int hist_=-1);

    /// Get a pointer to a root TH2I
    //TH2I *GetTH2(int hist_=-1);

    /// Get a drr entry from the vector
    void GetEntry(const size_t &id);

    /// Load the specified histogram
    size_t GetHistogram(unsigned int hist_, bool no_copy_ = false);

    /// Load a specified histogram by ID
    size_t GetHistogramByID(unsigned int hist_id_, bool no_copy_ = false);

    /// Load the next histogram specified in the .drr file
    size_t GetNextHistogram(bool no_copy_ = false);

    /// Load drr entries from the .drr file
    bool LoadDrr(const char *prefix_, bool open_his_ = true);

    void PrintHeader();

    void PrintEntry();
    size_t CountDrrEntries();

    void AddDrrEntry(std::shared_ptr<drr_entry> entry);
};



struct WriteQueueObject
{

    /// @brief Location in the file that needs to be written to
    unsigned long long bufferLocation;
    /// @brief If the drr_entry for the histogram indicates that this histogram is integer valued
    bool isInt;
    /// @brief How much weight to add to the bin
    unsigned int weight;


    WriteQueueObject(unsigned long long bufferLoc, bool isInt, unsigned int weight) :
        bufferLocation(bufferLoc), isInt(isInt), weight(weight){};
};


class HisFileWriter final
{
    private:
        /// @brief How many events have been processed since the last write
        unsigned long long events_since_write;

        /// @brief How many events will be processed between writes, serves as a way to keep from
        /// writing too often
        unsigned long long max_events_between_writes;

        /// @brief Waiting writes to specific locations in the output file
        std::map<unsigned long long, std::pair<unsigned int, bool>> waiting_writes; 

        /// @brief Events that are scheduled to be written but not yet processed
        std::deque<WriteQueueObject> event_queue;

        /// @brief ptr to the output file in the OutputHisFile object
        std::fstream *ofile;

        /// @brief used to help the thread determine if it needs to halt execution
        bool stopCalled;
        
        /// @brief serves as a guard to prevent multiple threads of execution being started
        bool running;
 
        /// @brief The execution thread for the HisFileWriter
        /// static object because I'm a dolt and sleep deprived
        std::thread HisFileWriterEventThread;
    public:
        static std::shared_ptr<HisFileWriter> instance;
        HisFileWriter()
        {
            events_since_write = 0;
            max_events_between_writes = 1000000;
            waiting_writes = {};
            event_queue = {};
            ofile = nullptr;
            stopCalled = false;
            running = false;
        }
        /// @brief Used to allow utkscan and HisFileWriter to share the queues
        std::mutex HisWriterMutex;

        static void SetOfile(std::fstream *ofile_)
        {
            instance->ofile = ofile_;
        }
        
        /// @brief Takes a vector of events and pushes them to the queue on the execution thread
        /// uses the mutex to lock access to the queue while operating
        /// @param events a vector of events
        static void EnqueueWrites(std::vector<WriteQueueObject> &events);

        inline static void SetMaxEventsBetweenWrites(unsigned long long newMax)
        {
            instance->max_events_between_writes = newMax;
        }
        
        /// Stops the event loop, finishing processing all events
        /// before terminating the thread
        static void Stop(){
            if(!instance->running) {return;}
            instance->stopCalled = true;
            instance->HisFileWriterEventThread.join();
            instance->running = false;
        }

        /// @brief Starts the execution thread fo the HisFileWriter which does nothing until
        /// events are passed in using EnqeueWrite()
        static void Start(){
            if(instance->running) {return;}
            instance->running = true;
            instance->HisFileWriterEventThread = std::thread(EventLoop);
        }

        /// @brief Takes events from the event queue and pushes them into waiting writes
        static void ProcessQueue();

        /// @brief Flushes all waiting writes to disk
        static void FlushWrites();

        /// @brief Completely empties the event queue and then flushes all remaining writes to disk
        static void Finalize();

        /// @brief Handles processing the queue and flushing the disk, this is the
        /// alternates between reading from the queue and writing to disk.
        static inline void EventLoop(){
            while(!instance->stopCalled)
            {
                ProcessQueue();
                FlushWrites();
                //usleep(10);
            }
	    while( instance->event_queue.size() > 0 ){
            	ProcessQueue();
            	FlushWrites();
	    }
            Finalize();
        }
};



class OutputHisFile : public HisFile {
private:
    std::fstream ofile; /// The output .his file stream
    std::string fname; /// The output filename prefix
    bool writable; /// True if the output .his file is open and writable
    bool finalized; /// True if the .his and .drr files are locked
    bool existing_file; /// True if the .his file was a previously existing file
    unsigned int Flush_wait; /// Number of fills to wait between Flushes
    unsigned int Flush_count; /// Number of fills since last Flush

    /// @brief a vector of events we're waiting to send to the writer
    std::vector<WriteQueueObject> waiting_to_enqueue = {};
    std::map<unsigned int, unsigned long long> failed_fills; /// Vector containing list of histogram fills into an invalid his id
    std::set<unsigned int> undef_his_failed_fills; /// Vector containing list of histogram fills into an invalid his id
    /// Vector containing list of histogram fills that failed because they did not try to access a valid bin
    std::map<unsigned int, unsigned long long>  bin_not_found_failed_fills;
    std::streampos total_his_size; /// Total size of .his file

    /// Find the specified .drr entry in the drr list using its histogram id
    std::shared_ptr<drr_entry> find_drr_in_list(unsigned int hisID_);

    /// Enqueues a write in the output file with a 
    void EnqueueWrite(std::shared_ptr<drr_entry> entry, unsigned int bin, unsigned int weight);

public:
    OutputHisFile();

    OutputHisFile(std::string fname_prefix);

    ~OutputHisFile();

    /// Return true if the output .his file is open and writable and false otherwise
    bool IsWritable() { return writable; }

    /// Return a pointer to the output file
    std::fstream *GetOutputFile() { return &ofile; }

    /// Set the number of fills to wait between file Flushes
    void SetFlushWait(unsigned int wait_) { 
        Flush_wait = wait_;
        HisFileWriter::SetMaxEventsBetweenWrites(wait_);
    }

    /* Push back with another histogram entry. This command will also
     * extend the length of the .his file (if possible). DO NOT delete
     * the passed drr_entry after calling. OutputHisFile will handle cleanup.
     * On success, returns the number of bytes the file was extended by and zero
     * upon failure.
     */
    size_t push_back(std::shared_ptr<drr_entry> entry_);

    /* Lock the .his and .drr files from being modified. This prevents the user from
     * adding any more histograms to the .drr entry list.
     */
    bool Finalize(bool make_list_file_ = false,
                  const std::string &descrip_ = "RootPixieScan .drr file");

    /// Increment a histogram at (x, y) by weight_
    bool Fill(unsigned int hisID_, unsigned int x_, unsigned int y_,
              unsigned int weight_ = 1);

    /// Increment a histogram at bin (x, y) by weight_
    bool FillBin(unsigned int hisID_, unsigned int x_, unsigned int y_,
                 unsigned int weight_ = 1);

    /// Zero the specified histogram 
    bool Zero(unsigned int hisID_);

    /// Zero all histograms
    bool Zero();

    /// Open a new .his file
    bool Open(std::string fname_prefix);

    /// Flush histogram fills to file
    void Flush();

    /// Close the histogram file and write the drr file
    void Close();
};

extern OutputHisFile *output_his; /// The global .his file handler

#endif
