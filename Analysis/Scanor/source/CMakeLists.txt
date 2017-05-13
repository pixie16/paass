#We need fPIC below to create shared libraries.
set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fsecond-underscore -fPIC")

if (${CMAKE_Fortran_COMPILER} MATCHES gfortran)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D LINK_GFORTRAN")
endif (${CMAKE_Fortran_COMPILER} MATCHES gfortran)

set(SCANOR_SOURCES
        set2cc.f
        messlog.f
        mildatim.f
        scanor.f
        ScanorInterface.cpp
        Scanor.cpp
        GetArguments.cpp
        )

add_library(ScanorObjects OBJECT ${SCANOR_SOURCES})

if (PAASS_BUILD_SHARED_LIBS)
    add_library(ScanHHIRF SHARED $<TARGET_OBJECTS:ScanorObjects>)
    install(TARGETS ScanHHIRF DESTINATION lib)
endif (PAASS_BUILD_SHARED_LIBS)
