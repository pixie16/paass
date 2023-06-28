/** \file monitor.hpp
 * \brief header containing common functions for the monitor and monitor2 programs
 * \author T. T. King
 * \date Feb 14th, 2023
 */
#ifndef __MONITOR_HPP
#define __MONITOR_HPP

#include <getopt.h>
#include <signal.h>
#include <unistd.h>

#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class monitor {
   protected:
    string ObjName;
    int socketToUse = 0;                            // submonitor socket to listen on
    static const int MAX_NUM_SUBMONITORS = 6;       // max number of submonitors. Limited since each needs its own port
    static const int PREDEFINED_POLL2_PORT = 5556;  // hardcoded POLL2 master socket.
    static const size_t poll2_msg_size = 5844;      // 5.8 kB of stats data max
    const int modColumnWidth = 25;                  // width of each module section in chars

    static const int KILOBYTE = 1024;          // bytes
    static const int MEGABYTE = 1048576;       // bytes
    static const int GIGABYTE = 1073741824;    // bytes
    bool useColor = false;                     // do we colozize the output
    bool useLogging = false;                   // do we log to text file
    bool dummyMode = false;                    // are we in dummymode for debugging
    string cThreshGroup = "defaultRateGroup";  // colorize rate group

    vector<pair<int, int>> deadChan;  // list of "dead" chans for dummy mode

    int numRowsOfMods = 1;  // number of rows to put the modules on:

   public:
    // ANSI color codes
    /*
    color    foreground  background
    black        30         40
    red          31         41
    green        32         42
    yellow       33         43
    blue         34         44
    magenta      35         45
    cyan         36         46

    reset             0  (everything back to normal)
    bold/bright       1  (often a brighter shade of the same colour)
    underline         4
    inverse           7  (swap foreground and background colours)
    bold/bright off  21
    underline off    24
    inverse off      27
    */
    enum ColorCode {
        FG_DEFAULT = 0,
        FG_BLACK = 30,
        FG_RED = 31,
        FG_GREEN = 32,
        FG_YELLOW = 33,
        FG_BLUE = 34,
        FG_MAGENTA = 35,
        FG_CYAN = 36,
        BG_BLACK = 40,
        BG_RED = 41,
        BG_GREEN = 42,
        BG_YELLOW = 43,
        BG_BLUE = 44,
        BG_MAGENTA = 45,
        BG_CYAN = 46,
        BG_DEFAULT = 49,
        SP_INVERT = 7,
        SP_BOLD = 1,
        SP_UNDERLINE = 4,
        SP_UNDERLINE_OFF = 24,
        SP_BOLD_OFF = 21
    };  //

    // structure containing the parsed arrays of things out of the packet
    struct poll2_UDP_msg {
        double time_in_sec = 0;
        double data_rate = 0;
        double **ICR = nullptr;
        double **OCR = nullptr;
        double **Data = nullptr;
        unsigned **Totals = nullptr;
    };

    // struc of various color thresholds
    struct colorThresholds {
        double crit_high = 20000;
        double warn_high = 8000;
        double warn_low = 1;
        double crit_low = 0.5;
        double disabled = 0.0;
    };

    // Default Constructor
    monitor() : ObjName("defaultmonitor"){};

    // Constuctor taking exe name as argument as string
    monitor(string a) : ObjName(a){};

    // Deconstructor
    ~monitor() = default;

    // Return the order of magnitude of a number
    double GetOrder(unsigned int input_, unsigned int &power);

    // Receive and parse udp packet
    void DecodeUdpMsg(char *prt, poll2_UDP_msg &ret, int &num_modules, bool &first_packet);

    // Overload for dummy mode. we remove the char buff from the arg list
    void DecodeUdpMsg(poll2_UDP_msg &ret, int &num_modules, bool &first_packet, vector<pair<int, int>> &deadChan);

    // Expects input rate in Hz. Add suffix, and truncate to fit.
    pair<string, ColorCode> GetChanRateString(double chanRateInput, const colorThresholds &cThresh);

    // Parse out the channel total string (make sci notation and truncate)
    pair<string, ColorCode> GetChanTotalString(unsigned int input_);

    // Expects input rate in B/s
    string GetRateString(double input_, const bool &useColorOut);

    // Expects input time in seconds
    string GetTimeString(double input_);

    /* Print help dialogue for command line options. */
    void help(const char *progName_);

    // Get Maximum number of "Submonitors". This shouldnt be large as each "submonitor" requires its own udp port
    int GetMaxNumSubMonitors() { return MAX_NUM_SUBMONITORS; };

    // Get the poll2->Monitor main port, which is defined in the poll2 source
    int GetPredefinedPoll2Port() { return PREDEFINED_POLL2_PORT; };

    // Get the UDP packet size
    static constexpr size_t GetPoll2MsgSize() { return poll2_msg_size; };

    // Get Width of a module section in characters
    const int GetModColumWidth() { return modColumnWidth; };

    // Get the Number of rows to put the modules one
    const int GetNumOfModRows() { return numRowsOfMods; };

    // Parse the flags set on cli
    int ParseCliFlags(int &argc, char *argv[], monitor *obj);

    // Set bool for using colorized output. We do NO checking about color support or theme. So if you see a bunch of "/033;<>[" strings then your term does NOT support colors (this should be exceedingly rare nowadays)
    void SetColorOut(const bool &a) { useColor = a; };

    // set Color Thresh group
    virtual void SetColorThreshGroup(const string &threshGroup) { cThreshGroup = threshGroup; };

    string GetColorThreshGroup() { return cThreshGroup; };

    // Set if to write the txt log out. location of log is currently undecided. Im thinking /tmp but i dont have a good handle on what it will grow to (effectivly infinity but thats bad so we need to do something smart here)
    void SetLoggerOut(const bool &a) { useLogging = a; };

    // Get status of colorize out bool
    const bool GetColorOut() { return useColor; };

    // Get status of logger out bool
    bool GetLoggerOut() { return useLogging; };

    // Set ColorThresholds struct based on passed cli flag.
    void SetColorThresholdStruct(colorThresholds &cThresh, const string &rateGroup);

    // Get to use based on current table of thresholds.
    ColorCode GetColorFromThresholds(const double &input, const colorThresholds &cThresh);

    // For Submonitor, set socket to listen on.
    virtual void SetSocketToUse(const int &a) { socketToUse = a; };

    // For Submonitor Get socket to listen on.
    virtual const int &GetSocketToUse() { return socketToUse; };

    // Get Name of monitor derivative. I.E. mainmonitor/submonitor
    virtual const char *GetName() { return ObjName.c_str(); };

    // For debugging on non daq computers, we have a dummy mode which fills with random numbers
    void SetDummyMode(const bool &a) { dummyMode = a; };

    // Set Number of rows to print the modules on. We will automatically determine how many modules per row based on this number
    void SetNumberOfRowsForModuleList(const int &a) { numRowsOfMods = a; };

    // Get DummyMode status
    bool GetDummyMode() { return dummyMode; };

    /// Get the color code escape sequence. Colors are set by ColorCode enum.
    ///@param [in] pcode : ENUM color code from ColorCode group. Which is Defined in monitor.hpp
    ///@param [in] showcolor : Are we in colorful mode or not
    ///@return Returns either the ANSI escape string or an empty zero length string.
    static string GetEscSequence(ColorCode pcode, bool showcolor) {
        ;
        if (showcolor) {
            return "\033[" + std::to_string(pcode) + "m";
        } else {
            return "";  // This must be a zero length string to not mess with the column spacing
        }
    }

    vector<pair<int, int>> *GetDeadChanList() { return &deadChan; };
};

#endif
