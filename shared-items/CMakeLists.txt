find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)
pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)
pkg_check_modules(LIBSOUP REQUIRED libsoup-2.4)




file(GLOB SOURCE_LIST 
${CMAKE_SOURCE_DIR}/shared-items/source/*.c
)

add_library(shared-items ${SOURCE_LIST})




target_include_directories(shared-items 
PUBLIC
    ${CMAKE_SOURCE_DIR}/shared-items/include
    ${LIBSOUP_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS} 
    ${GIO_INCLUDE_DIRS} 
    ${GOBJECT_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
    ${JSON_GLIB_INCLUDE_DIRS}
)


set_target_properties(shared-items PROPERTIES 
 RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

target_compile_options(shared-items PUBLIC ${GLIB_CFLAGS})
target_compile_options(shared-items PUBLIC ${LIBSOUP_CFLAGS})
target_compile_options(shared-items PUBLIC ${GIO_CFLAGS})
target_compile_options(shared-items PUBLIC ${GOBJECT_CFLAGS})
target_compile_options(shared-items PUBLIC ${GST_CFLAGS})
target_compile_options(shared-items PUBLIC ${JSON_GLIB_CFLAGS})

target_link_directories(shared-items PUBLIC ${INCLUDE_LIBRARY} )    

target_link_libraries(shared-items PUBLIC 
    ${LIBSOUP_LIBRARIES}
    ${GLIB_LIBRARIES} 
    ${GIO_LIBRARIES} 
    ${GOBJECT_LIBRARIES}
    ${GST_LIBRARIES}
    ${JSON_GLIB_LIBRARIES}
)