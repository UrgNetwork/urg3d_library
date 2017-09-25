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
        $$PWD/include/urg_connection.h \
        $$PWD/include/urg_detect_os.h \
        $$PWD/include/urg_errno.h \
        $$PWD/include/urg_ring_buffer.h \
        $$PWD/include/urg_tcpclient.h \
        $$PWD/include/urg_ticks.h \
        $$PWD/include/urg3d_sensor.h

    SOURCES += \
        $$PWD/src/urg_connection.c \
        $$PWD/src/urg_ring_buffer.c \
        $$PWD/src/urg_tcpclient.c \
        $$PWD/src/urg_ticks.c \
        $$PWD/src/urg3d_sensor.c
}
