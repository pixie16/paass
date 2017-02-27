if(PAASS_USE_ROOT)
    add_subdirectory(Scope)
endif(PAASS_USE_ROOT)

if(PAASS_BUILD_SKELETON)
    add_subdirectory(Skeleton)
endif(PAASS_BUILD_SKELETON)

add_subdirectory(HeadReader)
add_subdirectory(TraceFilterer)