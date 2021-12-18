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
${CMAKE_SOURCE_DIR}/window/remote-app/source/*.c
${CMAKE_SOURCE_DIR}/shared-items/source/*.c
)


add_executable(remote-app ${SOURCE_LIST})




target_include_directories(remote-app 
PUBLIC
    ${CMAKE_SOURCE_DIR}/window/remote-app/include
    ${CMAKE_SOURCE_DIR}/shared-items/include
    ${LIBSOUP_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS} 
    ${GIO_INCLUDE_DIRS} 
    ${GOBJECT_INCLUDE_DIRS}
    ${SDP_INCLUDE_DIRS}
    ${GST_INCLUDE_DIRS}
    ${JSON_GLIB_INCLUDE_DIRS}
    ${WEBRTC_INCLUDE_DIRS}
    ${CONTROLLER_INCLUDE_DIRS}
    ${VIDEO_INCLUDE_DIRS})


set_target_properties(remote-app PROPERTIES 
 RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

target_compile_options(remote-app PUBLIC ${GLIB_CFLAGS})
target_compile_options(remote-app PUBLIC ${GIO_CFLAGS})
target_compile_options(remote-app PUBLIC ${GOBJECT_CFLAGS})
target_compile_options(remote-app PUBLIC ${SDP_CFLAGS})
target_compile_options(remote-app PUBLIC ${GST_CFLAGS})
target_compile_options(remote-app PUBLIC ${JSON_GLIB_CFLAGS})
target_compile_options(remote-app PUBLIC ${WEBRTC_CFLAGS})
target_compile_options(remote-app PUBLIC ${CONTROLLER_CFLAGS})
target_compile_options(remote-app PUBLIC ${VIDEO_CFLAGS})

target_link_directories(remote-app PUBLIC ${INCLUDE_LIBRARY} )    

target_link_libraries(remote-app 
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



