/// @file XiaListModeDataEncoder.hpp
/// @brief Class that handles encoding Pixie-16 list mode data from a XiaData
/// class
/// @author S. V. Paulauskas
/// @date December 30, 2016
#ifndef PIXIESUITE_XIALISTMODEDATAENCODER_HPP
#define PIXIESUITE_XIALISTMODEDATAENCODER_HPP

#include <vector>

#include "XiaData.hpp"
//For the FIRMWARE enum
#include "XiaListModeDataMask.hpp"

class XiaListModeDataEncoder {
public:
    ///Default constructor
    XiaListModeDataEncoder() {};

    ///Default destructor
    ~XiaListModeDataEncoder() {};

    ///Method that will create a Pixie List Mode Data Event from an XiaData
    /// object.
    ///@param[in] data : The data that we want to encode
    ///@param[in] firmware : The firmware version to encode into
    ///@param[in] frequency : The sampling frequency in MHz or MS/s
    ///@return A vector containing the encoded data.
    std::vector<unsigned int> EncodeXiaData(const XiaData &data,
                                            const DataProcessing::FIRMWARE &firmware,
                                            const unsigned int &frequency);

private:
    ///Encodes the first word of the data buffer.
    ///@param[in] data : The data to encode
    ///@param[in] mask : The object with the data masks
    ///@return The encoded data word.
    unsigned int EncodeWordZero(const XiaData &data,
                                const XiaListModeDataMask &mask);

    ///Encodes the second word of the data buffer.
    ///@param[in] data : The data to encode
    ///@param[in] mask : The object with the data masks
    ///@return The encoded data word.
    unsigned int EncodeWordOne(const XiaData &data,
                               const XiaListModeDataMask &mask);

    ///Encodes the third word of the data buffer.
    ///@param[in] data : The data to encode
    ///@param[in] mask : The object with the data masks
    ///@return The encoded data word.
    unsigned int EncodeWordTwo(const XiaData &data,
                               const XiaListModeDataMask &mask);

    ///Encodes the fourth word of the data buffer.
    ///@param[in] data : The data to encode
    ///@param[in] mask : The object with the data masks
    ///@return The encoded data word.
    unsigned int EncodeWordThree(const XiaData &data,
                                 const XiaListModeDataMask &mask);

    ///Encodes the Trace
    ///@param[in] data : The data to encode
    ///@param[in] mask : The object with the data masks
    ///@return The encoded data word.
    std::vector<unsigned int> EncodeTrace(const std::vector<unsigned int> &trc,
                                          const std::pair<unsigned int,
                                                  unsigned int> &mask);

    ///Encodes the Esums
    ///@param[in] data : The data to encode
    ///@param[in] mask : The object with the data masks
    ///@return The encoded data word.
    std::vector<unsigned int> EncodeEsums(const XiaData &data,
                                          const XiaListModeDataMask &mask);
};

#endif //PIXIESUITE_XIALISTMODEDATAENCODER_HPP
