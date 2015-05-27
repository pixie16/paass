#ifndef __TERM_COLORS_H_
#define __TERM_COLORS_H_

namespace TermColors {
  static const char DkRed[]     = "\e[0;31m";
  static const char DkGreen[]   = "\e[0;32m";
  static const char DkYellow[]  = "\e[0;33m";
  static const char DkBlue[]    = "\e[0;34m";
  static const char DkMagenta[] = "\e[0;35m";
  static const char DkCyan[]    = "\e[0;36m";
  static const char DkWhite[]   = "\e[0;37m";

  static const char BtRed[]     = "\e[1;31m";
  static const char BtGreen[]   = "\e[1;32m";
  static const char BtYellow[]  = "\e[1;33m";
  static const char BtBlue[]    = "\e[1;34m";
  static const char BtMagenta[] = "\e[1;35m";
  static const char BtCyan[]    = "\e[1;36m";
  static const char BtWhite[]   = "\e[1;37m";

  static const char Flashing[]  = "\e[5m";
  static const char Underline[] = "\e[4m"; 
  static const char Reset[]     = "\e[0m";
}

#endif // __TERM_COLORS_H_
