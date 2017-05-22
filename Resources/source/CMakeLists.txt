#Set the utility sources that we will make a lib out of
set(PaassResourceSources XmlInterface.cpp XmlParser.cpp)

if (PAASS_USE_ROOT)
    list(APPEND PaassResourceSources RootInterface.cpp)
endif (PAASS_USE_ROOT)

#Add the sources to the library
add_library(PaassResourceObjects OBJECT ${PaassResourceSources})
add_library(PaassResourceStatic STATIC $<TARGET_OBJECTS:PaassResourceObjects>)

if (BUILD_SHARED_LIBS)
    message(STATUS "Building Utility Shared Objects")
    add_library(PaassResourceLibrary SHARED
            $<TARGET_OBJECTS:PaassResourceObjects>)
    target_link_libraries(PaassResourceLibrary Pugixml)
    install(TARGETS PaassResourceLibrary DESTINATION lib)
endif (BUILD_SHARED_LIBS)