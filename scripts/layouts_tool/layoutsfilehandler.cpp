#include <iostream>
#include <dlfcn.h>
#include <QDebug>

#include "layoutsfilehandler.h"

LayoutsFileHandler::LayoutsFileHandler() {}

LayoutsFileHandler::~LayoutsFileHandler() {}

void LayoutsFileHandler::open(QDir &layoutsPath) {

    // Setup path where .so will be saved
    QDir layoutsSoDir = layoutsPath;
    layoutsSoDir.cdUp();
    layoutsSoDir = QDir(layoutsSoDir.canonicalPath() + "/layouts.so");

    const QString cppPath = layoutsPath.canonicalPath();
    const QString soPath = layoutsSoDir.canonicalPath();

    qDebug() << "SO PATH: " << soPath;

    // Compile layouts
    system(("c++ " + cppPath + " -o " + soPath + " -shared -fPIC").toLatin1().data());

    // Open layouts
    qDebug() << "Opening " << soPath;
    void* handle = dlopen(soPath.toLatin1().data(), RTLD_LAZY);
    if (!handle) {
        qFatal("Could not open layouts library");
    }

    // Load getLayout function
    qDebug() << "Loading getLayout function symbol...";
    typedef void (*getLayout_t)(std::string layoutName);
    getLayout_t getLayout = (getLayout_t) dlsym(handle, "getLayout");
    if (!getLayout) {
        qCritical() << dlerror();
        dlclose(handle);
        qFatal("Couldn't load symbol 'getLayout'");
    }

    // use it to do the calculation
    qDebug() << "Calling hello...";
    getLayout("de_DE");

    // close the library
    qDebug() << "Closing library...\n";
    dlclose(handle);
}

void LayoutsFileHandler::save(QFile &file) {

}
