find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)
pkg_check_modules(LIBSOUP REQUIRED libsoup-2.4)


file(GLOB SOURCE 
${CMAKE_SOURCE_DIR}/worker/agent/source/*.c 
${SHARED_SOURCE}
)

list(FILTER SOURCE EXCLUDE REGEX ".capture-key.c")
list(FILTER SOURCE EXCLUDE REGEX ".handle-key.c")
list(FILTER SOURCE EXCLUDE REGEX ".overlay-gui.c")

add_executable(agent ${SOURCE})


set_target_properties(agent PROPERTIES 
 RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")



target_include_directories(agent 
PUBLIC
    ${CMAKE_SOURCE_DIR}/worker/agent/include
    ${SHARED_HEADER}
    ${LIBSOUP_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS} 
    ${GIO_INCLUDE_DIRS} 
    ${GOBJECT_INCLUDE_DIRS}
    ${JSON_GLIB_INCLUDE_DIRS}
    ${CONTROLLER_INCLUDE_DIRS})

target_compile_options(agent PUBLIC ${GLIB_CFLAGS})
target_compile_options(agent PUBLIC ${GIO_CFLAGS})
target_compile_options(agent PUBLIC ${GOBJECT_CFLAGS})
target_compile_options(agent PUBLIC ${JSON_GLIB_CFLAGS})
target_compile_options(agent PUBLIC ${CONTROLLER_CFLAGS})


target_link_directories(agent PUBLIC ${INCLUDE_LIBRARY} )  

target_link_libraries(agent PUBLIC 
    ${LIBSOUP_LIBRARIES}
    ${GLIB_LIBRARIES} 
    ${GIO_LIBRARIES} 
    ${GOBJECT_LIBRARIES}
    ${SDP_LIBRARIES}
    ${GST_LIBRARIES}
    ${JSON_GLIB_LIBRARIES}
    ${WEBRTC_LIBRARIES}
    ${CONTROLLER_LIBRARIES}
)





