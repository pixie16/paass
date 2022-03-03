/** \file ScanInterface.cpp
 * \brief A class to handle reading from various UTK/ORNL pixie16 data formats.
 *
 * This class is intended to be used as a replacement to the older and unsupported
 * 'scanor' program from the UPAK acq library specifically for .ldf files which are
 * constructed using the UTK/ORNL pixie16 style. This class also interfaces with poll2
 * shared memory output without the need to use pacman.
 * CRT
 *
 * \author C. R. Thornsberry, S. V. Paulauskas
 * \date Feb. 12th, 2016
 */
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

#include <cstring>

#include <unistd.h>
#include <getopt.h>

#include "Unpacker.hpp"
#include "poll2_socket.h"
#include "CTerminal.h"

#include "ScanInterface.hpp"

using namespace std;

void start_run_control(ScanInterface *main_) {
    main_->RunControl();
}

void start_cmd_control(ScanInterface *main_) {
    main_->CmdControl();
}

/////////////////////////////////////////////////////////////////////
// class optionExt
/////////////////////////////////////////////////////////////////////

optionExt::optionExt(const char *name_, const int &has_arg_, int *flag_, const int &val_, const string &argstr_,
                     const string &helpstr_) : name(name_), has_arg(has_arg_), flag(flag_), val(val_), argstr(argstr_),
                                               helpstr(helpstr_), active(false) {
}

void
optionExt::print(const size_t &len_/*=0*/, const string &prefix_/*=""*/) {
    stringstream stream;
    stream << prefix_ << "--" << name << " ";
    if (val)
        stream << "(-" << (char) val << ") ";
    stream << argstr;

    if (stream.str().length() < len_)
        stream << string(len_ - stream.str().length(), ' ');

    stream << "- " << helpstr;
    cout << stream.str() << endl;
}

option optionExt::getOption() {
    struct option output;
    output.name = name;
    output.has_arg = has_arg;
    output.flag = flag;
    output.val = val;
    return output;
}

/////////////////////////////////////////////////////////////////////
// class fileInformation
/////////////////////////////////////////////////////////////////////

bool fileInformation::at(const size_t &index_, string &name, string &value) {
    if (index_ >= parnames.size()) { return false; }
    name = parnames.at(index_);
    value = parvalues.at(index_);
    return true;
}

template<typename T>
bool fileInformation::push_back(const string &name_, const T &value_, const string &units_/*=""*/) {
    if (!is_in(name_)) {
        stringstream stream;
        stream << value_;
        if (units_.size() > 0) {
            stream << " " << units_;
        }
        parnames.push_back(name_);
        parvalues.push_back(stream.str());
        return true;
    }
    return false;
}

bool fileInformation::is_in(const string &name_) {
    for (vector<string>::iterator iter = parnames.begin(); iter != parnames.end(); iter++)
        if (name_ == (*iter))
            return true;
    return false;
}

string fileInformation::print(const size_t &index_) {
    if (index_ >= parnames.size()) { return ""; }
    return string(parnames.at(index_) + ": " + parvalues.at(index_));
}

void fileInformation::clear() {
    parnames.clear();
    parvalues.clear();
}

/////////////////////////////////////////////////////////////////////
// class ScanInterface
/////////////////////////////////////////////////////////////////////

///Start the scan, if ScanInterface is initialized and is not already running.
void ScanInterface::start_scan() {
    if (!(file_open || shm_mode))
        cout << " No input file loaded.\n";
    else if (!input_file.good())
        cout << " Error reading from input file!\n";
    else if (input_file.eof())
        cout << " Physical end-of-file reached.\n";
    else if (is_running)
        cout << " Already running.\n";
    else {
        unpacker_->Run();
        is_running = true;
        total_stopped = false;
        if (!batch_mode)
            term->SetStatus("\033[0;33m[IDLE]\033[0m Waiting for Unpacker...");

        // Notify that the user has started the scan.
        Notify("START_SCAN");
    }
}

/** Stop the scan, if it is running.
  * \return Nothing.
  */
void ScanInterface::stop_scan() {
    if (!scan_init) { cout << " Not initialized!\n"; }
    else if (!is_running) { cout << " Not running.\n"; }
    else {
        unpacker_->Stop();
        is_running = false;
        if (!batch_mode)
            term->SetStatus("\033[0;31m[STOP]\033[0m Acquisition stopped.");
        else
            cout << "\033[0;31m[STOP]\033[0m Acquisition stopped.\n";
    }

    // Notify that the user has stopped the scan.
    Notify("STOP_SCAN");
}

/** Print a command line argument help dialogue.
  * \param[in]  name_ The name of the program.
  * \return Nothing.
  */
void ScanInterface::OutputCommandLineHelp(char *name_) {
    SyntaxStr(name_);
    cout << "  Available options:\n";
    for (vector<optionExt>::iterator iter = baseOpts.begin(); iter != baseOpts.end(); iter++) {
        if (!iter->name)
            continue;
        iter->print(40, "   ");
    }
    for (vector<optionExt>::iterator iter = userOpts.begin(); iter != userOpts.end(); iter++) {
        if (!iter->name)
            continue;
        iter->print(40, "   ");
    }
}

/** Seek to a specified position in the file.
  * \param[in]  offset_ The position, with respect to the start of the file, to seek to.
  * \return True upon success and false otherwise.
  */
bool ScanInterface::rewind(const unsigned long &offset_/*=0*/) {
    if (!scan_init)
        return false;

    // Ensure that the scan is not running.
    if (!(file_open || shm_mode)) {
        cout << " No input file loaded.\n";
        return false;
    } else if (is_running) {
        cout << " Cannot change file position while scan is running!\n";
        return false;
    }

    // Move to the first word in the file.
    cout << " Seeking to word no. " << offset_ << " in file\n";
    input_file.seekg(offset_ * 4, input_file.beg);
    cout << " Input file is now at " << input_file.tellg() << " bytes\n";

    // Notify that the user has rewound to the start of the file.
    Notify("REWIND_FILE");

    return true;
}

/** Open a new binary input file for reading.
  * \param[in]  fname_ Input filename to open for reading.
  * \return True upon successfully opening the file and false otherwise.
  */
bool ScanInterface::open_input_file(const string &fname_) {
    if (is_running) {
        cout << " ERROR! Unable to open input file while scan is running.\n";
        return false;
    } else if (shm_mode) {
        cout << " ERROR! Unable to open input file in shm mode.\n";
        return false;
    }

    extension = get_extension(fname_, prefix);
    if (prefix == "") {
        cout << " ERROR! Input filename was not specified!\n";
        return false;
    }

    if (extension == "ldf") { // List data format file
        file_format = 0;
    } else if (extension == "pld") { // Pixie list data file format
        file_format = 1;
    } else if (extension == "evt") { // NSCLDAQ presort ring buffer format
        file_format = 3;
    } else {
        cout << " ERROR! Invalid file format '" << extension << "'\n";
        cout << "  The current valid data formats are:\n";
        cout << "   ldf - list data format (HRIBF)\n";
        cout << "   pld - pixie list data format\n";
        cout << "   evt - NSCLDAQ presort ring buffer format\n";
        return false;
    }

    // Close the previous file, if one is open.
    if (file_open) {
        cout << " Note: Closing previously opened file.\n";
        input_file.close();
    }

    file_open = true;

    // Load the input file.
    input_file.open(fname_.c_str(), ios::binary);
    if (!input_file.is_open() || !input_file.good()) {
        cout << " ERROR! Failed to open input file '" << fname_ << "'! Check that the path is correct.\n";
        input_file.close();
        file_open = false;
        return false;
    }
    input_file.seekg(0, input_file.end);
    file_length = input_file.tellg();
    input_file.seekg(0, input_file.beg);

    if (!shm_mode) {
        // Clear the file information container.
        finfo.clear();

        // Start reading the file
        // Every poll2 ldf file starts with a DIR buffer followed by a HEAD buffer
        int num_buffers;
        if (file_format == 0) {
            dirbuff.Read(&input_file);
            headbuff.Read(&input_file);

            // Store the file information for later use.
            finfo.push_back("Run number", dirbuff.GetRunNumber());
            finfo.push_back("Number buffers", num_buffers);
            finfo.push_back("Facility", headbuff.GetFacility());
            finfo.push_back("Format", headbuff.GetFormat());
            finfo.push_back("Type", headbuff.GetType());
            finfo.push_back("Date", headbuff.GetDate());
            finfo.push_back("Title", headbuff.GetRunTitle());

            dirbuff.Print();
            headbuff.Print();
            cout << endl;
        } else if (file_format == 1) {
            pldHead.Read(&input_file);

            max_spill_size = pldHead.GetMaxSpillSize();

            // Store the file information for later use.
            finfo.push_back("Facility", pldHead.GetFacility());
            finfo.push_back("Format", pldHead.GetFormat());
            finfo.push_back("Start", pldHead.GetStartDate());
            finfo.push_back("Stop", pldHead.GetEndDate());
            finfo.push_back("Title", pldHead.GetRunTitle());
            finfo.push_back("Run number", pldHead.GetRunNumber());
            finfo.push_back("Max spill", max_spill_size, "words");
            finfo.push_back("ACQ time", pldHead.GetRunTime(), "seconds");

            pldHead.Print();
            cout << endl;
        } else if (file_format == 3) {
            // just skip "header" information for now...
        }
    }

    // Notify that the user has loaded a new file.
    Notify("LOAD_FILE");

    return true;
}

/** Add a command line option to the option list.
  * \param[in]  opt_ The option to add to the list.
  * \return Nothing.
  */
void ScanInterface::AddOption(optionExt opt_) {
    char tempChar = opt_.val;
    if (tempChar) {
        if (optstr.find(tempChar) != string::npos)
            opt_.val = 0x0;
        else {
            optstr += tempChar;
            if (opt_.has_arg == required_argument)
                optstr += ":";
            else if (opt_.has_arg == optional_argument)
                optstr += "::";
        }
    }
    userOpts.push_back(opt_);
}

/** SyntaxStr is used to print a linux style usage message to the screen.
  * Prints a standard usage message by default.
  * \param[in]  name_ The name of the program.
  * \return Nothing.
  */
void ScanInterface::SyntaxStr(char *name_) {
    cout << " usage: " << name_ << " [options]\n";
}

void ScanInterface::SetOutputInformation(const string &a) {
    unsigned long found = a.find_last_of("/");
    if (found == string::npos) {
        outputFilename_ = a;
        outputPath_ = "./";
    } else {
        outputFilename_ = a.substr(found + 1, a.size());
        outputPath_ = a.substr(0, found + 1);
    }
}

/** Initialize the Unpacker object. 
  * Does nothing useful by default.
  * \param[in]  prefix_ String to append to the beginning of system output.
  * \return True upon successfully initializing and false otherwise.
  */
bool ScanInterface::Initialize(string prefix_) {
    if (scan_init) { return false; }
    return (scan_init = true);
}

///Default constructor. We set quite a number of default values here
///@param[in]  unpacker Pointer to an object derived from Unpacker.
ScanInterface::ScanInterface() {
    prefix = "";
    extension = "";

    // Get the current working directory.
    char workingDirectory[1024];
    workDir = string(getcwd(workingDirectory, 1024));

    // Get the home directory.
    homeDir = getenv("HOME");

    maxShmSizeL = 4052;
    maxShmSize = maxShmSizeL * 4;

    max_spill_size = 0;
    file_format = -1;

    file_start_offset = 0;
    num_spills_recvd = 0;

    total_stopped = true;
    write_counts = false;
    is_running = false;
    is_verbose = true;
    debug_mode = false;
    dry_run_mode = false;
    shm_mode = false;
    batch_mode = false;
    scan_init = false;
    file_open = false;

    //Initialize the setup and output file names and path
    outputFilename_ = "";
    outputPath_ = "";
    setup_filename = "";

    kill_all = false;
    run_ctrl_exit = false;

    poll_server = NULL;
    term = NULL;

    //Setup all the arguments that are known to the program.
    baseOpts = {
            optionExt("batch", no_argument, NULL, 'b', "", "Run in batch mode (i.e. with no command line)"),
            optionExt("config", required_argument, NULL, 'c', "<path>", "Specify path to setup to use for scan"),
            optionExt("counts", no_argument, NULL, 0, "", "Write all recorded channel counts to a file"),
            optionExt("debug", no_argument, NULL, 0, "", "Enable readout debug mode"),
            optionExt("dry-run", no_argument, NULL, 0, "", "Extract spills from file, but do no processing"),
            optionExt("fast-fwd", required_argument, NULL, 0, "<word>",
                      "Skip ahead to a specified word in the file (start of file at zero)"),
            optionExt("firmware", required_argument, NULL, 'f', "<firmware>", "Sets the firmware revision for decoding the data. "
                              "See the wiki or HelperEnumerations.hpp for more information."),
            optionExt("frequency", required_argument, NULL, 0, "<frequency in MHz or MS/s>",
                      "Specifies the sampling frequency used to collect the data."),
            optionExt("help", no_argument, NULL, 'h', "", "Display this dialogue"),
            optionExt("input", required_argument, NULL, 'i', "<filename>", "Specifies the input file to analyze"),
            optionExt("output", required_argument, NULL, 'o', "<filename>",
                      "Specifies the name of the output file. Default is \"out\""),
            optionExt("quiet", no_argument, NULL, 'q', "", "Toggle off verbosity flag"),
            optionExt("shm", no_argument, NULL, 's', "", "Enable shared memory readout"),
            optionExt("version", no_argument, NULL, 'v', "", "Display version information")
    };

    knownArgumentMap_.insert(make_pair("debug", "Toggle debug mode flag (default=false)"));
    knownArgumentMap_.insert(make_pair("quiet", "Toggle quiet mode flag (default=false)"));
    knownArgumentMap_.insert(make_pair("quit", "Close the program"));
    knownArgumentMap_.insert(make_pair("help", "Usage (h)elp : <cmd> | Display this dialogue or just the dialog for <cmd>"));
    knownArgumentMap_.insert(make_pair("version", "Usage : (v)ersion | Display version information."));
    knownArgumentMap_.insert(make_pair("run", "Start acquisition"));
    knownArgumentMap_.insert(make_pair("stop", "Stop acquisition"));
    knownArgumentMap_.insert(make_pair("file", "Usage : file <fileName> | Load an input file."));
    knownArgumentMap_.insert(make_pair("rewind", "Usage : rewind [offset] | Rewind to the beginning of the file or to the "
            "requested number of words"));
    knownArgumentMap_.insert(make_pair("sync", "Wait for the current run to finish"));

    optstr = "bc:f:hi:o:qsv";

    progName = "ScanInterface";
    msgHeader = progName + ": ";
}

/// Default destructor.
ScanInterface::~ScanInterface() {
    Close();
}

/// Main scan control method.
void ScanInterface::RunControl() {
    // Notify that we are starting run control.
    run_ctrl_exit = false;

    // Set debug mode, if enabled.
    if (debug_mode) {
        pldHead.SetDebugMode();
        pldData.SetDebugMode();
        dirbuff.SetDebugMode();
        headbuff.SetDebugMode();
        databuff.SetDebugMode();
        eofbuff.SetDebugMode();
    }

    while (true) {
        if (kill_all) { break; }

        // Now we're ready to read the first data buffer
        if (total_stopped) {
            // Sleep while waiting for the user to scan more data.
            IdleTask();
            usleep(1);
            continue;
        } else if (shm_mode) {
            cout << endl;
            unsigned int data[250000]; // Array for storing spill data. Larger than any RevF spill should be.
            unsigned int *shm_data = new unsigned int[maxShmSizeL]; // Array to store the temporary shm data (~16 kB)
            int dummy;
            int previous_chunk;
            int current_chunk;
            int total_chunks;
            int nWords;
            unsigned int nTotalWords;

            bool full_spill = false;

            while (true) {
                if (kill_all == true) {
                    break;
                } else if (!is_running) {
                    IdleTask();
                    usleep(100000); //0.1 seconds
                    continue;
                }

                int select_dummy;
                previous_chunk = 0;
                current_chunk = 0;
                total_chunks = -1;
                nTotalWords = 0;
                full_spill = true;

                if (!poll_server->Select(dummy)) {
                    if (!batch_mode) {
                        term->SetStatus("\033[0;33m[IDLE]\033[0m Waiting for a spill...");
                    } else {
                        cout << "\r\033[0;33m[IDLE]\033[0m Waiting for a spill...";
                    }
                    IdleTask();
                    continue;
                }

                if (!poll_server->Select(
                        select_dummy)) { continue; } // Server timeout

                // Get the spill
                while (current_chunk != total_chunks) {
                    if (!poll_server->Select(select_dummy)) { // Server timeout
                        cout << msgHeader << "Network timeout before recv full spill!\n";
                        full_spill = false;
                        break;
                    }

                    nWords = poll_server->RecvMessage((char *) shm_data, maxShmSize) / 4; // Read from the socket
                    if (strcmp((char *) shm_data, "$CLOSE_FILE") == 0 || strcmp((char *) shm_data, "$OPEN_FILE") == 0 ||
                        strcmp((char *) shm_data, "$KILL_SOCKET") == 0) { continue; } // Poll2 network flags
                        // Did not read enough bytes
                    else if (nWords < 2) {
                        continue;
                    }

                    if (debug_mode) {
                        cout << "debug: Received " << nWords << " words from the network\n";
                    }
                    memcpy((char *) &current_chunk, &shm_data[0], 4);
                    memcpy((char *) &total_chunks, &shm_data[1], 4);

                    if (previous_chunk == -1 && current_chunk != 1) {
                        // Started reading in the middle of a spill, ignore the rest of it
                        if (debug_mode) {
                            cout << "debug: Skipping chunk " << current_chunk << " of " << total_chunks << endl;
                        }
                        continue;
                    } else if (previous_chunk != current_chunk - 1) { // We missed a spill chunk somewhere
                        if (debug_mode) {
                            cout << "debug: Found chunk " << current_chunk << " but expected chunk "
                                 << previous_chunk + 1 << endl;
                        }
                        break;
                    }

                    previous_chunk = current_chunk;

                    // Copy the shm spill chunk into the data array
                    if (nTotalWords + 2 + nWords <= 250000) { // This spill chunk will fit into the data buffer
                        memcpy(&data[nTotalWords], &shm_data[2], (nWords - 2) * 4);
                        nTotalWords += (nWords - 2);
                    } else {
                        if (debug_mode) {
                            cout << "debug: Abnormally full spill buffer with " << nTotalWords + 2 + nWords << " words!\n";
                        }
                        break;
                    }
                }

                stringstream status;
                status << "\033[0;32m" << "[RECV] " << "\033[0m" << nTotalWords
                       << " words";
                if (!batch_mode) { term->SetStatus(status.str()); }
                else { cout << "\r" << status.str(); }

                if (debug_mode) {
                    cout << "debug: Retrieved spill of " << nTotalWords << " words (" << nTotalWords * 4 << " bytes)\n";
                }
                if (!dry_run_mode && full_spill) {
                    int word1 = 2, word2 = 9999;
                    memcpy(&data[nTotalWords], (char *) &word1, 4);
                    memcpy(&data[nTotalWords + 1], (char *) &word2, 4);
                    unpacker_->ReadSpill(data, nTotalWords + 2, is_verbose);
                    IdleTask();
                }

                if (!full_spill) {
                    cout << msgHeader << "Not processing spill fragment!\n";
                } else { num_spills_recvd++; }
            }

            delete[] shm_data;
        } else if (file_format == 0) {
            if (debug_mode) cout << "debug: file_format == 0: ldf" << endl;

            unsigned int *data = NULL;
            bool full_spill;
            bool bad_spill;
            unsigned int nBytes;

            if (!dry_run_mode) { data = new unsigned int[250000]; }

            // Reset the buffer reader to default values.
            databuff.Reset();

            while (true) {
                if (kill_all == true) {
                    break;
                } else if (!is_running) {
                    IdleTask();
                    usleep(100000); //0.1 seconds
                    continue;
                }

                if (!databuff.Read(&input_file, (char *) data, nBytes, 1000000, full_spill, bad_spill, dry_run_mode)) {
                    if (databuff.GetRetval() == 1) {
                        if (debug_mode) {
                            cout << "debug: Encountered single EOF buffer (end of run).\n";
                        }
                    } else if (databuff.GetRetval() == 2) {
                        if (debug_mode) {
                            cout << "debug: Encountered double EOF buffer (end of file).\n";
                        }
                        break;
                    } else if (databuff.GetRetval() == 3) {
                        if (debug_mode) {
                            cout << "debug: Encountered unknown ldf buffer type.\n";
                        }
                    } else if (databuff.GetRetval() == 4) {
                        if (debug_mode) {
                            cout << "debug: Encountered invalid spill chunk.\n";
                        }
                    } else if (databuff.GetRetval() == 5) {
                        if (debug_mode) {
                            cout << "debug: Received bad spill footer size.\n";
                        }
                    } else if (databuff.GetRetval() == 6) {
                        if (debug_mode) {
                            cout << "debug: Failed to read buffer from input file.\n";
                        }
                        break;
                    }
                    continue;
                }

                stringstream status;
                status << "\033[0;32m" << "[READ] " << "\033[0m" << nBytes / 4 << " words ("
                       << 100 * input_file.tellg() / file_length << "%), ";
                status << "GOOD = " << databuff.GetNumChunks() << ", LOST = " << databuff.GetNumMissing();
                if (!batch_mode) { term->SetStatus(status.str()); }
                else { cout << "\r" << status.str(); }

                if (full_spill) {
                    if (debug_mode) {
                        cout << "debug: Retrieved spill of " << nBytes << " bytes (" << nBytes / 4 << " words)\n";
                        cout << "debug: Read up to word number " << input_file.tellg() / 4 << " in input file\n";
                    }
                    if (!dry_run_mode) {
                        if (!bad_spill) {
                            unpacker_->ReadSpill(data, nBytes / 4, is_verbose);
                            IdleTask();
                        } else {
                            cout << " WARNING: Spill has been flagged as corrupt, skipping (at word " << input_file.tellg() / 4
                                 << " in file)!\n";
                        }
                    }
                } else if (debug_mode) {
                    cout << "debug: Retrieved spill fragment of " << nBytes << " bytes (" << nBytes / 4 << " words)\n";
                    cout << "debug: Read up to word number " << input_file.tellg() / 4 << " in input file\n";
                }
                num_spills_recvd++;
            }

            if (!dry_run_mode) { delete[] data; }

            if (!batch_mode) {
                term->SetStatus("\033[0;33m[IDLE]\033[0m Finished scanning file.");
            } else { cout << endl << endl; }
        } else if (file_format == 1) {
            if (debug_mode) cout << "debug: file_format == 1: pld" << endl;

            unsigned int *data = NULL;
            unsigned int nBytes;

            if (!dry_run_mode) { data = new unsigned int[max_spill_size + 2]; }

            // Reset the buffer reader to default values.
            pldData.Reset();

            while (pldData.Read(&input_file, (char *) data, nBytes, 4 * max_spill_size, dry_run_mode)) {
                if (kill_all == true) {
                    break;
                } else if (!is_running) {
                    IdleTask();
                    usleep(100000); //0.1 seconds
                    continue;
                }

                stringstream status;
                status << "\033[0;32m" << "[READ] " << "\033[0m" << nBytes / 4 << " words ("
                       << 100 * input_file.tellg() / file_length << "%)";
                if (!batch_mode) { term->SetStatus(status.str()); }
                else { cout << "\r" << status.str(); }

                if (debug_mode) {
                    cout << "debug: Retrieved spill of " << nBytes << " bytes (" << nBytes / 4 << " words)\n";
                    cout << "debug: Read up to word number " << input_file.tellg() / 4 << " in input file\n";
                }

                if (!dry_run_mode) {
                    int word1 = 2, word2 = 9999;
                    memcpy(&data[(nBytes / 4)], (char *) &word1, 4);
                    memcpy(&data[(nBytes / 4) + 1], (char *) &word2, 4);
                    unpacker_->ReadSpill(data, nBytes / 4 + 2, is_verbose);
                    IdleTask();
                }
                num_spills_recvd++;
            }

            if (eofbuff.ReadHeader(&input_file)) {
                cout << msgHeader << "Encountered EOF buffer.\n";
            } else {
                cout << msgHeader << "Failed to find end of file buffer!\n";
            }

            if (!dry_run_mode) { delete[] data; }

            if (!batch_mode) {
                term->SetStatus("\033[0;33m[IDLE]\033[0m Finished scanning file.");
            } else { cout << endl << endl; }
        } else if (file_format == 2) {
            if (debug_mode) cout << "debug: file_format == 2: root (not implemented)" << endl;
        }
        else if (file_format == 3) {
            if (debug_mode) cout << "debug: file_format == 3: evt" << endl;

            std::vector<unsigned int> spillbuf; // spill buffer
            std::vector<unsigned int> modfifo; // module fifo
            int prevmodn = 0;
            unsigned int prevnBytes = 0;

            while (true) {
                if (kill_all == true) {
                    break;
                } else if (!is_running) {
                    IdleTask();
                    usleep(100000); //0.1 seconds
                    continue;
                }

                if (!input_file.is_open() || !input_file.good()) { break; }
                std::vector<unsigned int> modfifofrag; // module fifo fragment, this is needed because module fifo data is chopped into multiple ring items
                unsigned int nBytes = 0; // module fifo data fragment size in bytes
                // ring item size is self inclusive
                unsigned int ringitemsize = 0;
                unsigned int ringitemtype;
                unsigned int bodyhdrsize = 0;
                input_file.read((char *) &ringitemsize, 4); // ring item size (in bytes) self inclusive
                input_file.read((char *) &ringitemtype, 4); // 30 for PHYSICS_EVENTS
                if (ringitemtype == 30) {
                    input_file.read((char *) &bodyhdrsize, 4);
                    if (bodyhdrsize == 0) { // PHYSICS_EVENT of pre-sort ring has no body header
                        if (debug_mode) std::cout << "debug: got a PHYSICS_EVENT item (ring item type " << ringitemtype << ")" << std::endl;
                        // skip two words inserted by NSCLDAQ
                        input_file.seekg(8, input_file.cur);
                        nBytes = ringitemsize-20;
                        // this is raw pixie list-mode data
                        modfifofrag.resize(nBytes/4);
                        input_file.read((char *) modfifofrag.data(), nBytes);
                    } else { // if body header size is NOT zero, it's NOT a PHYSICS_EVENT we are looking for
                        if (debug_mode) std::cout << "debug: got a PHYSICS_EVENT item (ring item type " << ringitemtype << ") but non-zero body header size" << std::endl;
                        // most likely bodyhdrsize == 20 but it doesn't matter, just skip the rest
                        input_file.seekg(ringitemsize-12, input_file.cur);
                    }
                } else {
                    if (debug_mode) std::cout << "debug: got a non-PHYSICS_EVENT item (ring item type " << ringitemtype << "), skipping..." << std::endl;
                    input_file.seekg(ringitemsize-8, input_file.cur);
                    if(input_file.eof()) break;
                }

                if (nBytes == 0) { continue; }

                stringstream status;
                status << "\033[0;32m" << "[READ] " << "\033[0m" << nBytes / 4 << " words ("
                       << 100 * input_file.tellg() / file_length << "%)";
                if (!batch_mode) { term->SetStatus(status.str()); }
                else { cout << "\r" << status.str(); }

                if (debug_mode) {
                    cout << "debug: Retrieved *partial* module fifo data of " << nBytes << " bytes (" << nBytes / 4 << " words)\n";
                    cout << "debug: Read up to word number " << input_file.tellg() / 4 << " in input file\n";
                }

                if (!dry_run_mode) {
                    int modn = 9999;
                    // peek the first pixie event
                    // a bit of sanity check + module (slot) number extraction
                    if (nBytes >= 32) { // there's one pixie event at least
                        unsigned int pixhead1 = modfifofrag[0];
                        // this is revision specific!! has to be changed for RevH
                        modn = (pixhead1 >> 4) & 0xf;
                        modn -= 2; // modnum is slotnum - 2
                        if (debug_mode) {
                            std::cout << "debug: first pixie event header in this module fifo fragment 0x"  << std::setfill('0') << std::setw(8) << std::right << std::hex << pixhead1;
                            std::cout << " (module number " << std::dec << modn << ")" << std::endl;
                        }
                        if (modn < 0) {
                            std::cout << "invalid slot number, likely corrupted data" << std::endl;
                            continue;
                        }
                    } else {
                        std::cout << "data size of this ring item is too small (" << nBytes << " bytes), likely corrupted data" << std::endl;
                        continue;
                    }

                    // detect completion of module fifo
                    if (prevmodn < modn) {
                        if (debug_mode) std::cout << "debug: module fifo completion detected" << std::endl;
                        spillbuf.push_back(modfifo.size()+2); // number of words (including this header)
                        spillbuf.push_back(prevmodn);
                        spillbuf.insert(spillbuf.end(), modfifo.begin(), modfifo.end());
                        modfifo.clear();
                    }
                    // detect completion of a spill
                    // second condition is for systems where only one module is present
                    else if ( (prevmodn > modn) || (prevmodn == modn && prevnBytes < nBytes)){
                        if (debug_mode) std::cout << "debug: spill completion detected" << std::endl;
                        spillbuf.push_back(modfifo.size()+2); // number of words (including this header)
                        spillbuf.push_back(prevmodn);
                        spillbuf.insert(spillbuf.end(), modfifo.begin(), modfifo.end());
                        // spill delimiter
                        spillbuf.push_back(2);
                        spillbuf.push_back(9999);
                        unpacker_->ReadSpill(spillbuf.data(), spillbuf.size(), is_verbose);
                        IdleTask();
                        num_spills_recvd++;
                        spillbuf.clear();
                        modfifo.clear();
                    }

                    modfifo.insert(modfifo.end(), modfifofrag.begin(), modfifofrag.begin()+nBytes/4);
                    prevnBytes = nBytes;
                    prevmodn = modn;
                }
            }

            if (!dry_run_mode) {
                if (debug_mode) std::cout << "debug: closing out last spill" << std::endl;
                if (spillbuf.size()>0 || modfifo.size()>0) {
                    spillbuf.push_back(modfifo.size()+2); // number of words (including this header)
                    spillbuf.push_back(prevmodn);
                    spillbuf.insert(spillbuf.end(), modfifo.begin(), modfifo.end());
                    spillbuf.push_back(2);
                    spillbuf.push_back(9999);
                    unpacker_->ReadSpill(spillbuf.data(), spillbuf.size(), is_verbose);
                    num_spills_recvd++;
                }
            }

            if (!batch_mode) {
                term->SetStatus("\033[0;33m[IDLE]\033[0m Finished scanning file.");
            } else { cout << endl << endl; }
        }

        // Notify that the scan has completed.
        Notify("SCAN_COMPLETE");

        total_stopped = true;
        stop_scan();

        if (batch_mode) { break; }
    }

    // Notify that run control is exiting.
    run_ctrl_exit = true;
}

/// Main command interpreter method.
void ScanInterface::CmdControl() {
    if (!unpacker_) { return; }

    string cmd = "", arg;
    bool waiting_for_run = false;

    while (true) {
        if (run_ctrl_exit) { break; }

        if (waiting_for_run) {
            if (!is_running) { waiting_for_run = false; }
            else {
                term->flush(); // Update the terminal so the user knows something is happening.
                sleep(1); // Sleep and wait for the run to finish.
                continue;
            }
        }

        cmd = term->GetCommand(arg);
        if (cmd == "_SIGSEGV_") {
            cout << "\033[0;31m[SEGMENTATION FAULT]\033[0m\n";
            exit(EXIT_FAILURE);
        } else if (cmd == "CTRL_D") {
            cout << msgHeader << "Received EOF (ctrl-d) signal. Exiting...\n";
            cmd = "quit";
        } else if (cmd == "CTRL_C") {
            cout << msgHeader << "Warning! Received SIGINT (ctrl-c) signal.\n";
            continue;
        } else if (cmd == "CTRL_Z") {
            cout << msgHeader << "Warning! Received SIGTSTP (ctrl-z) signal.\n";
            continue;
        }

        term->flush();

        if (cmd == "")
            continue;

        // Replace the '~' with the user's home directory.
        if (!arg.empty() && arg.find('~') != string::npos)
            arg.replace(arg.find('~'), 1, homeDir);

        vector<string> arguments;
        unsigned int p_args = split_str(arg, arguments);

        if (cmd == "quit" || cmd == "exit") {
            stop_scan();
            kill_all = true;
            while (!run_ctrl_exit) { sleep(1); }
            break;
        } else if (cmd == "version" || cmd == "v") {
            cout << "  " << progName << "	  v" << SCAN_VERSION << " (" << SCAN_DATE << ")\n";
            cout << "  Poll2 Socket  v" << POLL2_SOCKET_VERSION << " (" << POLL2_SOCKET_DATE << ")\n";
            cout << "  HRIBF Buffers v" << HRIBF_BUFFERS_VERSION << " (" << HRIBF_BUFFERS_DATE << ")\n";
            cout << "  CTerminal	 v" << CTERMINAL_VERSION << " (" << CTERMINAL_DATE << ")\n";
        } else if (cmd == "help" || cmd == "h") {
            cout << "  Help:\n";
            if(p_args == 0) {
                OutputCommandHelpArguments(knownArgumentMap_);
                OutputCommandHelpArguments(auxillaryKnownArgumentMap_);
            } else if (knownArgumentMap_.find(arg) != knownArgumentMap_.end()) {
                OutputCommandHelpArgument(knownArgumentMap_.find(arg)->first, knownArgumentMap_.find(arg)->second);
            } else if (auxillaryKnownArgumentMap_.find(arg) != auxillaryKnownArgumentMap_.end())
                OutputCommandHelpArgument(auxillaryKnownArgumentMap_.find(arg)->first,
                                          auxillaryKnownArgumentMap_.find(arg)->second);
            else
                OutputUnkownCommandMessage(arg);
        } else if (cmd == "run") { // Start acquisition.
            start_scan();
        } else if (cmd == "stop") { // Stop acquisition.
            stop_scan();
        } else if (cmd == "debug") { // Toggle debug mode
            if (debug_mode) {
                cout << msgHeader << "Toggling debug mode OFF\n";
                debug_mode = false;
            } else {
                cout << msgHeader << "Toggling debug mode ON\n";
                debug_mode = true;
            }
            unpacker_->SetDebugMode(debug_mode);
        } else if (cmd == "quiet") { // Toggle quiet mode
            if (!is_verbose) {
                cout << msgHeader << "Toggling quiet mode OFF\n";
                is_verbose = true;
            } else {
                cout << msgHeader << "Toggling quiet mode ON\n";
                is_verbose = false;
            }
        } else if (cmd == "file") { // Rewind the file to the start position
            if (p_args > 0) {
                if (!open_input_file(arguments.at(0))) {
                    cout << msgHeader << "Failed to open input file!\n";
                }
            } else {
                cout << msgHeader
                          << "Invalid number of parameters to 'file'\n";
                cout << msgHeader << " -SYNTAX- file <filename>\n";
            }
        } else if (cmd == "rewind") { // Rewind the file to the start position
            if (p_args > 0) {
                rewind(strtoul(arguments.at(0).c_str(), NULL, 0));
            } else { rewind(); }
        } else if (cmd == "sync") { // Wait until the current run is completed.
            if (is_running) {
                cout << msgHeader
                          << "Waiting for current scan to complete.\n";
                waiting_for_run = true;
            } else { cout << msgHeader << "Scan is not running.\n"; }
        } else if (!ExtraCommands(cmd, arguments)) { // Unrecognized command. Send it to a derived object.
            cout << msgHeader << "Unknown command '" << cmd << "'\n";
        }
    }
}

void ScanInterface::OutputCommandHelpArguments(const map<string, string> &args) {
    for (map<string, string>::const_iterator it = args.begin(); it != args.end(); it++)
        OutputCommandHelpArgument((*it).first, (*it).second);
}

void ScanInterface::OutputCommandHelpArgument(const std::string &arg, const std::string &help) {
    cout << "   " << setw(7) << left << arg << resetiosflags(std::ios::showbase) << " - " << help << endl;
}

void ScanInterface::OutputUnkownCommandMessage(const std::string &arg) {
    cout << "You've asked for help with \"" << arg << "\", which is unknown to us." << endl;
}

/** Setup user options and initialize all required objects.
  * \param[in]  argc Number of arguments passed from the command line.
  * \param[in]  argv Array of strings passed as arguments from the command line.
  * \return True upon success and false otherwise.
  */
bool ScanInterface::Setup(int argc, char *argv[], Unpacker *unpacker/*=NULL*/) {
    if (scan_init)
        return false;

    debug_mode = false;
    dry_run_mode = false;
    shm_mode = false;
    num_spills_recvd = 0;
    unsigned int samplingFrequency = 0;
    string firmware = "";
    string input_filename = "";

    // Add derived class options to the option list.
    this->ArgHelp();

    // Build the vector of all command line options.
    for (vector<optionExt>::iterator iter = baseOpts.begin();
         iter != baseOpts.end(); iter++) {
        longOpts.push_back(iter->getOption());
    }
    for (vector<optionExt>::iterator iter = userOpts.begin();
         iter != userOpts.end(); iter++) {
        longOpts.push_back(iter->getOption());
    }

    // Append all zeros onto the option list. Required for getopt_long.
    struct option zero_opt{0, 0, 0, 0};
    longOpts.push_back(zero_opt);

    int idx = 0;
    int retval = 0;

    //getopt_long is not POSIX compliant. It is provided by GNU. This may mean
    //that we are not compatible with some systems. If we have enough
    //complaints we can either change it to getopt, or implement our own class.
    while ((retval = getopt_long(argc, argv, optstr.c_str(), longOpts.data(),
                                 &idx)) != -1) {
        if (retval == 0x0) { // Long option
            if (strcmp("config", longOpts[idx].name) == 0) {
                setup_filename = optarg;
            } else if (strcmp("counts", longOpts[idx].name) == 0) {
                write_counts = true;
            } else if (strcmp("debug", longOpts[idx].name) == 0) {
                debug_mode = true;
            } else if (strcmp("dry-run", longOpts[idx].name) == 0) {
                dry_run_mode = true;
            } else if (strcmp("fast-fwd", longOpts[idx].name) == 0) {
                file_start_offset = atoll(optarg);
            } else if (strcmp("frequency", longOpts[idx].name) == 0)
                samplingFrequency = (unsigned int) stoi(optarg);
            else if (strcmp("firmware", longOpts[idx].name) == 0)
                firmware = optarg;
            else {
                for (vector<optionExt>::iterator iter = userOpts.begin();
                     iter != userOpts.end(); iter++) {
                    if (strcmp(iter->name, longOpts[idx].name) == 0) {
                        iter->active = true;
                        if (optarg)
                            iter->argument = string(optarg);
                        break;
                    }
                }
            }
        } else if (retval == 0x3F) { // Unknown option, '?'
            return false;
        } else { // Single character option.
            switch (retval) {
                case 'b' :
                    batch_mode = true;
                    break;
                case 'c' :
                    setup_filename = optarg;
                    break;
                case 'f' :
                    firmware = optarg;
                    break;
                case 'h' :
                    OutputCommandLineHelp(argv[0]);
                    return false;
                case 'i' :
                    input_filename = optarg;
                    break;
                case 'o' :
                    SetOutputInformation(optarg);
                    break;
                case 'q' :
                    is_verbose = false;
                    break;
                case 's':
                    file_format = 0;
                    shm_mode = true;
                    break;
                case 'v' :
                    cout << "  " << progName << "	  v" << SCAN_VERSION << " (" << SCAN_DATE << ")\n";
                    cout << "  Poll2 Socket  v" << POLL2_SOCKET_VERSION << " (" << POLL2_SOCKET_DATE << ")\n";
                    cout << "  HRIBF Buffers v" << HRIBF_BUFFERS_VERSION << " (" << HRIBF_BUFFERS_DATE << ")\n";
                    cout << "  CTerminal	 v" << CTERMINAL_VERSION << " (" << CTERMINAL_DATE << ")\n";
                    return false;
                default :
                    for (vector<optionExt>::iterator iter = userOpts.begin(); iter != userOpts.end(); iter++) {
                        if (retval == iter->val) {
                            iter->active = true;
                            if (optarg)
                                iter->argument = string(optarg);
                            break;
                        }
                    }
                    break;
            }
        }
    }//while

    if (!unpacker)
        throw invalid_argument("ScanInterface::Setup - The Unpacker object has not been set properly.");
    else
        unpacker_ = unpacker;

    if (samplingFrequency != 0 || firmware != "") {
        if (samplingFrequency == 0)
            throw invalid_argument("ScanInterface::Setup - The frequency has not been set.");
        if (firmware == "")
            throw invalid_argument("ScanInterface::Setup - The firmware has not been set.");
        unpacker_->InitializeDataMask(firmware, samplingFrequency);
    } else if(setup_filename != "")
        unpacker_->InitializeDataMask(setup_filename);
    else
        throw invalid_argument("ScanInterface::Setup - Firmware/Frequency Flags or Config file are not set properly. "
                                       "Cannot Initialize Data Mask.");

    if (debug_mode)
        unpacker_->SetDebugMode();

    // Parse for any extra arguments that are known to the derived class.
    ExtraArguments();

    // Initialize everything.
    cout << msgHeader << "Initializing derived class.\n";
    if (!Initialize(msgHeader)) { // Failed to initialize the object. Clean up and exit.
        cout << " FATAL ERROR! Failed to initialize derived class!\n";
        cout << "\nCleaning up...\n";
        return false;
    }

#ifndef USE_HRIBF
    if (shm_mode) {
        poll_server = new Server();
        if (!poll_server->Init(5555, 1)) {
            cout << " FATAL ERROR! Failed to open shm socket 5555!\n" << "\nCleaning up...\n";
            return false;
        }
        if (batch_mode) {
            cout << msgHeader << "Unable to enable batch mode for shared-memory mode!\n";
            batch_mode = false;
        }
    }

    // Initialize the terminal.
    if (!batch_mode) {
        term = new Terminal();
        term->Initialize();
        term->SetCommandHistory(("." + progName + ".cmd").c_str());
        term->SetPrompt((progName + " $ ").c_str());
        term->AddStatusWindow();
        term->SetStatus("\033[0;31m[STOP]\033[0m Acquisition stopped.");
    }

    cout << "\n " << progName << " v" << SCAN_VERSION << "\n";
    cout << " ==  ==  ==  ==  == \n\n";

    if (debug_mode) { cout << msgHeader << "Using debug mode.\n\n"; }
    if (dry_run_mode) { cout << msgHeader << "Doing a dry run.\n\n"; }
    if (shm_mode) {
        cout << msgHeader << "Using shared-memory mode.\n\n";
        cout << msgHeader << "Listening on poll2 SHM port 5555\n\n";
    }

    // Load the input file, if the user has supplied a filename.
    if (!shm_mode && !input_filename.empty()) {
        cout << msgHeader << "Using filename " << input_filename << ".\n";
        if (open_input_file(input_filename)) {
            // Start the scan.
            start_scan();
        } else { cout << msgHeader << "Failed to load input file!\n"; }
    }
#endif

    // Do any last minute initialization.
    try {
        FinalInitialization();
    }
    catch (exception &ex) {
        cout << ex.what() << endl;
        cout << "\nFinal initialization failed!\n";
    }

    return (scan_init = true);
}

/** Run the scan program.
  * This should be called from main().
  * \return Integer return value. 0 on success and 1 otherwise.
  */
int ScanInterface::Execute() {
#ifndef USE_HRIBF
    if (!scan_init) {
        cout << " FATAL ERROR! ScanInterface is not initialized!\n";
        return 1;
    }

    // Seek to the beginning of the file.
    if (file_start_offset != 0)
        rewind();

    // Process the file.
    if (!batch_mode) {
        // Start the run control thread
        cout << "\n Starting data control thread\n";
        thread runctrl(start_run_control, this);

        // Start the command control thread. This needs to be the last thing we do to
        // initialize, so the user cannot enter commands before setup is complete
        cout << " Starting command thread\n\n";
        thread comctrl(start_cmd_control, this);

        // Synchronize the threads and wait for completion
        comctrl.join();
        runctrl.join();
    } else { start_run_control(this); }
#endif
    return 0;
}

/** Shutdown cleanly. Uninitialize the ScanInterface object.
  * \return True upon success and false if ScanInterface has not been initialized.
  */
bool ScanInterface::Close() {
    if (!scan_init)
        return false;
#ifndef USE_HRIBF
    // Close the socket and restore the terminal
    if (!batch_mode) {
        term->Close();
    }

    // Only close the server if this is shared memory mode. Otherwise
    // the server would never have been initialized.
    if (shm_mode) { poll_server->Close(); }

    //Reprint the leader as the carriage was returned
    cout << "Running " << progName << " v" << SCAN_VERSION << " (" << SCAN_DATE << ")\n";
    cout << msgHeader << "Retrieved " << num_spills_recvd << " spills!\n";

    if (input_file.good())
        input_file.close();

    // Clean up detector driver
    cout << "\n" << msgHeader << "Cleaning up...\n";

    // Show the number of lost spill chunks.
    cout << msgHeader << "Read " << databuff.GetNumChunks() << " spill chunks.\n";
    cout << msgHeader << "Lost at least " << databuff.GetNumMissing() << " spill chunks.\n";

    if (write_counts)
        unpacker_->Write();

    if (poll_server) { delete poll_server; }
    if (term) { delete term; }
#endif
    scan_init = false;
    return true;
}

/** Get the file extension from an input filename string.
  * \param[in]  filename_ The full input filename path.
  * \param[out] prefix	The input filename path without the file extension.
  * \return The file extension string.
  */
///@TODO This method needs cleaned up signficantly. This should only take
/// about 4 lines of code if using string::find_last_of()
string get_extension(string filename_, string &prefix) {
    size_t count = 0;
    size_t last_index = 0;
    string output = "";
    prefix = "";

    if (filename_.find('.') != string::npos) {
        // Find the final period in the filename
        for (count = 0; count < filename_.size(); count++) {
            if (filename_[count] == '.') { last_index = count; }
        }

        // Get the filename prefix and the extension
        for (size_t i = 0; i < count; i++) {
            if (i < last_index) { prefix += filename_[i]; }
            else if (i > last_index) { output += filename_[i]; }
        }
    } else { // The filename has no extension.
        prefix = filename_;
    }

    return output;
}
