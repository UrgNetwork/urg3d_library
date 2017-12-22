!contains( included_modules, $$PWD ) {
    included_modules += $$PWD

    win32:LIBS  += -lwsock32 -lsetupapi

#    DEFINES += DEBUG_LOW
#    win32:DEFINES     += _WIN32

    DEPENDPATH  += \
                $$PWD

    INCLUDEPATH += \
                $$PWD/include

    HEADERS += \
        $$PWD/include/urg3d_sensor.h \
        $$PWD/include/urg3d_connection.h \
        $$PWD/include/urg3d_detect_os.h \
        $$PWD/include/urg3d_errno.h \
        $$PWD/include/urg3d_ring_buffer.h \
        $$PWD/include/urg3d_tcpclient.h \
        $$PWD/include/urg3d_ticks.h

    SOURCES += \
        $$PWD/src/urg3d_sensor.c \
        $$PWD/src/urg3d_connection.c \
        $$PWD/src/urg3d_ring_buffer.c \
        $$PWD/src/urg3d_tcpclient.c \
        $$PWD/src/urg3d_ticks.c
}
