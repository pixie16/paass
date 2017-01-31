set(ThirdPartySources pugixml.cpp)

add_library(ThirdPartyObjects OBJECT ${ThirdPartySources})
add_library(ThirdPartyStatic STATIC $<TARGET_OBJECTS:ThirdPartyObjects>)

if(BUILD_SHARED_LIBS)
    add_library(ThirdParty SHARED $<TARGET_OBJECTS:ThirdPartyObjects>)
    install(TARGETS ThirdParty DESTINATION lib)
endif(BUILD_SHARED_LIBS)