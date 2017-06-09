#authors K. Smith, C. R. Thornsberry
#Build and install setup utilities, and configuration file
set(SETUP_UTILS adjust_offsets find_tau copy_params pread pwrite pmread pmwrite
        rate boot trace get_traces csr_test toggle set_standard set_pileups_only
        set_pileups_reject set_hybrid)

foreach (UTIL ${SETUP_UTILS})
    add_executable(${UTIL} ${UTIL}.cpp)
    target_link_libraries(${UTIL} PixieSupport PixieInterface) # The order matters here! CRT
endforeach (UTIL)

install(TARGETS ${SETUP_UTILS} DESTINATION bin)

if (PAASS_USE_ROOT)
    add_executable(paramScan paramScan.cpp)
    target_link_libraries(paramScan PixieInterface MCA_LIBRARY ${ROOT_LIBRARIES}
            "-lSpectrum")
    install(TARGETS paramScan DESTINATION bin)
endif (PAASS_USE_ROOT)
