///@author S. V. Paulauskas
#ifndef __UTKSCANINTERFACE_HPP__
#define __UTKSCANINTERFACE_HPP__

#include <deque>
#include <string>

#include <ScanInterface.hpp>
#include <XiaData.hpp>

///Class derived from ScanInterface to handle UI for the scan.
class UtkScanInterface : public ScanInterface {
public:
    /// Default constructor.
    UtkScanInterface();

    /// Destructor.
    ~UtkScanInterface();

    /** Initialize the map file, the config file, the processor handler, 
     * and add all of the required processors.
     * \param[in]  prefix_ String to append to the beginning of system output.
     * \return True upon successfully initializing and false otherwise. */
    bool Initialize(std::string prefix_ = "");
private:
    bool init_; /// Set to true when the initialization process successfully completes.
    std::string outputFname_; /// The output histogram filename prefix.
};

#endif //__UTK_SCAN_INTERFACE_HPP__
