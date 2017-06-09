///@authors K. Smith, C. R. Thornsberry
#ifndef MCA_H
#define MCA_H

#include <ctime>

#include "PixieSupport.h"

class PixieInterface;

///Abstract MCA class
class MCA {
protected:
    /// Timers for the MCA object
    time_t start_time;
    time_t stop_time;

    ///Default number of bins in histogram.
    static const size_t HIS_SIZE = 16384;
    ///Default number of channels in ADC.
    static const size_t ADC_SIZE = 32768;

    ///Flag indicating if histogram construction was successful.
    bool _isOpen;
    ///Pointer to the PixieInterface
    PixieInterface *_pif;
public:
    ///Default constructor.
    MCA(PixieInterface *pif);

    ///Default destructor.
    virtual ~MCA() {};

    ///Return the length of time the MCA has been running.
    double GetRunTime();

    ///Abstract method describing how the MCA data is stored.
    virtual bool StoreData(int mod, int ch) = 0;

    ///Abstract method to open a storage file.
    virtual bool OpenFile(const char *basename) = 0;

    ///Flush the current memory to disk.
    virtual void Flush() = 0;

    ///Check if the histogram construction was successful.
    virtual bool IsOpen() { return _isOpen; };

    ///Start the MCA running.
    virtual void Run(float duration, bool *stop = NULL);

    ///Update the MCA histograms.
    virtual bool Step();
};

#endif 
