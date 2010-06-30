#ifndef __TERM_COLORS_H_
#define __TERM_COLORS_H_

namespace TermColors {
  const char *DkRed     = "\e[0;31m";
  const char *DkGreen   = "\e[0;32m";
  const char *DkYellow  = "\e[0;33m";
  const char *DkBlue    = "\e[0;34m";
  const char *DkMagenta = "\e[0;35m";
  const char *DkCyan    = "\e[0;36m";
  const char *DkWhite   = "\e[0;37m";

  const char *BtRed     = "\e[1;31m";
  const char *BtGreen   = "\e[1;32m";
  const char *BtYellow  = "\e[1;33m";
  const char *BtBlue    = "\e[1;34m";
  const char *BtMagenta = "\e[1;35m";
  const char *BtCyan    = "\e[1;36m";
  const char *BtWhite   = "\e[1;37m";

  const char *Flashing  = "\e[5m";
  const char *Underline = "\e[4m"; 
  const char *Reset     = "\e[0m";
}

#endif // __TERM_COLORS_H_
