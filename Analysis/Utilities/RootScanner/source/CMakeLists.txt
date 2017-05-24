root_generate_dictionary(HistScannerDictionary HistScannerChanData.hpp
        LINKDEF HistScannerLinkDef.h)
add_executable(rootscan HistUnpacker.cpp HistScanner.cpp HistScannerChanData.cpp
        HistScannerDictionary.cxx hist.cpp)
target_link_libraries(rootscan PaassScanStatic ResourceStatic
        PaassResourceStatic ${ROOT_LIBRARIES} -lTreePlayer)