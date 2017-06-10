///@file ScopeScanner.hpp
///@brief
///@author C. R. Thornsberry, S. V. Paulauskas, and K. Smith
///@date May 19, 2017
#ifndef PIXIESUITE_SCOPESCANNER_HPP
#define PIXIESUITE_SCOPESCANNER_HPP

#include <ctime>
#include <vector>
#include <deque>
#include <cmath>
#include <string>

#include "ScanInterface.hpp"
#include "ScopeUnpacker.hpp"

class ScopeScanner : public ScanInterface {
public:
    /// Default constructor.
    ScopeScanner(ScopeUnpacker *unpacker);

    /// Destructor.
    ~ScopeScanner(){};

    /// Stop the run.
    void StopACQ() { running = false; }

    /// Star the run.
    void StartACQ() { running = true; }

    /** ExtraCommands is used to send command strings to classes derived
      * from ScanInterface. If ScanInterface receives an unrecognized
      * command from the user, it will pass it on to the derived class.
      * \param[in]  cmd_ The command to interpret.
      * \param[out] arg_ Vector or arguments to the user command.
      * \return True if the command was recognized and false otherwise.
      */
    virtual bool ExtraCommands(const std::string &cmd_, std::vector<std::string> &args_);

    /** ExtraArguments is used to send command line arguments to classes derived
      * from ScanInterface. This method should loop over the optionExt elements
      * in the vector userOpts and check for those options which have been flagged
      * as active by ::Setup(). This should be overloaded in the derived class.
      * \return Nothing.
      */
    virtual void ExtraArguments();

    /** ArgHelp is used to allow a derived class to add a command line option
      * to the main list of options. This method is called at the end of
      * from the ::Setup method.
      * Does nothing useful by default.
      * \return Nothing.
      */
    virtual void ArgHelp();

    /** SyntaxStr is used to print a linux style usage message to the screen.
      * \param[in]  name_ The name of the program.
      * \return Nothing.
      */
    virtual void SyntaxStr(char *name_);

    /** Initialize the map file, the config file, the processor handler,
      * and add all of the required processors.
      * \param[in]  prefix_ String to append to the beginning of system output.
      * \return True upon successfully initializing and false otherwise.
      */
    virtual bool Initialize(std::string prefix_ = "");

    /** Initialize the root output.
      * \param[in]  fname_     Filename of the output root file.
      * \param[in]  overwrite_ Set to true if the user wishes to overwrite the output file.
      * \return True upon successfully opening the output file and false otherwise.
      */
    virtual bool
    InitRootOutput(std::string fname_, bool overwrite_ = true) { return false; }

    /** Receive various status notifications from the scan.
      * \param[in] code_ The notification code passed from ScanInterface methods.
      * \return Nothing.
      */
    virtual void Notify(const std::string &code_ = "");

private:
    ScopeUnpacker *unpacker_;
    unsigned int numAvgWaveforms_;
    unsigned int num_displayed; ///< The number of displayed traces.

    bool need_graph_update; /// Set to true if the graph range needs updated.
    bool acqRun_;
    bool singleCapture_;
    bool init;
    bool running;
    bool performFit_;
    bool performCfd_;

    time_t last_trace; ///< The time of the last trace.

    std::string saveFile_; ///< The name of the file to save a trace.
};
#endif //PIXIESUITE_SCOPESCANNER_HPP
