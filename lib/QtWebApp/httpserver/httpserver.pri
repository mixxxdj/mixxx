INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

QT += network

# Enable very detailed debug messages when compiling the debug version
CONFIG(debug, debug|release) {
    DEFINES += SUPERVERBOSE
}

HEADERS += $$PWD/httpglobal.h \
           $$PWD/httplistener.h \
           $$PWD/httpconnectionhandler.h \
           $$PWD/httpconnectionhandlerpool.h \
           $$PWD/httprequest.h \
           $$PWD/httpresponse.h \
           $$PWD/httpcookie.h \
           $$PWD/httprequesthandler.h \
           $$PWD/httpsession.h \
           $$PWD/httpsessionstore.h \
           $$PWD/staticfilecontroller.h

SOURCES += $$PWD/httpglobal.cpp \
           $$PWD/httplistener.cpp \
           $$PWD/httpconnectionhandler.cpp \
           $$PWD/httpconnectionhandlerpool.cpp \
           $$PWD/httprequest.cpp \
           $$PWD/httpresponse.cpp \
           $$PWD/httpcookie.cpp \
           $$PWD/httprequesthandler.cpp \
           $$PWD/httpsession.cpp \
           $$PWD/httpsessionstore.cpp \
           $$PWD/staticfilecontroller.cpp
