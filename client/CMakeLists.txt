cmake_minimum_required(VERSION 2.20.0)
project(Slave C)


# add_compile_options (-fdiagnostics-color=always)
# add_compile_options (warning_level=1)

if (MINGW)
    set(CMAKE_PREFIX_PATH C:/gstreamer/1.0/mingw_x86)
    set(INCLUDE_LIBRARY C:/gstreamer/1.0/mingw_x86/lib)

    foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
        string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
        set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin )
        set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin )
        set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin )
    endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
elseif(MSVC)
    set(CMAKE_PREFIX_PATH C:/gstreamer/1.0/msvc_x86_64)
    set(INCLUDE_LIBRARY C:/gstreamer/1.0/msvc_x86_64/lib)
    
    foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
        string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
        set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin )
        set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin )
        set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin )
    endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
endif()




set(SHARED_ITEMS ${CMAKE_SOURCE_DIR}/shared-items/include)

add_subdirectory(shared-items)
add_subdirectory(window)
add_subdirectory(test)