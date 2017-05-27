#ifndef __UTKSCANINTERFACE_HPP__
#define __UTKSCANINTERFACE_HPP__

#include <deque>
#include <string>

#include <ScanInterface.hpp>
#include <XiaData.hpp>

///Class derived from ScanInterface to handle UI for the scan.
class UtkScanInterface : public ScanInterface {
public:
    /// Default constructor.
    UtkScanInterface();

    /// Destructor.
    ~UtkScanInterface();

    /** ExtraCommands is used to send command strings to classes derived
     * from ScanInterface. If ScanInterface receives an unrecognized
     * command from the user, it will pass it on to the derived class.
     * \param[in]  cmd_ The command to interpret.
     * \param[out] arg_ Vector or arguments to the user command.
     * \return True if the command was recognized and false otherwise. */
    bool ExtraCommands(const std::string &cmd_,
                               std::vector<std::string> &args_);

    /** ExtraArguments is used to send command line arguments to classes derived
     * from ScanInterface. It has its own instance of getopts to look for its known 
     * parameters. 
     * \param[in] argc : The number of command line arguments
     * \param[in] argv[] : The arrary containing all command line arguments */
    void ExtraArguments(int argc, char *argv[]) {}

    /** Initialize the map file, the config file, the processor handler, 
     * and add all of the required processors.
     * \param[in]  prefix_ String to append to the beginning of system output.
     * \return True upon successfully initializing and false otherwise. */
    bool Initialize(std::string prefix_ = "");

    /** Initialize the root output. 
     * \param[in]  fname_     Filename of the output root file. 
     * \param[in]  overwrite_ Set to true to overwrite the output file. 
     * \return True upon successfully opening the output file, false otherwise. */
    bool InitRootOutput(std::string fname_, bool overwrite_ = true) { return (false); }

    /** ArgHelp is used to allow a derived class to print a help statment about
     * its own command line arguments. This method is called at the end of
     * the ScanInterface::help method.
     * \return Nothing. */
    void ArgHelp(void) {}

    /** SyntaxStr is used to print a linux style usage message to the screen.
     * \param[in]  name_ The name of the program.
     * \return Nothing. */
    void SyntaxStr(char *name_);

    /** IdleTask is called whenever a scan is running in shared
     * memory mode, and a spill has yet to be received. This method may
     * be used to update things which need to be updated every so often
     * (e.g. a root TCanvas) when working with a low data rate. 
     * \return Nothing. */
    void IdleTask(void) {}

    /** Peform any last minute initialization before processing data. 
     * /return Nothing. */
    void FinalInitialization();

    /** Receive various status notifications from the scan.
     * \param[in] code_ The notification code passed from ScanInterface methods.
     * \return Nothing. */
    void Notify(const std::string &code_ = "");

private:
    bool init_; /// Set to true when the initialization process successfully completes.
    std::string outputFname_; /// The output histogram filename prefix.
};

#endif //__UTK_SCAN_INTERFACE_HPP__
