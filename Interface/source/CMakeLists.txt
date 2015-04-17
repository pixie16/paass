set(Interface_SOURCES PixieInterface.cpp Lock.cpp Display.cpp)

add_library(PixieInterface STATIC ${Interface_SOURCES})

#Order is important, PXI before PLX
target_link_libraries(PixieInterface ${PXI_LIBRARIES} ${PLX_LIBRARIES})

add_library(PixieSupport STATIC PixieSupport.cpp)

add_library(Utility STATIC Utility.cpp)
