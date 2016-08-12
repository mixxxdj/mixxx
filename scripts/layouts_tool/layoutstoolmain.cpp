#include "layoutstoolmain.h"
#include <QDebug>
#include <QString>

LayoutsToolMain::LayoutsToolMain(QObject *parent) :
        QObject(parent),
        mFilePath(LAYOUTS_CPP_PATH) {

    app = QCoreApplication::instance();
    pLayoutsFileHandler = new LayoutsFileHandler();
}

void LayoutsToolMain::run() {
    qDebug() << "Welcome to the Layouts tool :)";

    pLayoutsFileHandler->open(mFilePath, mLayouts);
    mainMenu();
}

void LayoutsToolMain::quit() {
    emit finished();
}

void LayoutsToolMain::aboutToQuitApp() {
    delete pLayoutsFileHandler;
}

void LayoutsToolMain::mainMenu() {
    QTextStream qtin(stdin);

    bool userWantsToQuit = false;
    do {
        bool loaded = !mFilePath.isEmpty();
        int menuChoice = 0;

        // Print menu
        qDebug() << "********** LAYOUT TOOLS MAIN MENU **********";
        if (loaded) qDebug() << "Currently opened file: " << mFilePath;
        qDebug() << "(1): Open file";
        qDebug() << "(2): Save file";
        if (loaded) qDebug() << "(3): Edit file";
        qDebug() << (loaded ? "(4): Quit" : "(3): Quit");

        // Prompt user for choice
        qtin >> menuChoice;

        switch(menuChoice) {
            case 1: {
                qDebug() << "Please tell me the path to the layouts cpp file (no spaces please): ";
                qtin >> mFilePath;
                pLayoutsFileHandler->open(mFilePath, mLayouts);
                break;
            }

            case 2: {
                qDebug() << "Save file...";
                break;
            }

            case 3: {
                if (loaded) {
                    qDebug() << "Edit file...";
                } else {
                    qDebug() << "Exit...";
                }
                break;
            }

            case 4: {
                if (loaded) {
                    userWantsToQuit = true;
                    qDebug() << "Exit...";
                    quit();
                }
                break;
            }

            default:
                qDebug() << "ERROR! You have selected an invalid choice.";
                break;
        }
        qDebug();
    } while (!userWantsToQuit);
}