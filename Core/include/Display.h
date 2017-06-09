// A few things to make the output more pleasing to look at
///@authors D. Miller, C. Thornsberry, K. Smith


#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include <string>

#include "TermColors.h"

namespace Display {
    void SetColorTerm(void);

    std::string CriticalStr(const std::string &str);

    std::string ErrorStr(const std::string &str = "[ERROR]");

    std::string InfoStr(const std::string &str = "[INFO]");

    std::string OkayStr(const std::string &str = "[OK]");

    std::string WarningStr(const std::string &str = "[WARNING]");

    void LeaderPrint(const std::string &str);

    bool StatusPrint(bool errorStatus);

    extern bool hasColorTerm;
}

#endif // __DISPLAY_H_
