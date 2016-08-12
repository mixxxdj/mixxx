#include "layoutstoolmain.h"
#include "defs.h"
#include <QDebug>

LayoutsToolMain::LayoutsToolMain(QObject *parent) : QObject(parent) {
    app = QCoreApplication::instance();

    pLayoutsFileHandler = new LayoutsFileHandler();
}

void LayoutsToolMain::run() {
    qDebug() << "Welcome to the Layouts tool :)";

    QDir layoutsPath(LAYOUTS_CPP_PATH);
    pLayoutsFileHandler->open(layoutsPath);

    quit();
}

void LayoutsToolMain::quit() {
    emit finished();
}

void LayoutsToolMain::aboutToQuitApp() {
    delete pLayoutsFileHandler;
}