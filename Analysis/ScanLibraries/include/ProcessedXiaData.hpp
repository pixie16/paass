///@file ProcessedXiaData.hpp
///@brief An XiaData object that has undergone additional processing.
///@author S. V. Paulauskas and C. R. Thornsberry
///@date December 2, 2016
#ifndef PIXIESUITE_PROCESSEDXIADATA_HPP
#define PIXIESUITE_PROCESSEDXIADATA_HPP
#include <memory>

#include "XiaData.hpp"

///This class contains additional information about the XiaData after
/// additional processing has been done. The processing includes, but is not
/// limited to energy/time calibrations, high resolution timing analysis,
/// trace analysis, etc. 
class ProcessedXiaData : public XiaData {
public:
    /// Default constructor.
    ProcessedXiaData() {Clear();}

    /// Constructor from XiaData. ProcessedXiaData will take ownership via
    /// the unique pointer.
    ProcessedXiaData(XiaData *event) {
        data_ = event;
    }

    /// Default Destructor.
    ~ProcessedXiaData(){};
private:
    ///A pointer to the XiaData
    XiaData *data_;

    /// Clear all variables and clear the trace vector and arrays.
    void Initialize(){};

    bool isIgnored_; /// Ignore this event.
    bool isValidData; /// True if the high resolution energy and time are valid.
    double hires_time; /// High resolution time taken from pulse fits (in ns).
};
#endif //PIXIESUITE_PROCESSEDXIADATA_HPP
