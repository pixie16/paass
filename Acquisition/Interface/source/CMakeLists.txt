# @authors C. R. Thornsberry, K. Smith
set(Interface_SOURCES PixieInterface.cpp Lock.cpp)

add_library(PixieInterface STATIC ${Interface_SOURCES})

#Order is important, XIA before PLX
target_link_libraries(PixieInterface PaassCoreStatic ${XIA_LIBRARIES}
        ${PLX_LIBRARIES})

set(Support_SOURCES PixieSupport.cpp)
add_library(PixieSupport STATIC ${Support_SOURCES})

add_library(Utility STATIC Utility.cpp)
