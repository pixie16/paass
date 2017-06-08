///@file EventReaderInterface.hpp
///@brief Derived class to handle the user interface for EventReader
///@authors S. V. Paulauskas and C. R. Thornsberry
///@date June 8, 2017
#ifndef PAASS_EVENTREADERINTERFACE_HPP
#define PAASS_EVENTREADERINTERFACE_HPP

#include <string>
#include <vector>

#include "EventReaderUnpacker.hpp"
#include "ScanInterface.hpp"

class EventReaderInterface : public ScanInterface {
public:
    /// Default constructor.
    EventReaderInterface(EventReaderUnpacker *unpacker) : ScanInterface(), init(false) { }

    /// Destructor.
    ~EventReaderInterface() { };

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

    /** CmdHelp is used to allow a derived class to print a help statement about
      * its own commands. This method is called whenever the user enters 'help'
      * or 'h' into the interactive terminal (if available).
      * \param[in]  prefix_ String to append at the start of any output. Not used by default.
      * \return Nothing.
      */
    void CmdHelp(const std::string &prefix_="");

    /** ArgHelp is used to allow a derived class to add a command line option
      * to the main list of options. This method is called at the end of
      * from the ::Setup method.
      * Does nothing useful by default.
      * \return Nothing.
      */
    void ArgHelp();

    /** Receive various status notifications from the scan.
      * \param[in] code_ The notification code passed from ScanInterface methods.
      * \return Nothing.
      */
    void Notify(const std::string &code_="");

private:
    EventReaderUnpacker *unpacker_;

    bool init; /// Set to true when the initialization process successfully completes.
};


#endif //PAASS_EVENTREADERINTERFACE_HPP
