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


if(MSVC)
else()
    pkg_check_modules(GTK REQUIRED gtkmm-3.0)
endif()



file(GLOB SOURCE_LIST ${CMAKE_SOURCE_DIR}/client/remote-udp/source/*.c)
add_executable(remote-udp ${SOURCE_LIST} ${SHARED_SOURCE})




target_include_directories(remote-udp 
PUBLIC
    ${CMAKE_SOURCE_DIR}/client/remote-udp/include
    ${SHARED_HEADER}
    ${LIBSOUP_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS} 
    ${GIO_INCLUDE_DIRS} 
    ${GOBJECT_INCLUDE_DIRS}
    ${SDP_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
    ${JSON_GLIB_INCLUDE_DIRS}
    ${WEBRTC_INCLUDE_DIRS}
    ${GTK_INCLUDE_DIRS}
    ${CONTROLLER_INCLUDE_DIRS}
    ${VIDEO_INCLUDE_DIRS})


set_target_properties(remote-udp PROPERTIES 
 RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

target_compile_options(remote-udp PUBLIC ${GLIB_CFLAGS})
target_compile_options(remote-udp PUBLIC ${GIO_CFLAGS})
target_compile_options(remote-udp PUBLIC ${GOBJECT_CFLAGS})
target_compile_options(remote-udp PUBLIC ${SDP_CFLAGS})
target_compile_options(remote-udp PUBLIC ${GST_CFLAGS})
target_compile_options(remote-udp PUBLIC ${JSON_GLIB_CFLAGS})
target_compile_options(remote-udp PUBLIC ${WEBRTC_CFLAGS})
target_compile_options(remote-udp PUBLIC ${CONTROLLER_CFLAGS})
target_compile_options(remote-udp PUBLIC ${VIDEO_CFLAGS})
target_compile_options(remote-udp PUBLIC ${GTK_CFLAGS})

target_link_directories(remote-udp PUBLIC ${INCLUDE_LIBRARY} )    

target_link_libraries(remote-udp 
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
    ${GTK_LIBRARIES}
)



