///@authors D. Miller, K. Smith, C. R. Thornsberry
#include <iomanip>
#include <iostream>

#include <cstdlib>

#include "Display.h"

using namespace std;

using namespace TermColors;

static const size_t leaderLength = 50;

bool Display::hasColorTerm = true;

void Display::SetColorTerm() {
#ifdef NOCOLORINTERFACE
    hasColorTerm = false;
#else
    string termname = getenv("TERM");

    if (termname == "xterm")
        hasColorTerm = true;
#endif
}

void Display::LeaderPrint(const string &str) {
    char ch = cout.fill();

    cout.fill('.');
    cout << setw(leaderLength) << left << str << flush;
    cout.fill(ch);
}

string Display::CriticalStr(const string &str) {
    if (hasColorTerm) {
        return (string(BtRed) + Flashing + str + Reset);
    } else {
        return (string("###") + str + "###");
    }
}

string Display::ErrorStr(const string &str) {
    if (hasColorTerm) {
        return (DkRed + str + Reset);
    } else {
        return str;
    }
}

string Display::InfoStr(const string &str) {
    if (hasColorTerm) {
        return (DkBlue + str + Reset);
    } else {
        return str;
    }
}

string Display::OkayStr(const string &str) {
    if (hasColorTerm) {
        return (DkGreen + str + Reset);
    } else {
        return str;
    }
}

string Display::WarningStr(const string &str) {
    if (hasColorTerm) {
        return (DkYellow + str + Reset);
    } else {
        return str;
    }
}

bool Display::StatusPrint(bool errorStatus) {
    if (errorStatus)
        cout << Display::ErrorStr() << endl;
    else
        cout << Display::OkayStr() << endl;
    return errorStatus;
}
