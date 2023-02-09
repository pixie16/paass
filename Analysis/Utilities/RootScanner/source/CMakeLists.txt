# @authors K. Smith, S. V. Paulauskas

ROOT_GENERATE_DICTIONARY(HistScannerDictionary HistScannerChanData.hpp
        LINKDEF ../include/HistScannerLinkDef.h)
add_executable(rootscan HistUnpacker.cpp HistScanner.cpp HistScannerChanData.cpp
        HistScannerDictionary.cxx hist.cpp)
target_link_libraries(rootscan PaassScanStatic ResourceStatic PugixmlStatic
        PaassResourceStatic ${ROOT_LIBRARIES} -lTreePlayer)