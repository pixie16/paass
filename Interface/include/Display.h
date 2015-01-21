// A few things to make the output more pleasing to look at

#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include <string>

namespace Display {
  void SetColorTerm(void);

  std::string CriticalStr(const std::string &str);
  std::string ErrorStr(const std::string &str = "[ERROR]");
  std::string InfoStr(const std::string &str);
  std::string OkayStr(const std::string &str = "[OK]");
  std::string WarningStr(const std::string &str);

  void LeaderPrint(const std::string &str);
  bool StatusPrint(bool errorStatus);

  extern bool hasColorTerm;
};

#endif // __DISPLAY_H_
