#We need fPIC below to create shared libraries.
set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -fsecond-underscore -fPIC -Wl,-no_compact_unwinding")

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

