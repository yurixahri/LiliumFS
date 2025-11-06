QT = core httpserver network concurrent QCoroCore

CONFIG += c++20 cmdline
QMAKE_CXXFLAGS += -fcoroutines
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

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
    webSocketHelper.h

DISTFILES += \
    utils/openwall_crypt/x86.S

RESOURCES += \
    resources.qrc
