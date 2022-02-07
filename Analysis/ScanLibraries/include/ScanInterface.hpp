/** \file ScanInterface.hpp
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
#ifndef SCANINTERFACE_HPP
#define SCANINTERFACE_HPP

#include <deque>
#include <map>
#include <string>
#include <sstream>
#include <vector>

#include <getopt.h>

#include "hribf_buffers.h"
#include "XiaData.hpp"

#define SCAN_VERSION "1.2.29"
#define SCAN_DATE "Aug. 11th, 2016"

class Server;

class Terminal;

class Unpacker;

class optionExt {
public:
    const char *name;
    int has_arg;
    int *flag;
    int val;

    std::string argstr; /// The argument syntax for the command line option.
    std::string helpstr; /// The help & syntax string to print when -h is passed.
    std::string argument; /// The argument received from getopt_long (if available).
    bool active; /// Set to true if this option was selected by the user.

    optionExt() : has_arg(0), flag(0), val(0), active(false) {}

    optionExt(const char *name_, const int &has_arg_, int *flag_,
              const int &val_, const std::string &arg_,
              const std::string &help_);

    /// Print a help string for this option.
    void print(const size_t &len_ = 0, const std::string &prefix_ = "");

    option getOption();
};

class fileInformation {
public:
    fileInformation() {}

    ~fileInformation() {}

    size_t size() { return parnames.size(); }

    bool at(const size_t &index_, std::string &name, std::string &value);

    template<typename T>
    bool push_back(const std::string &name_, const T &value_,
                   const std::string &units_ = "");

    bool is_in(const std::string &name_);

    std::string print(const size_t &index_);

    void clear();

private:
    std::vector<std::string> parnames;

    std::vector<std::string> parvalues;
};

class ScanInterface {
public:
    /// Default constructor.
    ScanInterface();

    /// Default destructor.
    virtual ~ScanInterface();

    /// Return true if the ScanInterface object has been initialized.
    bool IsInit() { return scan_init; }

    /// Return true if verbose output mode is enabled.
    bool IsVerbose() { return is_verbose; }

    /// Return true if debug mode is enabled.
    bool DebugMode() { return debug_mode; }

    /// Return true if dry run mode is enabled.
    bool DryRunMode() { return dry_run_mode; }

    /// Return true if shared memory mode is enabled.
    bool ShmMode() { return shm_mode; }

    /// Return true if batch processing mode is enabled.
    bool BatchMode() { return batch_mode; }

    /// Return the header string used to prefix output messages.
    std::string GetMessageHeader() { return msgHeader; }

    /// Return the name of the program.
    std::string GetProgramName() { return progName; }

    /// \return The name of the configuration file
    std::string GetSetupFilename() { return (setup_filename); }

    /// \return The name of the output file that doesn't include the path
    std::string GetOutputFilename() { return outputFilename_; }

    /// @return The path for the output file
    std::string GetOutputPath() { return outputPath_; }

    /// Return a pointer to a fileInformation object used to store file header info.
    fileInformation *GetFileInfo() { return &finfo; }

    /// Set the header string used to prefix output messages.
    void SetProgramName(const std::string &head_) {
        progName = head_;
        msgHeader = head_ + ": ";
    }

    /// Enable or disable verbose output mode.
    bool SetVerboseMode(bool state_ = true) { return (is_verbose = state_); }

    /// Enable or disable debug mode.
    bool SetDebugMode(bool state_ = true) { return (debug_mode = state_); }

    /// Enable or disable dry run mode.
    bool SetDryRunMode(bool state_ = true) { return (dry_run_mode = state_); }

    /// Enable or disable shared memory mode.
    bool SetShmMode(bool state_ = true) { return (shm_mode = state_); }

    /// Enable or disable batch processing mode.
    bool SetBatchMode(bool state_ = true) { return (batch_mode = state_); }

    /// Main scan control method.
    void RunControl();

    /// Main command interpreter method.
    void CmdControl();

    /// Setup user options and initialize all required objects.
    ///\param[in] argc : Number of arguments. Comes from main()
    ///\param[in] argv : Argument list. Comes from main()
    ///\param[in] unpacker : A pointer to an unpacker object if need one.
    bool Setup(int argc, char *argv[], Unpacker *unpacker = NULL);

    /// Run the program.
    int Execute();

    /// Shutdown cleanly.
    bool Close();

protected:
    std::map<std::string, std::string> knownArgumentMap_; //!< Arguments that are known by the parent class.
    std::map<std::string, std::string> auxillaryKnownArgumentMap_; //!< Arguments that are supplied by the derived classes.

    std::string msgHeader; /// The string to print before program output.
    std::string progName; /// The name of the program.

    std::vector<option> longOpts; /// Vector of all command line options.
    std::vector<optionExt> baseOpts; /// Base level command line options for the scan.
    std::vector<optionExt> userOpts; /// User added command line options.
    std::string optstr;

    Unpacker *unpacker_; /// Pointer to class derived from Unpacker class.

    /** Add a command line option to the option list.
      * \param[in]  opt_ The option to add to the list.
      * \return Nothing.
      */
    void AddOption(optionExt opt_);

    /** ExtraCommands is used to send command strings to classes derived
      * from ScanInterface. If ScanInterface receives an unrecognized
      * command from the user, it will pass it on to the derived class.
      * Does nothing useful by default.
      * \param[in]  cmd_ The command to interpret. Not used by default.
      * \param[out] arg_ Vector or arguments to the user command. Not used by default.
      * \return True if the command was recognized and false otherwise. Returns false by default.
      */
    virtual bool ExtraCommands(const std::string &cmd_, std::vector<std::string> &args_) { return false; }

    /** ExtraArguments is used to send command line arguments to classes derived
      * from ScanInterface. This method should loop over the optionExt elements
      * in the vector userOpts and check for those options which have been flagged
      * as active by ::Setup(). This should be overloaded in the derived class.
      * \return Nothing.
      */
    virtual void ExtraArguments() {}

    /** ArgHelp is used to allow a derived class to add a command line option
      * to the main list of options. This method is called at the end of
      * from the ::Setup method.
      * Does nothing useful by default.
      * \return Nothing.
      */
    virtual void ArgHelp() {}

    /** SyntaxStr is used to print a linux style usage message to the screen.
      * Prints a standard usage message by default.
      * \param[in]  name_ The name of the program.
      * \return Nothing.
      */
    virtual void SyntaxStr(char *name_);

    /** IdleTask is called whenever a scan is running in shared
      * memory mode, and a spill has yet to be received. This method may
      * be used to update things which need to be updated every so often
      * (e.g. a root TCanvas) when working with a low data rate.
      * Does nothing useful by default.
      * \return Nothing.
      */
    virtual void IdleTask() {}

    /** Initialize the program
      * Does nothing useful by default.
      * \param[in]  prefix_ String to append to the beginning of system output.
      * \return True upon successfully initializing and false otherwise.
      */
    virtual bool Initialize(std::string prefix_ = "");

    /** Peform any last minute initialization which derived classes need
      * to run before processing data.
      * Does nothing useful by default.
      * /return Nothing.
      */
    virtual void FinalInitialization() {}

    /** Initialize the root output.
      * Does nothing useful by default.
      * \param[in]  fname_     Filename of the output root file. Not used by default.
      * \param[in]  overwrite_ Set to true if the user wishes to overwrite the output file. Not used by default.
      * \return True upon successfully opening the output file and false otherwise. Returns false by default.
      */
    virtual bool InitRootOutput(std::string fname_, bool overwrite_ = true) { return false; }

    /** Notify the unpacker object of a user action. This method should be
      * used in order to pass information to a class derived from Unpacker.
      * Does nothing useful by default.
      * \param[in] code_ The notification code passed from ScanInterface methods. Not used by default.
      * \return Nothing.
      */
    virtual void Notify(const std::string &code_ = "") {}

    ///Print a help message for the provided argument
    ///@param[in] arg : The argument that we've asked about
    ///@param[in] help : A brief message about the argument.
    void OutputCommandHelpArgument(const std::string &arg, const std::string &help);

    ///Print a help message for the provided argument
    ///@param[in] arg : The argument that we've asked about
    ///@param[in] help : A brief message about the argument.
    void OutputCommandHelpArguments(const std::map<std::string, std::string> &args);

    ///Print a message when the user asks for help with an unkonwn command.
    ///@param[in] arg : The argument that we didn't recognize
    void OutputUnkownCommandMessage(const std::string &arg);
private:
    unsigned int maxShmSizeL; /// Max size of shared memory buffer in pixie words (4050 + 2 header words)
    unsigned int maxShmSize; /// Max size of shared memory buffer in bytes

    std::string prefix; /// Input filename prefix (without extension).
    std::string extension; /// Input file extension.
    std::string workDir; /// Linux system current working directory.
    std::string homeDir; /// Linux user home directory.
    std::string setup_filename; //!< Configuration file to be opened
    std::string outputFilename_; //!< Name of file to be used for output
    std::string outputPath_;

    int max_spill_size; /// Maximum size of a spill to read.
    int file_format; /// Input file format to use (0=.ldf, 1=.pld, 2=.root, 3=.evt).

    unsigned long num_spills_recvd; /// The total number of good spills received from either the input file or shared memory.
    unsigned long file_start_offset; /// The first word in the file at which to start scanning.

    bool write_counts; /// Set to true if raw channel counts are to be written to file.

    bool total_stopped; /// Set to true if when the scan finishes.
    bool is_running; /// Set to true if the acqusition is running.
    bool is_verbose; /// Set to true if the user wishes verbose information to be displayed.
    bool debug_mode; /// Set to true if the user wishes to display debug information.
    bool dry_run_mode; /// Set to true if a dry run is to be performed i.e. data is to be read but not processed.
    bool shm_mode; /// Set to true if shared memory mode is to be used.
    bool batch_mode; /// Set to true if the program is to be run with no interactive command line.
    bool scan_init; /// Set to true when ScanInterface is initialized properly and is ready to scan.
    bool file_open; /// Set to true when an input binary file is successfully opened for reading.

    bool kill_all; /// Set to true when user has sent kill command.
    bool run_ctrl_exit; /// Set to true when run control thread has exited.

    Server *poll_server; /// Poll2 shared memory server.

    std::ifstream input_file; /// Main input binary data file.
    std::streampos file_length; /// Main input file length (in bytes).

    fileInformation finfo; /// Data structure for storing binary file header information.

    PLD_header pldHead; /// PLD style HEAD buffer handler.
    PLD_data pldData; /// PLD style DATA buffer handler.
    DIR_buffer dirbuff; /// HRIBF DIR buffer handler.
    HEAD_buffer headbuff; /// HRIBF HEAD buffer handler.
    DATA_buffer databuff; /// HRIBF DATA buffer handler.
    EOF_buffer eofbuff; /// HRIBF EOF buffer handler.

    Terminal *term; /// ncurses terminal used for displaying output and handling user input.

    /// Start the scan.
    void start_scan();

    /// Stop the scan.
    void stop_scan();

    /// Print a command line argument help dialogue.
    void OutputCommandLineHelp(char *name_);

    /// Seek to a specified position in the file.
    bool rewind(const unsigned long &offset_ = 0);

    /// Open a new binary input file for reading.
    bool open_input_file(const std::string &fname_);

    ///Sets output Filename and path that were passed using the -o flag.
    ///@param[in] a : The parameter that we are going to set
    void SetOutputInformation(const std::string &a);
};

/// Get the file extension from an input filename string.
std::string get_extension(std::string filename_, std::string &prefix);

#endif
