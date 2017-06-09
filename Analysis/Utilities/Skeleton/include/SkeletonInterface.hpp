///@file SkeletonInterface.hpp
///@brief
///@author C. R. Thornsberry, S. V. Paulauskas
///@date May 14, 2017

#ifndef PAASS_SkeletonInterface_HPP
#define PAASS_SkeletonInterface_HPP

#include <string>
#include <vector>

#include "ScanInterface.hpp"

class SkeletonInterface : public ScanInterface {
public:
    /// Default constructor.
    SkeletonInterface();

    /// Destructor.
    ~SkeletonInterface(){
        if (init) {
            // Handle some cleanup.
        }
    }

    /** ExtraCommands is used to send command strings to classes derived
      * from ScanInterface. If ScanInterface receives an unrecognized
      * command from the user, it will pass it on to the derived class.
      * \param[in]  cmd_ The command to interpret.
      * \param[out] arg_ Vector or arguments to the user command.
      * \return True if the command was recognized and false otherwise.
      */
    bool ExtraCommands(const std::string &cmd_, std::vector<std::string> &args_);

    /** ExtraArguments is used to send command line arguments to classes derived
      * from ScanInterface. This method should loop over the optionExt elements
      * in the vector userOpts and check for those options which have been flagged
      * as active by ::Setup(). This should be overloaded in the derived class.
      * \return Nothing.
      */
    void ExtraArguments();

    /** ArgHelp is used to allow a derived class to add a command line option
      * to the main list of options. This method is called at the end of
      * from the ::Setup method.
      * Does nothing useful by default.
      * \return Nothing.
      */
    void ArgHelp();

    /** SyntaxStr is used to print a linux style usage message to the screen.
      * \param[in]  name_ The name of the program.
      * \return Nothing.
      */
    void SyntaxStr(char *name_);

    /** IdleTask is called whenever a scan is running in shared
      * memory mode, and a spill has yet to be received. This method may
      * be used to update things which need to be updated every so often
      * (e.g. a root TCanvas) when working with a low data rate.
      * \return Nothing.
      */
    void IdleTask() {}

    /** Initialize the map file, the config file, the processor handler,
      * and add all of the required processors.
      * \param[in]  prefix_ String to append to the beginning of system output.
      * \return True upon successfully initializing and false otherwise.
      */
    bool Initialize(std::string prefix_ = "");

    /** Peform any last minute initialization before processing data.
      * /return Nothing.
      */
    void FinalInitialization();

    /** Initialize the root output.
      * \param[in]  fname_     Filename of the output root file.
      * \param[in]  overwrite_ Set to true if the user wishes to overwrite the output file.
      * \return True upon successfully opening the output file and false otherwise.
      */
    bool
    InitRootOutput(std::string fname_, bool overwrite_ = true) { return false; }

    /** Receive various status notifications from the scan.
      * \param[in] code_ The notification code passed from ScanInterface methods.
      * \return Nothing.
      */
    void Notify(const std::string &code_ = "");
private:
    bool init; /// Set to true when the initialization process successfully completes.
};


#endif //PAASS_SkeletonInterface_HPP
