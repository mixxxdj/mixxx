#include "layoutstoolmain.h"
#include <QDebug>

LayoutsToolMain::LayoutsToolMain(QObject *parent) : QObject(parent) {
    app = QCoreApplication::instance();
}

void LayoutsToolMain::run() {
    qDebug() << "Welcome to te Layouts tool :)";
    quit();
}

void LayoutsToolMain::quit() {
    emit finished();
}

void LayoutsToolMain::aboutToQuitApp() { }