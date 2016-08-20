#include "layoutstoolmain.h"
#include "utils.h"
#include <QDebug>

LayoutsToolMain::LayoutsToolMain(QObject* parent) :
        QObject(parent) {

    // Find layouts.h path (check for Mixxx directory four directories up)
    bool layoutsFound = false;
    QDir cp = QDir::currentPath();

    for (int i = 0; i < 5; i++) {
        QString path(cp.absolutePath()+"/src/controllers/keyboard/layouts.cpp");
        QFileInfo checkFile(path);

        if (checkFile.exists() && checkFile.isFile()) {
            m_FilePath = path;
            qDebug() << "Found path: " << path;
            layoutsFound = true;
        }
        cp.cdUp();
    }

    if (!layoutsFound) {
        m_FilePath = "";
    }

    qApp;
    m_pLayoutsFileHandler = new LayoutsFileHandler();
}

void LayoutsToolMain::run() {
    qDebug() << "Welcome to the Layouts tool :)";

    m_pLayoutsFileHandler->open(m_FilePath, m_Layouts);
    mainMenu();
}

void LayoutsToolMain::quit() {
    emit finished();
}

void LayoutsToolMain::aboutToQuitApp() {
    delete m_pLayoutsFileHandler;
}

void LayoutsToolMain::mainMenu() {
    QTextStream qtin(stdin);

    bool userWantsToQuit = false;
    do {
        bool loaded = !m_FilePath.isEmpty();
        int menuChoice = 0;

        // Print menu
        utils::clearTerminal();
        qDebug() << "********** LAYOUTS TOOL - MAIN MENU **********";
        if (loaded) qDebug() << "Currently opened file: " << m_FilePath;
        qDebug() << "(1): Open file";
        qDebug() << "(2): Save file";
        if (loaded) qDebug() << "(3): Edit file";
        qDebug() << (loaded ? "(4): Quit" : "(3): Quit");

        // Prompt user for choice
        qtin >> menuChoice;

        switch(menuChoice) {
            case 1: {
                utils::clearTerminal();
                qDebug() << "Please tell me the path to the layouts cpp file (no spaces please): ";
                qtin >> m_FilePath;
                m_pLayoutsFileHandler->open(m_FilePath, m_Layouts);
                break;
            }

            case 2: {
                qDebug() << "Save file...";
                QFile f(m_FilePath);
                m_pLayoutsFileHandler->save(f, m_Layouts);
                break;
            }

            case 3: {
                if (loaded) {
                    editLayoutMenu();
                } else {
                    userWantsToQuit = true;
                    quit();
                }
                break;
            }

            case 4: {
                if (loaded) {
                    userWantsToQuit = true;
                    quit();
                }
                break;
            }

            default:
                qDebug() << "ERROR! You have selected an invalid choice.";
                break;
        }
    } while (!userWantsToQuit);
}

void LayoutsToolMain::editLayoutMenu() {
    QTextStream qtin(stdin);
    bool loaded = !m_FilePath.isEmpty();
    if (!loaded) {
        qDebug() << "Can't edit any layout, any layout loaded.";
        return;
    }

    bool backToMain = false;
    do {
        int menuChoice = 0;

        // Print menu
        utils::clearTerminal();
        qDebug() << "********** LAYOUTS TOOL - EDIT **********";
        qDebug() << "Editing file: " << m_FilePath;
        qDebug() << "(1): Remove layout";
        qDebug() << "(2): Add layout";
        qDebug() << "(3): Back to main menu";

        // Prompt user for choice
        qtin >> menuChoice;

        switch(menuChoice) {
            case 1: {
                // Remove layouts
                qDebug() << "Remove layouts...";
                removeLayoutMenu();
                break;
            }

            case 2: {
                // Add layout
                qDebug() << "Add layout...";
                addLayoutMenu();
                break;
            }

            case 3: {
                // Back to main menu
                backToMain = true;
                break;
            }

            default:
                qDebug() << "ERROR! You have selected an invalid choice.";
                break;
        }
    } while (!backToMain);
}

void LayoutsToolMain::addLayoutMenu() {
    QTextStream qtin(stdin);
    QString kbdLocale = utils::inputLocaleName();

    utils::clearTerminal();
    qDebug() << "********** LAYOUTS TOOL - ADD LAYOUT **********";
    qDebug() << "Qt keyboard locale: " << kbdLocale
             << " (NOTE: This is not accurate any more when changing layout runtime)";

    qDebug() << "Enter layout name: ";
    QString layoutName = kbdLocale;
    qtin.skipWhiteSpace();
    layoutName = qtin.readLine();

    qDebug() << "Enter layout variable name (see Qt keyboard locale): ";
    qDebug() << "NOTE: Please switch to the layout you want to save before hitting ENTER!";
    QString variableName = kbdLocale;
    qtin >> variableName;

    qDebug() << QString("\nAdding layout with:\n  Name:\t\t\t\t%1\n  Variable name:\t%2\n").arg(layoutName, variableName)
            .toStdString()
            .c_str();

    Layout layout(variableName, layoutName);
    m_Layouts.append(layout);
}

void LayoutsToolMain::removeLayoutMenu() {
    QTextStream qtin(stdin);

    bool backToEdit;
    do {
        int menuChoice = 0;

        // Print menu
        utils::clearTerminal();
        qDebug() << "********** LAYOUTS TOOL - REMOVE LAYOUT **********";
        showLayouts();
        qDebug("(%d)  %s", m_Layouts.size(), "Back to edit menu");

        // Prompt user for choice
        qtin >> menuChoice;

        if (menuChoice == m_Layouts.size()) {
            break;
        }

        if (menuChoice >= m_Layouts.size() || menuChoice < 0) {
            qDebug() << "ERROR! You have selected an invalid choice.";
            continue;
        }

        m_Layouts.removeAt(menuChoice);
        backToEdit = true;
    } while (!backToEdit);
}

void LayoutsToolMain::showLayouts() {
    int i = 0;
    for (const Layout& layout : m_Layouts) {
        qDebug("(%d)  %s, [%s]", i++, layout.m_name.toLatin1().data(), layout.m_variableName.toLatin1().data());
    }
}
