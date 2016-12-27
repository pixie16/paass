/// @file XiaListModeDataDecoder.hpp
/// @brief Class that handles decoding list mode data from XIA Pixie-16
/// modules.
/// @author S. V. Paulauskas
/// @date December 23, 2016
#ifndef PIXIESUITE_XIALISTMODEDATADECODER_HPP
#define PIXIESUITE_XIALISTMODEDATADECODER_HPP
#include <vector>

#include "XiaData.hpp"

class XiaListModeDataDecoder {
public:
    XiaListModeDataDecoder(){};
    ~XiaListModeDataDecoder(){};

    std::vector<XiaData> DecodeBuffer(unsigned int *buf);
private:

};

#endif //PIXIESUITE_XIALISTMODEDATADECODER_HPP
