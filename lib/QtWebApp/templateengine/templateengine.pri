INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

greaterThan(QT_VERSION,6) {
    QT += core5compat
}

HEADERS += $$PWD/templateglobal.h
HEADERS += $$PWD/template.h 
HEADERS += $$PWD/templateloader.h 
HEADERS += $$PWD/templatecache.h

SOURCES += $$PWD/template.cpp 
SOURCES += $$PWD/templateloader.cpp 
SOURCES += $$PWD/templatecache.cpp
