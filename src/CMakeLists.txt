PROJECT(voipc)

IF(WIN32)
    set(CMAKE_PREFIX_PATH "C:\\Qt\\5.5\\mingw492_32\\")
ENDIF()

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

SET(VOIPCSources ${VOIPCSources}
  main.cpp
  application.cpp
  mainwindow.cpp
  settings.cpp
  voipc.cpp
  PjCallback.cpp
  network/server.cpp
  network/clientthread.cpp
  network/package.cpp
  network/serverpackagemanager.cpp
)

SET(VOIPCUI ${VOIPCUI}
  mainwindow.ui
)

INCLUDE_DIRECTORIES(
  .
  ${CMAKE_CURRENT_BINARY_DIR}
#  utils
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

qt5_wrap_ui(voipcUI_H ${VOIPCUI})
#qt5_add_resources(RC_SRCS "../resources.qrc")

SET(final_src ${final_src} ${VOIPCUI} ${VOIPCSources} ${RC_SRCS})

IF(WIN32)
  ADD_EXECUTABLE(voipc WIN32 ${final_src})
ELSE()
  ADD_EXECUTABLE(voipc ${final_src})
ENDIF()

SET_TARGET_PROPERTIES(voipc PROPERTIES AUTOMOC TRUE)

TARGET_LINK_LIBRARIES(voipc Qt5::Widgets)
TARGET_LINK_LIBRARIES(voipc Qt5::Network)
if(WIN32)
    target_link_libraries(voipc
        pjsua-i686-pc-mingw32
        pjsip-ua-i686-pc-mingw32
        pjsip-simple-i686-pc-mingw32
        pjsip-i686-pc-mingw32
        pjmedia-i686-pc-mingw32
        pjmedia-codec-i686-pc-mingw32
        pjmedia-audiodev-i686-pc-mingw32
        pjnath-i686-pc-mingw32
        pjlib-util-i686-pc-mingw32
        pj-i686-pc-mingw32
        portaudio-i686-pc-mingw32
        pjmedia-i686-pc-mingw32
        gsmcodec-i686-pc-mingw32
        ilbccodec-i686-pc-mingw32
        speex-i686-pc-mingw32
        resample-i686-pc-mingw32
        srtp-i686-pc-mingw32
        m
        winmm
        ole32
        ws2_32
        wsock32
        ssl
        crypto
        )
ELSE()
    target_link_libraries(voipc
        pjsua-x86_64-unknown-linux-gnu
        pjsip-ua-x86_64-unknown-linux-gnu
        pjsip-simple-x86_64-unknown-linux-gnu
        pjsip-x86_64-unknown-linux-gnu
        pjmedia-codec-x86_64-unknown-linux-gnu
        pjmedia-x86_64-unknown-linux-gnu
        pjmedia-audiodev-x86_64-unknown-linux-gnu
        pjnath-x86_64-unknown-linux-gnu
        pjlib-util-x86_64-unknown-linux-gnu
        pj-x86_64-unknown-linux-gnu
        portaudio-x86_64-unknown-linux-gnu
        gsmcodec-x86_64-unknown-linux-gnu
        speex-x86_64-unknown-linux-gnu
        ilbccodec-x86_64-unknown-linux-gnu
        srtp-x86_64-unknown-linux-gnu
        resample-x86_64-unknown-linux-gnu
        opencore-amrnb
        asound
        m
        pthread
        ssl
        crypto
        uuid
        )
ENDIF()

target_link_libraries(voipc qjson)

