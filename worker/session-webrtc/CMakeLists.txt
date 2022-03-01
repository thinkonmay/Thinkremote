find_package(PkgConfig REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIO REQUIRED gio-2.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)
pkg_check_modules(SDP REQUIRED gstreamer-sdp-1.0)
pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)
pkg_check_modules(LIBSOUP REQUIRED libsoup-2.4)
pkg_check_modules(WEBRTC REQUIRED gstreamer-webrtc-1.0)
pkg_check_modules(CONTROLLER REQUIRED gstreamer-controller-1.0)
pkg_check_modules(VIDEO REQUIRED gstreamer-video-1.0)





file(GLOB SOURCE_LIST 
${CMAKE_SOURCE_DIR}/worker/session-webrtc/source/*.c
${SHARED_SOURCE}
)

add_executable(session-webrtc ${SOURCE_LIST})




target_include_directories(session-webrtc PUBLIC
    ${CMAKE_SOURCE_DIR}/worker/session-webrtc/include
    ${SHARED_HEADER}
    ${LIBSOUP_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS} 
    ${GIO_INCLUDE_DIRS} 
    ${GOBJECT_INCLUDE_DIRS}
    ${SDP_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
    ${JSON_GLIB_INCLUDE_DIRS}
    ${WEBRTC_INCLUDE_DIRS}
    ${CONTROLLER_INCLUDE_DIRS}
    ${VIDEO_INCLUDE_DIRS}
)


set_target_properties(session-webrtc PROPERTIES 
 RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

target_compile_options(session-webrtc PUBLIC ${GLIB_CFLAGS})
target_compile_options(session-webrtc PUBLIC ${GIO_CFLAGS})
target_compile_options(session-webrtc PUBLIC ${GOBJECT_CFLAGS})
target_compile_options(session-webrtc PUBLIC ${SDP_CFLAGS})
target_compile_options(session-webrtc PUBLIC ${GST_CFLAGS})
target_compile_options(session-webrtc PUBLIC ${JSON_GLIB_CFLAGS})
target_compile_options(session-webrtc PUBLIC ${WEBRTC_CFLAGS})
target_compile_options(session-webrtc PUBLIC ${VIDEO_CFLAGS})
target_compile_options(session-webrtc PUBLIC ${CONTROLLER_CFLAGS})

target_link_directories(session-webrtc PUBLIC ${INCLUDE_LIBRARY} )    

target_link_libraries(session-webrtc PUBLIC 
    ${LIBSOUP_LIBRARIES}
    ${GLIB_LIBRARIES} 
    ${GIO_LIBRARIES} 
    ${GOBJECT_LIBRARIES}
    ${SDP_LIBRARIES}
    ${GST_LIBRARIES}
    ${JSON_GLIB_LIBRARIES}
    ${WEBRTC_LIBRARIES}
    ${CONTROLLER_LIBRARIES}
    ${VIDEO_LIBRARIES}
)



if(MSVC)
else()
    pkg_check_modules(X11 REQUIRED x11)

    target_link_libraries(session-webrtc PUBLIC  ${X11_LIBRARIES})
    target_compile_options(session-webrtc PUBLIC ${X11_CFLAGS})

    # this is due to the missing of include path of x11 module in pkg-config and should not consider as a good example
    target_include_directories(session-webrtc PUBLIC "/usr/include/X11" ) 
endif()


