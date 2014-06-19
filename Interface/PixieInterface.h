// C++ wrapper to pixie functions including error handling
//   and configuration parameters

// David Miller, Jan 2010

#ifndef __PIXIEINTERFACE_H_
#define __PIXIEINTERFACE_H_ 

#include "pixie16app_defs.h"

// define PIXIE16_REVD_GENERAL if it isn't defined in the header
//   or else it will evalute to 0 (i.e. the same as PIXIE16_REVA)
#ifndef PIXIE16_REVD_GENERAL
#define PIXIE16_REVD_GENERAL 999 // arbitrary large number
#endif

#if PIXIE16_REVISION == PIXIE16_REVF
#define PIF_REVF
#define PIF_FIFO
#elif PIXIE16_REVISION == PIXIE16_REVD_GENERAL
#define PIF_CATCHER
#define PIF_REVD
#define PIF_FIFO
#else
#define PIF_REVA
#define PIF_BUFFER
#endif

#include <fstream>
#include <map>
#include <string>
#include <set>

#include <stdint.h>

#include "Lock.h"

#ifdef PIF_CATCHER
const int CCSRA_PILEUP  = 15;
const int CCSRA_CATCHER = 16;
#endif

/// Only for SHE custom firmware w/ virtual digital-summed channels
const int CCSRA_SUMMED_SINGLE_EVENT = 18; 

class PixieInterface
{
 public:
  static const size_t STAT_SIZE  = N_DSP_PAR - DSP_IO_BORDER;
  static const size_t HISTO_SIZE = MAX_HISTOGRAM_LENGTH; 
#ifdef PIF_CATCHER
  enum CatcherModes {PC_STANDARD, PC_REJECT, PC_HYBRID, PC_ACCEPT};
#endif

  typedef uint32_t word_t;
  typedef uint16_t halfword_t;

  typedef word_t stats_t[STAT_SIZE];
  class Histogram {
  public: 
      enum ErrorTypes {NO_ERROR, ERROR_SUBTRACT, ERROR_READ, ERROR_WRITE};
      Histogram();
      Histogram(const Histogram& x);

      const Histogram& operator=(const Histogram& right);
      Histogram operator+(const Histogram& right);
      Histogram operator-(const Histogram& right);
      const Histogram& operator+=(const Histogram& right);
      const Histogram& operator-=(const Histogram& right);

      ErrorTypes GetError(void) const {return error;};
      void ClearError(void) {error = NO_ERROR;};
      bool Read(PixieInterface &pif, unsigned int mod, unsigned int ch);
      bool Write(std::ofstream &file);
  private:      
      word_t data[HISTO_SIZE];
      ErrorTypes error;            
  };

  PixieInterface(const char *fn);
  ~PixieInterface();


  bool ReadConfigurationFile(const char *fn);
  bool GetSlots(const char *slotF = NULL);
  // wrappers to the pixie-16 app functions
  bool Init(bool offlineMode = false);
  bool Boot(int mode = 0x7f, bool useWorkingSetFile = false);
  bool WriteSglModPar(const char *name, word_t val, int mod);
  bool ReadSglModPar(const char *name, word_t *pval, int mod);
  void PrintSglModPar(const char *name, int mod);
  bool WriteSglChanPar(const char *name, double val, int mod, int chan);
  bool ReadSglChanPar(const char *name, double *pval, int mod, int chan);
  void PrintSglChanPar(const char *name, int mod, int chan);
  bool SaveDSPParameters(const char *fn = NULL);
  bool AcquireTraces(int mod);
  // # AcquireTraces must be called before calling this #
  bool ReadSglChanTrace(unsigned short *buf, unsigned long sz,
			unsigned short mod, unsigned short chan);
  // # #
  bool GetStatistics(unsigned short mod);
  // # GetStatistics must be called before calling these #
  stats_t& GetStatisticsData(void) {return statistics;}
  double GetInputCountRate(int mod, int chan);
  double GetOutputCountRate(int mod, int chan);
  double GetLiveTime(int mod, int chan);
  double GetRealTime(int mod);
  double GetProcessedEvents(int mod);
  // # #
  bool StartHistogramRun(unsigned short mode = NEW_RUN);
  bool StartHistogramRun(unsigned short mod, unsigned short mode);
  bool StartListModeRun(unsigned short listMode = LIST_MODE_RUN,
			unsigned short runMode = NEW_RUN);
  bool StartListModeRun(unsigned short mod, unsigned short listMode,
			unsigned short runMode);
  bool CheckRunStatus(void); // check status in all modules
  bool CheckRunStatus(int mod);

#ifdef PIF_FIFO
  unsigned long CheckFIFOWords(unsigned short mod);
  bool ReadFIFOWords(word_t *buf, unsigned long nWords,
		     unsigned short mod);
#endif

  bool EndRun(void); // end run in all modules
  bool EndRun(int mod);
  bool RemovePresetRunLength(int mod);

  bool ReadHistogram(word_t *hist, unsigned long sz,
		     unsigned short mod, unsigned short ch);
  bool AdjustOffsets(unsigned short mod);
  // accessors
  unsigned short GetNumberCards(void) const {return numberCards;};
  static size_t GetNumberChannels(void) {return NUMBER_OF_CHANNELS;};
  static size_t GetTraceLength(void) {return TRACE_LENGTH;};

  short GetSlotNumber(int mod) const {return slotMap[mod];}
  enum BootFlags {
    BootComm = 0x01, BootTrigger = 0x02, BootSignal = 0x04,
    BootDSP = 0x08, DownloadParameters = 0x10, ProgramFPGA = 0x20,
    SetDAC = 0x40, BootAll = 0x7f
  };

  /** Handy functions for manipulating CSRA/B */
  bool ToggleGain(int mod, int chan);
  bool ToggleGood(int mod, int chan);
  bool TogglePolarity(int mod, int chan);
  bool ToggleTraceCapture(int mod, int chan);

#ifdef PIF_CATCHER
  bool ToggleCatcherBit(int mod, int chan);
  bool TogglePileupBit(int mod, int chan);

  bool SetProtonCatcherMode(int mod, int chan, CatcherModes mode);
#endif

 private:
  bool ToggleChannelBit(int mod, int chan, const char *parameter, int bit);

  static const size_t MAX_MODULES = 14;
  static const size_t CONFIG_LINE_LENGTH = 80;
  static const size_t TRACE_LENGTH = RANDOMINDICES_LENGTH;

#ifdef PIF_CATCHER
  void CatcherMessage(void);
#endif

  static std::set<std::string> validConfigKeys;
  std::map<std::string, std::string> configStrings;

  bool doneInit;

  // convert a configuration string to be relative to pixieBaseDir unless it begins with a . 
  std::string ConfigFileName(const std::string &str);
  // checks retval and outputs default OK/ERROR message
  bool CheckError(bool exitOnError = false) const;

  unsigned short numberCards;
  unsigned short slotMap[MAX_MODULES];
  unsigned short firmwareConfig[MAX_MODULES];
  bool hasAlternativeConfig;

  stats_t statistics;

  int retval; // return value from pixie functions
  Lock lock;  // class to prevent simultaneous access to pixies

  word_t extraWord[MAX_MODULES];
  bool hasExtra[MAX_MODULES];

  // temporary variables which hold the parameter which is being modified
  //   to deal with the const-incorrectness of the Pixie-16 API
  static const size_t nameSize=120;
  char tmpName[nameSize];
};

#endif // __PIXIEINTERFACE_H_
