/// @authors K. Smith, S. V. Paulauskas

#ifndef HISTSCANNER_H
#define HISTSCANNER_H

#include "ScanInterface.hpp"
#include "HistUnpacker.hpp"

class HistScanner : public ScanInterface {
public:
    /// @brief Default constructor.
    HistScanner(HistUnpacker *unpacker);

    /// @brief Default destructor.
    ~HistScanner() {};

    /** ExtraCommands is used to send command strings to classes derived
     * from ScanInterface. If ScanInterface receives an unrecognized
     * command from the user, it will pass it on to the derived class.
     * \param[in]  cmd_ The command to interpret.
     * \param[out] arg_ Vector or arguments to the user command.
     * \return True if the command was recognized and false otherwise.
     */
    bool ExtraCommands(const std::string &cmd_, std::vector<std::string> &args_);

    /** Receive various status notifications from the scan.
     * \param[in] code_ The notification code passed from ScanInterface methods.
     * \return Nothing.
     */
    void Notify(const std::string &code_ = "");

private:
    HistUnpacker *unpacker_;
};

#endif //HISTSCANNER_H
