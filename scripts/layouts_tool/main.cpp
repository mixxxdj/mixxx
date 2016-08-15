#include <QtCore/QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QApplication>
#include "layoutstoolmain.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    LayoutsToolMain layoutTools;

    // Connect up the signals
    QObject::connect(&layoutTools, SIGNAL(finished()), &app, SLOT(quit()));
    QObject::connect(&app, SIGNAL(aboutToQuit()), &layoutTools, SLOT(aboutToQuitApp()));

    // This code will start the messaging engine in QT and in
    // 10ms it will start the execution in the LayoutsToolMain.run routine
    QTimer::singleShot(10, &layoutTools, SLOT(run()));
    return app.exec();
}