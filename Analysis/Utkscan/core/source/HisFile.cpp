/** \file hisFile.cpp
 * \brief A class to output histograms in a format which HHIRF damm is able to read.
 *
 * This is a special hacked version of the .his file handler which has all of
 * the root dependencies removed. This version is not supported directly. In
 * order to get the most recent version of this file, clone the git repo via
 *  git clone https://github.com/cthornsb/his2root.git
 * CRT
 *
 * \author C. R. Thornsberry
 * \date Feb. 12th, 2016
 */
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>

#include <string.h>
#include <time.h>
#include <math.h>

#include "HisFile.hpp"

#ifndef USE_HRIBF


///////////////////////////////////////////////////////////////////////////////
// class HisFileWriter
///////////////////////////////////////////////////////////////////////////////
// do not redefine
std::shared_ptr<HisFileWriter> HisFileWriter::instance = nullptr;

void HisFileWriter::EnqueueWrites(std::vector<WriteQueueObject> &events)
{
    /// called from the main utkscan thread
    instance->HisWriterMutex.lock();
    for(auto &evt: events){
        instance->event_queue.push_back(evt);
    }
    instance->HisWriterMutex.unlock();
}

void HisFileWriter::ProcessQueue()
{
    // prefer to let the main thread have control here
    if(!instance->HisWriterMutex.try_lock()){return;}
    // empty the whole queue every time we get the lock
    while(instance->event_queue.size() > 0)
    {
        WriteQueueObject evt = instance->event_queue.front();
        instance->event_queue.pop_front();
        if(instance->waiting_writes.find(evt.bufferLocation) == instance->waiting_writes.end())
        {
           instance->waiting_writes.insert({evt.bufferLocation, {evt.weight, evt.isInt}});
        }
        else
        {
            instance->waiting_writes.at(evt.bufferLocation).first += evt.weight;
        }
        instance->events_since_write++;
    }
    instance->HisWriterMutex.unlock();
}

void HisFileWriter::FlushWrites()
{
    if(instance->waiting_writes.size() == 0) {return;}
    for (auto &fill: instance->waiting_writes) {
        // the location in the file that we're filling with the value the map 
        // being sorted by smallest location allows piling what would've 
        // been multiple writes to the same location to a single write
        unsigned long long location = fill.first;
        unsigned int weight         = fill.second.first;
        bool isInt                  = fill.second.second;

        // Seek to the specified bin
        instance->ofile->seekg(location,
                    std::ios::beg); // input offset

        unsigned short sval = 0;
        unsigned int ival = 0;

        // Overwrite the bin value
        if (isInt) {
            // Get the original value of the bin
            instance->ofile->read((char *) &ival, 4);
            ival += weight;

            // Set the new value of the bin
            instance->ofile->seekp(location, std::ios::beg); 
            instance->ofile->write((char *) &ival, 4);
        } else {
            // Get the original value of the bin
            instance->ofile->read((char *) &sval, 2);
            sval += (short) weight;

            // Set the new value of the bin
            instance->ofile->seekp(location, std::ios::beg);
            instance->ofile->write((char *) &sval, 2);
        }
    }
    instance->events_since_write = 0;
    instance->waiting_writes.clear();
}

void HisFileWriter::Finalize()
{
    // block till we acquire the lock because we're going to lose data otherwise
    instance->HisWriterMutex.lock();
    while(instance->event_queue.size() > 0)
    {
        WriteQueueObject evt = instance->event_queue.front();
        instance->event_queue.pop_front();
        if(instance->waiting_writes.find(evt.bufferLocation) == instance->waiting_writes.end())
        {
           instance->waiting_writes.insert({evt.bufferLocation, {evt.weight, evt.isInt}});
        } else {
            instance->waiting_writes.at(evt.bufferLocation).first += evt.weight;
        }
        instance->events_since_write++;
    }
    instance->HisWriterMutex.unlock();
    FlushWrites();
}

/// Create a DAMM 1D histogram (implemented for backwards compatibility)
void
hd1d_(int dammId, int nHalfWords, int rawlen, int histlen, int min, int max,
      const char *title, unsigned int length) {
    if (!output_his)
        return;
    output_his->push_back(std::make_shared<drr_entry>(dammId, (short) nHalfWords, (short) rawlen,
                                     (short) histlen, (short) min, (short) max,
                                     title));
}

/// Create a DAMM 2D histogram (implemented for backwards compatibility)
void hd2d_(int dammId, int nHalfWords, int rawXlen, int histXlen, int xMin,
           int xMax, int rawYlen, int histYlen, int yMin, int yMax,
           const char *title,
           unsigned int length) {
    if (!output_his)
        return;
    output_his->push_back(
        std::make_shared<drr_entry>(dammId, (short) nHalfWords,
                                     (short) rawXlen, (short) histXlen,
                                     (short) xMin,
                                     (short) xMax, (short) rawYlen,
                                     (short) histYlen,
                                     (short) yMin, (short) yMax, title));
}

/// Do banana gating using ban files (implemented for backwards compatibility)
bool bantesti_(const int &id, const double &x, const double &y) {
    return (false);
}

/// Increment histogram dammID at x and y (implemented for backwards compatibility)
void count1cc_(const int &dammID, const int &x, const int &y) {
    if (!output_his)
        return;
    output_his->Fill(dammID, x, y);
}

/// Unknown (implemented for backwards compatibility)
void set2cc_(const int &dammID, const int &x, const int &y, const int &z) {
    if (!output_his)
        return;
    output_his->Fill(dammID, x, y, z);
}

#endif

/// Strip trailing whitespace from a c-string
std::string rstrip(char *input_) {
    if (strlen(input_) == 0) { return std::string(""); }

    unsigned int index = (int) strlen(input_) - 1;
    std::string output = "";

    // Skip all trailing whitespace
    while (input_[index--] == ' ' && index > 0) {}

    if (index == 0)
        return (output);
    else
        index++;

    for (unsigned int i = 0; i <= index; i++)
        output += input_[i];

    return (output);
}

/// Copy a string into a character array
void set_char_array(char *output, const std::string &input_, size_t arr_size_) {
    for (size_t index = 0; index < arr_size_; index++) {
        if (index < input_.size())
            output[index] = input_[index];
        else
            output[index] = ' ';
    }
    output[arr_size_] = '\0';
}

///////////////////////////////////////////////////////////////////////////////
// struct HisData
///////////////////////////////////////////////////////////////////////////////

HisData::HisData() {
    use_int = false;
    init = false;
    data = NULL;
    size = 0;
}

HisData::~HisData() {
    this->Delete();
}

void HisData::Initialize(size_t size_, bool use_int_) {
    if (init) { this->Delete(); }
    use_int = use_int_;
    size = size_;
    data = new unsigned int[size_];
    init = true;
}

bool HisData::Read(std::ifstream *input_) {
    if (!init || !input_ || !input_->good()) { return false; }
    unsigned int temp1;
    unsigned short temp2;
    for (size_t i = 0; i < size; i++) {
        if (use_int) {
            input_->read((char *) &temp1, 4);
            data[i] = temp1;
        } else {
            input_->read((char *) &temp2, 2);
            data[i] = (unsigned int) temp2;
        }

    }
    return (true);
}

unsigned int HisData::Get(size_t index_) {
    if (!init || index_ >= size) { return 0; }
    return data[index_];
}

unsigned int HisData::Set(size_t index_, unsigned int val_) {
    if (!init || index_ >= size) { return 0; }
    return (data[index_] = val_);
}

unsigned int HisData::Set(size_t index_, unsigned short val_) {
    if (!init || index_ >= size) { return 0; }
    return (data[index_] = (unsigned int) val_);
}

void HisData::Delete() {
    if (data) { delete[] data; }

    use_int = false;
    init = false;
    data = NULL;
    size = 0;
}

unsigned int &HisData::operator[](const size_t &index_) {
    return data[index_];
}

///////////////////////////////////////////////////////////////////////////////
// struct drr_entry
///////////////////////////////////////////////////////////////////////////////

/// Constructor for 1d histogram
drr_entry::drr_entry(unsigned int hisID_, unsigned short halfWords_,
                     unsigned short raw_, unsigned short scaled_,
                     unsigned short min_, unsigned short max_,
                     const char *title_) {
    hisID = hisID_;
    hisDim = 1;
    halfWords = halfWords_;

    // Set range and scaling variables
    params[0] = 0;
    params[1] = 0;
    params[2] = 0;
    params[3] = 0;
    raw[0] = raw_;
    raw[1] = 0;
    raw[2] = 0;
    raw[3] = 0;
    scaled[0] = scaled_;
    scaled[1] = 0;
    scaled[2] = 0;
    scaled[3] = 0;
    comp[0] = raw_ / scaled_;
    comp[1] = 1;
    comp[2] = 1;
    comp[3] = 1;
    minc[0] = min_;
    minc[1] = 0;
    minc[2] = 0;
    minc[3] = 0;
    maxc[0] = max_;
    maxc[1] = 0;
    maxc[2] = 0;
    maxc[3] = 0;
    calcon[0] = 0;
    calcon[1] = 0;
    calcon[2] = 0;
    calcon[3] = 0;

    dx = ((float) (maxc[0] - minc[0])) / (scaled[0] - 1.0);
    dy = 0.0;

    // Set label and titles
    set_char_array(xlabel, "            ", 12);
    set_char_array(ylabel, "            ", 12);
    set_char_array(title, std::string(title_), 40);

    total_bins = scaled[0];
    total_size = total_bins * 2 * halfWords;
    total_counts = 0;
    good_counts = 0;

    good = true;
    offset = 0; // The file offset will be set later
    if (2 * halfWords == 4) { use_int = true; }
    else if (2 * halfWords == 2) { use_int = false; }
    else {
        std::cout << "Invalid cell size (" << 2 * halfWords << ")!\n";
        good = false;
    }
}

/// Constructor for 2d histogram
drr_entry::drr_entry(unsigned int hisID_, unsigned short halfWords_,
                     unsigned short Xraw_, unsigned short Xscaled_,
                     unsigned short Xmin_, unsigned short Xmax_,
                     unsigned short Yraw_, unsigned short Yscaled_,
                     unsigned short Ymin_, unsigned short Ymax_,
                     const char *title_) {
    hisID = hisID_;
    hisDim = 2;
    halfWords = halfWords_;

    // Set range and scaling variables
    params[0] = 0;
    params[1] = 0;
    params[2] = 0;
    params[3] = 0;
    raw[0] = Xraw_;
    raw[1] = Yraw_;
    raw[2] = 0;
    raw[3] = 0;
    scaled[0] = Xscaled_;
    scaled[1] = Yscaled_;
    scaled[2] = 0;
    scaled[3] = 0;
    comp[0] = Xraw_ / Xscaled_;
    comp[1] = Yraw_ / Yscaled_;
    comp[2] = 1;
    comp[3] = 1;
    minc[0] = Xmin_;
    minc[1] = Ymin_;
    minc[2] = 0;
    minc[3] = 0;
    maxc[0] = Xmax_;
    maxc[1] = Ymax_;
    maxc[2] = 0;
    maxc[3] = 0;
    calcon[0] = 0;
    calcon[1] = 0;
    calcon[2] = 0;
    calcon[3] = 0;

    dx = ((float) (maxc[0] - minc[0])) / (scaled[0] - 1.0);
    dy = ((float) (maxc[1] - minc[1])) / (scaled[1] - 1.0);

    total_bins = scaled[0] * scaled[1];
    total_size = total_bins * 2 * halfWords;

    // Set label and titles
    set_char_array(xlabel, "            ", 12);
    set_char_array(ylabel, "            ", 12);
    set_char_array(title, std::string(title_), 40);
    total_counts = 0;
    good_counts = 0;

    good = true;
    offset = 0; // The file offset will be set later
    if (2 * halfWords == 4) { use_int = true; }
    else if (2 * halfWords == 2) { use_int = false; }
    else {
        std::cout << "Invalid cell size (" << 2 * halfWords << ")!\n";
        good = false;
    }
}

void drr_entry::initialize() {
    if (hisDim == 1) {
        dx = ((float) (maxc[0] - minc[0])) / (scaled[0] - 1.0);
        dy = 0.0;

        total_bins = scaled[0];
        total_size = total_bins * 2 * halfWords;
    } else if (hisDim == 2) {
        dx = ((float) (maxc[0] - minc[0])) / (scaled[0] - 1.0);
        dy = ((float) (maxc[1] - minc[1])) / (scaled[1] - 1.0);

        total_bins = scaled[0] * scaled[1];
        total_size = total_bins * 2 * halfWords;
    } else { return; }

    good = true;
    if (2 * halfWords == 4) { use_int = true; }
    else if (2 * halfWords == 2) { use_int = false; }
    else {
        std::cout << "Invalid cell size (" << 2 * halfWords << ")!\n";
        good = false;
    }
}

bool drr_entry::get_bin(unsigned int x_, unsigned int y_, unsigned int &bin) {
    if (!check_x_bin(x_) || !check_y_bin(y_)) { return false; } // Range check
    bin = y_ * scaled[0] + x_;
    return true;
}

bool
drr_entry::get_bin_xy(unsigned int bin_, unsigned int &x, unsigned int &y) {
    if (!check_x_bin(x) || !check_y_bin(y)) { return false; } // Range check
    x = bin_ % scaled[0];
    y = bin_ / scaled[0];
    return true;
}

bool drr_entry::find_bin(unsigned int x_, unsigned int y_, unsigned int &bin) {
    if (!check_x_range(x_) ||
        !check_y_range(y_)) { return false; } // Range check
    bin = ((unsigned int) roundf(y_ / dy) * scaled[0] +
           (unsigned int) roundf(x_ / dx));
    return true;
}

bool drr_entry::find_bin_xy(unsigned int x_, unsigned int y_, unsigned int &x,
                            unsigned int &y) {
    if (!check_x_range(x_) ||
        !check_y_range(y_)) { return false; } // Range check
    x = (unsigned int) roundf(x_ / dx);
    y = (unsigned int) roundf(y_ / dy);
    return true;
}

void drr_entry::print_drr(std::ofstream *file_) {
    file_->write((char *) &hisDim, 2);
    file_->write((char *) &halfWords, 2);
    file_->write((char *) &params, 8);
    file_->write((char *) &raw, 8);
    file_->write((char *) &scaled, 8);
    file_->write((char *) &minc, 8);
    file_->write((char *) &maxc, 8);
    file_->write((char *) &offset, 4);
    file_->write(xlabel, 12);
    file_->write(ylabel, 12);
    file_->write((char *) &calcon, 16);
    file_->write(title, 40);
}

void drr_entry::print_list(std::ofstream *file_) {
    *file_ << std::setw(5) << hisID << std::setw(5) << hisDim << std::setw(4)
           << halfWords << std::setw(9) << scaled[0];
    *file_ << std::setw(8) << comp[0] << std::setw(6) << minc[0] << std::setw(6)
           << maxc[0] << std::setw(9) << offset;
    *file_ << "  " << rstrip(title) << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
// class HisFile
///////////////////////////////////////////////////////////////////////////////

/// Read an entry from the drr file
std::shared_ptr<drr_entry> HisFile::read_entry() {
    std::shared_ptr<drr_entry> output = std::make_shared<drr_entry>();

    // Read 128 bytes from the drr file
    drr.read((char *) &output->hisDim, 2);
    drr.read((char *) &output->halfWords, 2);
    drr.read((char *) &output->params, 8);
    drr.read((char *) &output->raw, 8);
    drr.read((char *) &output->scaled, 8);
    drr.read((char *) &output->minc, 8);
    drr.read((char *) &output->maxc, 8);
    drr.read((char *) &output->offset, 4);
    drr.read(output->xlabel, 12);
    output->xlabel[12] = '\0';
    drr.read(output->ylabel, 12);
    output->ylabel[12] = '\0';
    drr.read((char *) &output->calcon, 16);
    drr.read(output->title, 40);
    output->title[40] = '\0';

    // Update variables not stored in the .drr file
    output->initialize();

    return output;
}

/// Delete all drr drr_entries and clear the drr_entries vector
void HisFile::clear_drr_entries() {
    drr_entry_map.clear();
    current_entry = NULL;
}

void HisFile::initialize() {
    current_entry = NULL;
    err_flag = 0;
    hists_processed = 0;
    is_good = false;
    is_open = false;
    debug_mode = false;
}

HisFile::HisFile() {
    #ifndef NO_HIS_THREAD
    HisFileWriter::instance = std::make_shared<HisFileWriter>();
    #endif
    initialize();
}

HisFile::HisFile(const char *prefix_) {
    #ifndef NO_HIS_THREAD
    HisFileWriter::instance = std::make_shared<HisFileWriter>();
    #endif
    initialize();
    LoadDrr(prefix_);
}

HisFile::~HisFile() {
    drr.close();
    his.close();

    clear_drr_entries();
}

/// Get the error code for a member function call
int HisFile::GetError(bool verbose_/*=true*/) {
    if (verbose_) {
        std::cout << " HIS_FILE ERROR!\n";
        if (err_flag == 0) { std::cout << "  0: No error occurred.\n"; }
        else if (err_flag == 1) {
            std::cout
                    << "  1: Failed to open the .drr file. Check that the path is correct.\n";
        } else if (err_flag == 2) {
            std::cout
                    << "  2: The .drr file had an incorrect format and could not be read.\n";
        } else if (err_flag == 3) {
            std::cout
                    << "  3: Failed to open the .his file. Check that the path is correct.\n";
        } else if (err_flag == 4) {
            std::cout
                    << "  4: Either the .drr file and/or the .his file are not opened or are not of the correct format.\n";
        } else if (err_flag == 5) {
            std::cout
                    << "  5: Cannot call GetNextHistogram because the last entry in the .drr file is already loaded.\n";
        } else if (err_flag == -1) {
            std::cout
                    << "  -1: current_entry is uninitialized. Use GetHistogram, GetNextHistogram, or GetHistogramByID.\n";
        } else if (err_flag == -2) {
            std::cout
                    << "  -2: Specified .his cell size is larger than that of an integer (4 bytes).\n";
        } else if (err_flag == -3) {
            std::cout
                    << "  -3: GetHistogram returned 0. i.e. the specified histogram does not exist.\n";
        } else if (err_flag == -4) {
            std::cout
                    << "  -4: The current histogram has the incorrect dimension for the called function.\n";
        } else if (err_flag == -5) {
            std::cout
                    << "  -5: .his cell size is 2 bytes but size of histogram data array is not evenly divisible by 2.\n";
        } else if (err_flag == -6) {
            std::cout
                    << "  -6: .his cell size is 4 bytes but size of histogram data array is not evenly divisible by 4.\n";
        } else if (err_flag == -7) {
            std::cout << "  -7: Encountered an invalid .his cell size.\n";
        } else {
            std::cout << "  " << err_flag << ": An unknown error occurred!\n";
        }
    }
    return err_flag;
}

size_t HisFile::CountDrrEntries()
{
    size_t ret = 0;
    for(auto &i: drr_entry_map){
        ret += i != nullptr;
    }
    return ret;
}

void HisFile::AddDrrEntry(std::shared_ptr<drr_entry> entry)
{
    if(entry == nullptr){return;}
    while(drr_entry_map.size() < entry->hisID + 1)
    {
        drr_entry_map.push_back(nullptr);
    }
    if(drr_entry_map[entry->hisID] != nullptr)
    {
	#ifndef NDEBUG
        if(debug_mode)
        {
            std::cout << "tried to add a drr_entry that already exists!" << std::endl;
        }
	#endif
        return;
    }
    drr_entry_map[entry->hisID] = entry;
}

/// Return the date formatted as mmm dd, yyyy HH:MM
std::string HisFile::GetDate() {
    err_flag = 0; // Reset the error flag
    if (!is_open) {
        err_flag = 4;
        return "";
    }

    static std::string months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                     "Jul", "Aug", "Sep", "Nov", "Dec"};

    std::stringstream stream;
    if (date[2] > 0 && date[2] < 12) { stream << months[date[2]]; }
    else { stream << date[2]; }
    if (date[3] < 10) { stream << " 0" << date[3]; }
    else { stream << " " << date[3]; }
    stream << ", " << date[1] << " ";
    if (date[4] < 10) { stream << "0" << date[4] << ":"; }
    else { stream << date[4] << ":"; }
    if (date[5] < 10) { stream << "0" << date[5]; }
    else { stream << date[5]; }

    return stream.str();
}

/// Get the number of bins in the x-axis
short HisFile::GetXbins() {
    err_flag = 0; // Reset the error flag
    if (!current_entry) { return err_flag = -1; }
    return current_entry->scaled[0];
}

/// Get the number of bins in the y-axis
short HisFile::GetYbins() {
    err_flag = 0; // Reset the error flag
    if (!current_entry) { return err_flag = -1; }
    return current_entry->scaled[1];
}

/*/// Get the number of bins in the z-axis
  short HisFile::GetZbins(){ 
  if(!current_entry){ return -1; }
  return current_entry->scaled[2]; 
  }
  
  /// Get the number of bins in the a-axis
  short HisFile::GetAbins(){ 
  if(!current_entry){ return -1; }
  return current_entry->scaled[3]; 
  }*/

/// Get the range of a 1d histogram
bool HisFile::Get1dRange(short &xmin, short &xmax) {
    err_flag = 0; // Reset the error flag
    if (!current_entry) {
        err_flag = -1;
        return false;
    }
    xmin = current_entry->minc[0];
    xmax = current_entry->maxc[0];
    return true;
}

/// Get the range of a 2d histogram
bool HisFile::Get2dRange(short &xmin, short &xmax, short &ymin, short &ymax) {
    err_flag = 0; // Reset the error flag
    if (!current_entry) {
        err_flag = -1;
        return false;
    }
    xmin = current_entry->minc[0];
    xmax = current_entry->maxc[0];
    ymin = current_entry->minc[1];
    ymax = current_entry->maxc[1];
    return true;
}

/*/// Get the range of a 3d histogram
  bool HisFile::Get3dRange(short &xmin, short &xmax, short &ymin, short &ymax, short &zmin, short &zmax){
  if(!current_entry){ return false; }
  xmin = current_entry->minc[0];
  xmax = current_entry->maxc[0];
  ymin = current_entry->minc[1];
  ymax = current_entry->maxc[1];
  zmin = current_entry->minc[2];
  zmax = current_entry->maxc[2];
  }
  
  /// Get the range of a 4d histogram
  bool HisFile::Get4dRange(short &xmin, short &xmax, short &ymin, short &ymax, short &zmin, short &zmax, short &amin, short &amax){
  if(!current_entry){ return false; }
  xmin = current_entry->minc[0];
  xmax = current_entry->maxc[0];
  ymin = current_entry->minc[1];
  ymax = current_entry->maxc[1];
  zmin = current_entry->minc[2];
  zmax = current_entry->maxc[2];
  amin = current_entry->minc[3];
  amax = current_entry->maxc[3];
  }*/

/// Get the size of the .his file
std::streampos HisFile::GetHisFilesize() {
    err_flag = 0; // Reset the error flag
    if (!is_open) {
        err_flag = 4;
        return 0;
    }
    std::streampos initial_pos = his.tellg();
    his.seekg(0, std::ios::end);
    std::streampos output = his.tellg();
    his.seekg(initial_pos, std::ios::beg);
    return output;
}

/// Get the ID of the histogram
int HisFile::GetHisID() {
    err_flag = 0; // Reset the error flag
    if (!current_entry) { return err_flag = -1; }
    return current_entry->hisID;
}

/// Get the dimension of the histogram
short HisFile::GetDimension() {
    err_flag = 0; // Reset the error flag
    if (!current_entry) { return err_flag = -1; }
    return current_entry->hisDim;
}

/// Get a drr entry from the vector
void HisFile::GetEntry(const size_t &id) {
    if(id > drr_entry_map.size() + 1)
    {
        current_entry = nullptr;
    }
    else
    {
        current_entry = drr_entry_map[id];
    }
}

/// Load the specified histogram
size_t HisFile::GetHistogram(unsigned int hist_, bool no_copy_/*=false*/) {
    err_flag = 0; // Reset the error flag
    if (!is_open) {
        err_flag = 4;
        return 0;
    }

    // Get the requested drr entry
    GetEntry(hist_);
    if (!current_entry) {
        err_flag = -1;
        return 0;
    }

    if (!no_copy_) {
        // Seek to the start of this histogram
        his.seekg(current_entry->offset * 2, std::ios::beg);

        // Read the histogram data
        data.Initialize(current_entry->total_bins, current_entry->use_int);
        data.Read(&his);
    }

    return current_entry->total_size;
}

/// Load a specified histogram by ID
size_t
HisFile::GetHistogramByID(unsigned int hist_id, bool no_copy_/*=false*/) {
    err_flag = 0; // Reset the error flag
    if (!is_open) {
        err_flag = 4;
        return 0;
    }

    int his_count = 0;
    for (auto& iter: drr_entry_map) {
        if(iter == nullptr) {
            continue;
        }
        if (iter->hisID == hist_id)
            return (GetHistogram(his_count, no_copy_));
        his_count++;
    }
    return 0;
}

size_t HisFile::GetNextHistogram(bool no_copy_/*=false*/) {
    err_flag = 0; // Reset the error flag
    if (!is_open) {
        err_flag = 4;
        return 0;
    } else if (hists_processed >= nHis) {
        err_flag = 5;
        return 0;
    }

    return GetHistogram(hists_processed++, no_copy_);
}

bool HisFile::LoadDrr(const char *prefix_, bool open_his_/*=true*/) {
    err_flag = 0; // Reset the error flag
    if (drr.is_open())
        drr.close();

    // Clear the old drr drr_entries
    clear_drr_entries();

    hists_processed = 0;

    std::string filename_prefix(prefix_);

    // Open the drr file
    drr.open((filename_prefix + ".drr").c_str(), std::ios::binary);

    // Check that this is a drr file
    is_open = drr.is_open() && drr.good();
    if (is_open) {
        drr.read(initial, 12);
        initial[12] = '\0';

        if (strcmp(initial, "HHIRFDIR0001") == 0) { is_good = true; }
        else {
            err_flag = 2;
            is_good = false;
            return false;
        }
    } else {
        err_flag = 1;
        return false;
    }

    // Open the his file
    if (open_his_) {
        if (his.is_open()) { his.close(); }

        his.open((filename_prefix + ".his").c_str(), std::ios::binary);

        is_open = his.is_open() && his.good();
        if (!is_open) {
            err_flag = 3;
            return false;
        }
    }

    // Read in the drr header
    drr.read((char *) &nHis, 4);
    drr.read((char *) &nHWords, 4);
    for (int i = 0; i < 6; i++) { drr.read((char *) &date[i], 4); }
    drr.seekg(44, std::ios::cur); // skip the trailing garabage
    drr.read(description, 40);
    description[40] = '\0';

    // Read in all drr drr_entries
    for (int i = 0; i < nHis; i++) {
        AddDrrEntry(read_entry());
    }

    return true;
}

void HisFile::PrintHeader() {
    err_flag = 0; // Reset the error flag
    if (!is_open) {
        err_flag = 4;
        return;
    }

    std::cout << "head: " << initial << std::endl;
    std::cout << "nHisto: " << nHis << std::endl;
    std::cout << "nHWords: " << nHWords << std::endl;
    std::cout << "label: " << description << std::endl;
    std::cout << "date: " << GetDate() << std::endl;
}

void HisFile::PrintEntry() {
    err_flag = 0; // Reset the error flag
    if (!current_entry) {
        err_flag = -1;
        return;
    }

    std::cout << "hisID: " << current_entry->hisID << std::endl;
    std::cout << "dimension: " << current_entry->hisDim << std::endl;
    std::cout << "num hwords: " << current_entry->halfWords << std::endl;
    std::cout << "title: " << rstrip(current_entry->title) << std::endl;
    std::cout << "xlabel: " << current_entry->xlabel << std::endl;
    std::cout << "ylabel: " << current_entry->ylabel << std::endl;
    std::cout << "offset: " << current_entry->offset << std::endl;
    std::cout << "params: " << current_entry->params[0] << ", "
              << current_entry->params[1] << ", " << current_entry->params[2]
              << ", " << current_entry->params[3] << std::endl;
    std::cout << "raw: " << current_entry->raw[0] << ", "
              << current_entry->raw[1] << ", " << current_entry->raw[2] << ", "
              << current_entry->raw[3] << std::endl;
    std::cout << "scaled: " << current_entry->scaled[0] << ", "
              << current_entry->scaled[1] << ", " << current_entry->scaled[2]
              << ", " << current_entry->scaled[3] << std::endl;
    std::cout << "comp: " << current_entry->comp[0] << ", "
              << current_entry->comp[1] << ", " << current_entry->comp[2]
              << ", " << current_entry->comp[3] << std::endl;
    std::cout << "minc: " << current_entry->minc[0] << ", "
              << current_entry->minc[1] << ", " << current_entry->minc[2]
              << ", " << current_entry->minc[3] << std::endl;
    std::cout << "maxc: " << current_entry->maxc[0] << ", "
              << current_entry->maxc[1] << ", " << current_entry->maxc[2]
              << ", " << current_entry->maxc[3] << std::endl;
    std::cout << "cal: " << current_entry->calcon[0] << ", "
              << current_entry->calcon[1] << ", " << current_entry->calcon[2]
              << ", " << current_entry->calcon[3] << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
// class OutputHisFile
///////////////////////////////////////////////////////////////////////////////

std::shared_ptr<drr_entry> OutputHisFile::find_drr_in_list(unsigned int hisId) {
    if(hisId < drr_entry_map.size() + 1 && drr_entry_map[hisId])
    {
        return drr_entry_map[hisId];
    }
    return nullptr;
}

void OutputHisFile::Flush() {
    #ifndef NDEBUG
    if (debug_mode){
        std::cout << "debug: Flushing histogram entries to file.\n";
    }

    if (debug_mode && !writable) {
        std::cout << "debug: Output file is not writable!\n";
    }
    #endif
    #ifdef NO_HIS_THREAD
        if(writable){
            for (auto &fill: waiting_writes) {
                // the location in the file that we're filling with the value the map 
                // being sorted by smallest location allows piling what would've 
                // been multiple writes to the same location to a single write
                unsigned long long location = fill.first;
                unsigned int weight         = fill.second.first;
                bool isInt                  = fill.second.second;

                // Seek to the specified bin
                ofile.seekg(location,
                            std::ios::beg); // input offset

                unsigned short sval = 0;
                unsigned int ival = 0;

                // Overwrite the bin value
                if (isInt) {
                    // Get the original value of the bin
                    ofile.read((char *) &ival, 4);
                    ival += weight;

                    // Set the new value of the bin
                    ofile.seekp(location, std::ios::beg); 
                    ofile.write((char *) &ival, 4);
                } else {
                    // Get the original value of the bin
                    ofile.read((char *) &sval, 2);
                    sval += (short) weight;

                    // Set the new value of the bin
                    ofile.seekp(location, std::ios::beg);
                    ofile.write((char *) &sval, 2);
                }
            }
            waiting_writes.clear();
        }
    #else
        if (writable) { 
            HisFileWriter::EnqueueWrites(waiting_to_enqueue);
        } 
    #endif
    waiting_to_enqueue.clear();
    Flush_count = 0;
}

OutputHisFile::OutputHisFile() {
    fname = "";
    writable = false;
    finalized = false;
    existing_file = false;
    Flush_wait = 1000000;
    Flush_count = 0;
    total_his_size = 0;

    initialize();
    #ifndef NO_HIS_THREAD
    HisFileWriter::SetOfile(&ofile);
    HisFileWriter::SetMaxEventsBetweenWrites(Flush_wait);
    #endif
}

OutputHisFile::OutputHisFile(std::string fname_prefix) {
    fname = "";
    writable = false;
    finalized = false;
    existing_file = false;
    Flush_wait = 1000000;
    Flush_count = 0;
    total_his_size = 0;

    initialize();
    Open(fname_prefix);
    #ifndef NO_HIS_THREAD
    HisFileWriter::SetOfile(&ofile);
    HisFileWriter::SetMaxEventsBetweenWrites(Flush_wait);
    #endif
}

OutputHisFile::~OutputHisFile() {
    Close();
}

size_t OutputHisFile::push_back(std::shared_ptr<drr_entry> entry) {
    if (!entry) {
	#ifndef NDEBUG
        if (debug_mode)
            std::cout << "debug: OutputHisFile::push_back was passed a NULL pointer!\n";
        #endif
        return (0);
    } else if (!writable || finalized) {
	#ifndef NDEBUG
        if (debug_mode)
            std::cout << "debug: The .drr and .his files have already been finalized and are locked!\n";
	#endif
        return (0);
    }

    // Search for existing histogram with the same id
    if (find_drr_in_list(entry->hisID)) {
	#ifndef NDEBUG
        if (debug_mode)
            std::cout << "debug: His id = " << entry->hisID << " is already in the drr entry list!\n";
	#endif
        return (0);
    }

    // Seek to the end of this histogram file
    ofile.seekp(0, std::ios::end);
    entry->offset =
            (size_t) ofile.tellp() / 2; // Set the file offset (in 2 byte words)
    AddDrrEntry(entry);

    #ifndef NDEBUG
    if (debug_mode)
        std::cout << "debug: Extending .his file by " << entry->total_size
                  << " bytes for his ID = " << entry->hisID << " i.e. '"
                  << rstrip(entry->title) << "'\n";
    #endif

    char *block = new char[entry->total_size];
    memset(block, 0x0, entry->total_size);
    ofile.write(block, entry->total_size);
    delete[] block;

    ofile.seekp(0, std::ios::end);
    total_his_size = ofile.tellp();

    return entry->total_size;
}



bool OutputHisFile::Finalize(bool make_list_file_/*=false*/,
                             const std::string &descrip_/*="RootPixieScan .drr file"*/) {
    if (!writable || finalized) {
	#ifndef NDEBUG
        if (debug_mode)
            std::cout << "debug: The .drr and .his files have already been finalized and are locked!\n";
	#endif
        return (false);
    }

    bool retval = true;

    set_char_array(initial, "HHIRFDIR0001", 12);
    set_char_array(description, descrip_, 40);

    nHis = CountDrrEntries();
    #ifndef NDEBUG
    if (debug_mode)
        std::cout << "debug: NHIS = " << nHis << std::endl;
    #endif
    
    nHWords = (128 * (1 + nHis) + nHis * 4) / 2;

    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    date[0] = 0;
    date[1] = timeinfo->tm_year + 1900; // tm_year measures the year from 1900
    date[2] = timeinfo->tm_mon; // tm_mon ranges from 0 to 11
    date[3] = timeinfo->tm_mday;
    date[4] = timeinfo->tm_hour;
    date[5] = timeinfo->tm_min;

    // Write the .drr file
    std::ofstream drr_file((fname + ".drr").c_str(), std::ios::binary);
    if (drr_file.good()) {
        char dummy = 0x0;
        int his_id;

        // Write the 128 byte drr header
        drr_file.write(initial, 12);
        drr_file.write((char *) &nHis, 4);
        drr_file.write((char *) &nHWords, 4);
        for (int i = 0; i < 6; i++) { drr_file.write((char *) &date[i], 4); }
        for (int i = 0; i < 44; i++) {
            drr_file.write(&dummy, 1);
        } // add the trailing garbage
        drr_file.write(description, 40);

        // Write the drr entries
        for (auto &entry: drr_entry_map) {
            if(entry == nullptr){
		    continue;
	    }
            #ifndef NDEBUG
            if (debug_mode)
                std::cout << "debug: Writing .drr entry for his id = "
                          << entry->hisID << std::endl;
            #endif
            entry->print_drr(&drr_file);
        }

        // Write the histogram IDs
        for (auto &entry: drr_entry_map) {
            if(entry == nullptr){continue;}
            his_id = entry->hisID;
            drr_file.write((char *) &his_id, 4);
        }
    } else {
        #ifndef NDEBUG
        if (debug_mode)
            std::cout << "debug: Failed to open the .drr file for writing!\n";
        #endif
        retval = false;
    }
    drr_file.close();

    // Write the .list file (I'm trying to preserve the format of the original file)
    std::ofstream list_file((fname + ".list").c_str());
    if (list_file.good()) {
        int temp_count = 0;
        list_file << std::setw(7) << CountDrrEntries() << " HISTOGRAMS,"
                  << std::setw(13) << total_his_size / 2
                  << " HALF-WORDS\n ID-LIST:\n";
        for (auto &entry: drr_entry_map) {
            if(entry == nullptr){continue;}
            if (temp_count % 8 == 0 && temp_count != 0)
                list_file << std::endl;
            
            list_file << std::setw(8) << entry->hisID;
            temp_count++;
        }
        list_file
                << "\n  HID  DIM HWPC  LEN(CH)   COMPR  MIN   MAX   OFFSET    TITLE\n";
        for (auto &entry: drr_entry_map) {
            if(entry == nullptr){continue;}
            entry->print_list(&list_file);
        }
    } else {
	#ifndef NDEBUG
        if (debug_mode)
            std::cout << "debug: Failed to open the .list file for writing!\n";
        #endif
        retval = false;
    }
    list_file.close();

    finalized = true;

    return retval;
}

void OutputHisFile::PushWrite(WriteQueueObject evt){
    #ifdef NO_HIS_THREAD
        if(waiting_writes.find(evt.bufferLocation) == waiting_writes.end())
        {
            waiting_writes.insert({evt.bufferLocation, {evt.weight, evt.isInt}});
        }
        else
        {
            waiting_writes.at(evt.bufferLocation).first += evt.weight;
        }
    #else
        waiting_to_enqueue.push_back(evt);
    #endif
    if(++Flush_count > Flush_wait)
    {
        Flush();
    }
}


bool OutputHisFile::Fill(unsigned int hisID_, unsigned int x_, unsigned int y_,
                         unsigned int weight_/*=1*/) {
    if (!writable)[[unlikely]]{return false;}

    std::shared_ptr<drr_entry> temp_drr = find_drr_in_list(hisID_);
    if (temp_drr == nullptr) {
        undef_his_failed_fills.insert(hisID_);
        return false;
    }
    unsigned int bin;
    temp_drr->total_counts++;
    if (!temp_drr->find_bin((unsigned int) (x_ / temp_drr->comp[0]),
                            (unsigned int) (y_ / temp_drr->comp[1]), bin)){
        bin_not_found_failed_fills[hisID_] += 1;
        return false;
    }
    // Push this fill into the queue
    WriteQueueObject evt = WriteQueueObject(temp_drr->calculate_buffer_location(bin), temp_drr->use_int, weight_);
    PushWrite(evt);
    return true;
}


bool OutputHisFile::FillBin(unsigned int hisID_, unsigned int x_, unsigned int y_,
                       unsigned int weight_) {
    if (!writable)[[unlikely]] { return false; }

    std::shared_ptr<drr_entry> temp_drr = find_drr_in_list(hisID_);
    if (temp_drr == nullptr) {
        undef_his_failed_fills.insert(hisID_);
        return false;
    }
    unsigned int bin;
    temp_drr->total_counts++;
    if (!temp_drr->get_bin(x_, y_, bin)) { 
        bin_not_found_failed_fills[hisID_] += 1;
        return false; 
    }
    temp_drr -> good_counts++;
    // Push this fill into the queue
    WriteQueueObject evt = WriteQueueObject(temp_drr->calculate_buffer_location(bin), temp_drr->use_int, weight_);
    PushWrite(evt);
    return true;
}

bool OutputHisFile::Zero(unsigned int hisID_) {
    if (!writable) { return false; }

    std::shared_ptr<drr_entry> temp_drr = find_drr_in_list(hisID_);
    if (temp_drr) {
        ofile.seekp(temp_drr->offset * 2, std::ios::beg);

        char *block = new char[temp_drr->total_size];
        memset(block, 0x0, temp_drr->total_size);
        ofile.write(block, temp_drr->total_size);
        delete[] block;

        return true;
    }

    return false;
}

bool OutputHisFile::Zero() {
    if (!writable)
        return false;

    for (auto &entry: drr_entry_map) {
        ofile.seekp(entry->offset * 2, std::ios::beg);
        char *block = new char[entry->total_size];
        memset(block, 0x0, entry->total_size);
        ofile.write(block, entry->total_size);
        delete[] block;
    }
    return true;
}

bool OutputHisFile::Open(std::string fname_prefix) {
    if (writable) {
        if (debug_mode) {
            std::cout << "debug: The .his file is already open!\n";
        }
        return false;
    }

    fname = fname_prefix;
    existing_file = false;

    //touch.close();
    ofile.open((fname + ".his").c_str(),
               std::ios::out | std::ios::in | std::ios::trunc |
               std::ios::binary);
    #ifndef NO_HIS_THREAD
    HisFileWriter::SetOfile(&ofile);
    HisFileWriter::Start();
    #endif
    return (writable = ofile.good());
}

void OutputHisFile::Close() {
    Flush();

    if (!finalized) { Finalize(); }

    // Write the .log file
    std::ofstream log_file((fname + ".log").c_str());
    if (log_file.good()) {
        log_file << "  HID      TOTAL      GOOD\n\n";
        for (auto &entry: drr_entry_map) {
            if(entry == nullptr){continue;}
            log_file << std::setw(5) << entry->hisID << std::setw(10)
                     << entry->total_counts << std::setw(10)
                     << entry->good_counts << std::endl;
        }
        log_file << "\nFailed histogram fills with bad or out of range bin:\n\n";
        int ct = 0;
        for (auto &i: bin_not_found_failed_fills) {
            log_file << std::setw(5) << i.first << "-" << std::setw(10) << i.second;
            if(ct++%5 ==0 && ct != 0)
            {
                log_file<< std::endl;
            }
        }
        log_file << "\nFailed Histogram Fills For Undefined Histogram:\n\n";
        ct = 0;
        for (auto &i: undef_his_failed_fills) {
            log_file << std::setw(5) << i;
            if(ct++%5 ==0 && ct != 0)
            {
                log_file<< std::endl;
            }
        }
    #ifndef NDEBUG
    } else if (debug_mode) {
        std::cout << "debug: Failed to open the .log file for writing!\n";
    #endif
    }
    log_file.close();

    // Clear the .drr entries in the entries vector
    clear_drr_entries();
    writable = false;
    #ifndef NO_HIS_THREAD
    HisFileWriter::Stop();
    #endif
    ofile.close();
}
