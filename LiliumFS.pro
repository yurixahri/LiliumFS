QT = core5compat core httpserver network concurrent

win32 {
    QT += QCoroCore
}

CONFIG += c++20 cmdline static link_pkgconfig
QMAKE_CXXFLAGS += -fcoroutines

CONFIG(release, debug|release) {
    QMAKE_LFLAGS_RELEASE += -s
}
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

unix:!macx {
    QMAKE_LFLAGS += -Wl,-rpath,\'\$\$ORIGIN/libs\'
    INCLUDEPATH += /usr/include/qcoro6 /usr/include/qcoro6/QCoro /usr/include/qcoro6/qcoro
    LIBS += -lQCoro6Core
}

DEFINES += ZLIB_STATIC LIBARCHIVE_STATIC

LIBS += -larchive -lbz2 -llzma -lzstd

win32 {
    LIBS += -liconv -lbcrypt -lws2_32
}

SOURCES += \
        main.cpp \
        utils/config.cpp \
        utils/openwall_crypt/crypt_blowfish.c \
        utils/openwall_crypt/crypt_gensalt.c \
        utils/openwall_crypt/wrapper.c \
        utils/qtbcrypt.cpp \
        utils/stringUtils.cpp \
        webSocketHelper.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    fs.h \
    functions/accounts.h \
    functions/sources.h \
    htmlTemplate.h \
    httpHelper.h \
    utils/auth.h \
    utils/config.h \
    utils/inja.hpp \
    utils/json.hpp \
    utils/log.h \
    utils/openwall_crypt/crypt.h \
    utils/openwall_crypt/crypt_blowfish.h \
    utils/openwall_crypt/crypt_gensalt.h \
    utils/openwall_crypt/ow-crypt.h \
    utils/qtbcrypt.h \
    utils/rang.hpp \
    utils/stringUtils.h \
    utils/tar.h \
    utils/types.h \
    webSocketHelper.h


# RESOURCES += \
#     resources.qrc

win32 {
    DISTFILES += app.rc utils/openwall_crypt/x86.S
    RC_FILE += app.rc
}

target.path = /usr/bin
INSTALLS += target

