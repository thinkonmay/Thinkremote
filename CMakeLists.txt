cmake_minimum_required(VERSION 2.20.0)
project(personal-cloud-computing C)


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

    set(SHARED_HEADER 
    ${CMAKE_SOURCE_DIR}/shared-items/window
    ${CMAKE_SOURCE_DIR}/shared-items/include)

    file(GLOB SHARED_SOURCE 
    ${CMAKE_SOURCE_DIR}/shared-items/window/*.c
    ${CMAKE_SOURCE_DIR}/shared-items/source/*.c)
else()
    set(SHARED_HEADER 
    ${CMAKE_SOURCE_DIR}/shared-items/include)

    file(GLOB SHARED_SOURCE ${CMAKE_SOURCE_DIR}/shared-items/source/*.c)
endif()




set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/bin) 





add_subdirectory(worker)
add_subdirectory(client)
add_subdirectory(testing)