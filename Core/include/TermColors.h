///@authors D. Miller
#ifndef __TERM_COLORS_H_
#define __TERM_COLORS_H_

namespace TermColors {
    static const char DkRed[] = "\033[0;31m";
    static const char DkGreen[] = "\033[0;32m";
    static const char DkYellow[] = "\033[0;33m";
    static const char DkBlue[] = "\033[0;34m";
    static const char DkMagenta[] = "\033[0;35m";
    static const char DkCyan[] = "\033[0;36m";
    static const char DkWhite[] = "\033[0;37m";

    static const char BtRed[] = "\033[1;31m";
    static const char BtGreen[] = "\033[1;32m";
    static const char BtYellow[] = "\033[1;33m";
    static const char BtBlue[] = "\033[1;34m";
    static const char BtMagenta[] = "\033[1;35m";
    static const char BtCyan[] = "\033[1;36m";
    static const char BtWhite[] = "\033[1;37m";

    static const char Flashing[] = "\033[5m";
    static const char Underline[] = "\033[4m";
    static const char Reset[] = "\033[0m";
}

#endif // __TERM_COLORS_H_
