
#ifndef __OUTPUTHISFILE_HPP__
#define __OUTPUTHISFILE_HPP__

#include <iostream>
#include <fstream>
#include <vector>

#include "HisFile.hpp"

class OutputHisFile : public HisFile {
private:
    std::fstream ofile; /// The output .his file stream
    std::string fname; /// The output filename prefix
    bool writable; /// True if the output .his file is open and writable
    bool finalized; /// True if the .his and .drr files are locked
    bool existing_file; /// True if the .his file was a previously existing file
    unsigned int Flush_wait; /// Number of fills to wait between Flushes
    unsigned int Flush_count; /// Number of fills since last Flush
    std::vector<fill_queue *> fills_waiting; /// Vector containing list of histograms to be filled
    std::vector<unsigned int> failed_fills; /// Vector containing list of histogram fills into an invalid his id
    std::streampos total_his_size; /// Total size of .his file

    /// Find the specified .drr entry in the drr list using its histogram id
    drr_entry *find_drr_in_list(unsigned int hisID_);

public:
    OutputHisFile();

    OutputHisFile(std::string fname_prefix);

    ~OutputHisFile();

    /// Return true if the output .his file is open and writable and false otherwise
    bool IsWritable() { return writable; }

    /// Return a pointer to the output file
    std::fstream *GetOutputFile() { return &ofile; }

    /// Set the number of fills to wait between file Flushes
    void SetFlushWait(unsigned int wait_) { Flush_wait = wait_; }

    /* Push back with another histogram entry. This command will also
     * extend the length of the .his file (if possible). DO NOT delete
     * the passed drr_entry after calling. OutputHisFile will handle cleanup.
     * On success, returns the number of bytes the file was extended by and zero
     * upon failure.
     */
    size_t push_back(drr_entry *entry_);

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
