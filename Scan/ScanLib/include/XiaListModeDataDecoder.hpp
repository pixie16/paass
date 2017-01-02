/// @file XiaListModeDataDecoder.hpp
/// @brief Class that handles decoding list mode data from XIA Pixie-16
/// modules.
/// @author S. V. Paulauskas
/// @date December 23, 2016
#ifndef PIXIESUITE_XIALISTMODEDATADECODER_HPP
#define PIXIESUITE_XIALISTMODEDATADECODER_HPP

#include <vector>

#include "XiaData.hpp"
#include "XiaListModeDataMask.hpp"

///Class to decode Xia List mode Data
class XiaListModeDataDecoder {
public:
    ///Default constructor
    XiaListModeDataDecoder() {};

    ///Default destructor
    ~XiaListModeDataDecoder() {};

    ///Main decoding method
    ///@param[in] buf : Pointer to the beginning of the data buffer.
    ///@param[in] mask : The mask set that we need to decode the data
    ///@return A vector containing all of the decoded XiaData events.
    std::vector<XiaData*> DecodeBuffer(unsigned int *buf,
                                       const XiaListModeDataMask &mask);

private:
    ///Method to decode word zero from the header.
    ///@param[in] word : The word that we need to decode
    ///@param[in] data : The XiaData object that we are going to fill.
    ///@return The pair of the header length and event length for use in
    /// subsequent processing.
    std::pair<unsigned int, unsigned int> DecodeWordZero(
            const unsigned int &word, XiaData &data,
            const XiaListModeDataMask &mask);

    ///Method to decode word two from the header.
    ///@param[in] word : The word that we need to decode
    ///@param[in] data : The XiaData object that we are going to fill.
    ///@param[in] mask : The data mask to decode the data
    void DecodeWordTwo(const unsigned int &word, XiaData &data,
                       const XiaListModeDataMask &mask);

    ///Method to decode word three from the header.
    ///@param[in] word : The word that we need to decode
    ///@param[in] data : The XiaData object that we are going to fill.
    ///@param[in] mask : The data mask to decode the data
    ///@return The trace length
    unsigned int DecodeWordThree(const unsigned int &word, XiaData &data,
                                 const XiaListModeDataMask &mask);

    ///Method to decode word three from the header.
    ///@param[in] word : The word that we need to decode
    ///@param[in] data : The XiaData object that we are going to fill.
    void DecodeTrace(unsigned int *buf, XiaData &data,
                     const unsigned int &traceLength);

    ///Method to calculate the arrival time of the signal in samples
    ///@param[in] mask : The data mask containing the necessary information
    /// to calculate the time.
    ///@param[in] data : The data that we will use to calculate the time
    ///@return The calculated time in clock samples
    ///@TODO This method needs to be moved to Resources so that it
    /// can be taken advantage of by other classes.
    double CalculateTimeInSamples(const XiaListModeDataMask &mask,
                                  const XiaData &data);

    ///Method to calculate the arrival time of the signal in nanoseconds
    ///@param[in] mask : The data mask containing the necessary information
    /// to calculate the time.
    ///@param[in] data : The data that we will use to calculate the time
    ///@return The calculated time in nanoseconds
    ///@TODO This method needs to be moved to Resources so that it
    /// can be taken advantage of by other classes.
    double CalculateTimeInNs(const XiaListModeDataMask &mask,
                             const XiaData &data);
};

#endif //PIXIESUITE_XIALISTMODEDATADECODER_HPP
