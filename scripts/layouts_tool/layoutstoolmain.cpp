#include "layoutstoolmain.h"
#include "utils.h"
#include <QDebug>
#include <iostream>

LayoutsToolMain::LayoutsToolMain(QObject* parent)
        : QObject(parent) {
    // Find layouts.cpp path (check for Mixxx directory four directories up)
    bool layoutsFound = false;
    QDir cp = QDir::currentPath();

    for (int i = 0; i < 5; i++) {
        cp.cdUp();
        QString path(cp.absolutePath()+"/src/controllers/keyboard");
        QFileInfo checkFile(path);

        if (checkFile.exists()) {
            m_FilePath = path + "/layouts.cpp";
            qDebug() << "Found path: " << path;
            layoutsFound = true;
        }
    }

    if (!layoutsFound) {
        m_FilePath = "";
    }

    qApp;
    m_pLayoutsFileHandler = new LayoutsFileHandler();
}

void LayoutsToolMain::run() {
    utils::qout() << "Welcome to the Layouts tool :)" << endl;
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
    // TODO(Tomasito) Make Menu class to avoid this "if (loaded)"
    // ...            mess as well as this switch statement mess

    bool userWantsToQuit = false;
    do {
        bool loaded = !m_FilePath.isEmpty();
        int menuChoice = 0;

        // Print menu
        //utils::clearTerminal();
        utils::qout() << "********** LAYOUTS TOOL - MAIN MENU **********" << endl;
        if (loaded) utils::qout() << "Currently opened file: " << m_FilePath << endl;
        utils::qout() << "(1): Open file" << endl;
        utils::qout() << "(2): Save file" << endl;
        if (loaded) utils::qout() << "(3): Edit file" << endl;
        if (loaded) utils::qout() << "(4): Miscellaneous tools" << endl;
        utils::qout() << (loaded ? "(5): Quit" : "(3): Quit") << endl;

        // Prompt user for choice
        utils::qin() >> menuChoice;

        switch(menuChoice) {
            case 1: {
                utils::clearTerminal();
                utils::qout() << "Please tell me the path to the layouts cpp file (no spaces please): " << endl;
                utils::qin() >> m_FilePath;
                m_pLayoutsFileHandler->open(m_FilePath, m_Layouts);
                break;
            }

            case 2: {
                utils::qout() << "Save file " << m_FilePath << "..." << endl;
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
                    // Miscellaneous tools
                    miscToolsMenu();
                    break;
                }
            }

            case 5: {
                if (loaded) {
                    userWantsToQuit = true;
                    quit();
                    break;
                }
            }

            default:
                qDebug() << "ERROR! You have selected an invalid choice.";
                break;
        }
    } while (!userWantsToQuit);
}

void LayoutsToolMain::editLayoutMenu() {
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
        utils::qout() << "********** LAYOUTS TOOL - EDIT **********" << endl;
        utils::qout() << "Editing file: " << m_FilePath << endl;
        utils::qout() << "(1): Remove layout" << endl;
        utils::qout() << "(2): Add layout" << endl;
        utils::qout() << "(3): Back to main menu" << endl;

        // Prompt user for choice
        utils::qin() >> menuChoice;

        switch(menuChoice) {
            case 1: {
                // Remove layouts
                removeLayoutMenu();
                break;
            }

            case 2: {
                // Add layout
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
    QString kbdLocale = utils::inputLocaleName();

    utils::clearTerminal();
    utils::qout() << "********** LAYOUTS TOOL - ADD LAYOUT **********" << endl;
    utils::qout() << "Qt keyboard locale: " << kbdLocale
             << " (NOTE: This is not accurate any more when changing layout runtime)" << endl;

    utils::qout() << "Enter layout name: " << endl;
    QString layoutName = kbdLocale;
    utils::qin().skipWhiteSpace();
    layoutName = utils::qin().readLine();

    utils::qout() << "Enter layout variable name (see Qt keyboard locale): " << endl;
    utils::qout() << "NOTE: Please switch to the layout you want to save before hitting ENTER!" << endl;
    QString variableName = kbdLocale;
    utils::qin() >> variableName;

    qDebug() << QString("\nAdding layout with:\n  Name:\t\t\t\t%1\n  Variable name:\t%2\n").arg(layoutName, variableName)
            .toStdString()
            .c_str();

    Layout layout(variableName, layoutName);
    m_Layouts.append(layout);
}

void LayoutsToolMain::removeLayoutMenu() {
    bool backToEdit;
    do {
        int menuChoice = 0;

        // Print menu
        utils::clearTerminal();
        utils::qout() << "********** LAYOUTS TOOL - REMOVE LAYOUT **********" << endl;
        showLayouts();
        utils::qout() << "(" << m_Layouts.size() << ") " << "Back to edit menu";

        // Prompt user for choice
        utils::qin() >> menuChoice;

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

void LayoutsToolMain::miscToolsMenu() {
    bool loaded = !m_FilePath.isEmpty();
    if (!loaded) {
        qDebug() << "Miscellaneous tools do only work when a "
                 << "layouts file is loaded. Going back to main menu.";
        return;
    }

    bool backToMain = false;
    do {
        int menuChoice = 0;

        // Print menu
        utils::clearTerminal();
        utils::qout() << "********** LAYOUTS TOOL - MISCELLANEOUS TOOLS **********" << endl;
        utils::qout() << "Miscellaneous tools on file: " << m_FilePath << endl;
        utils::qout() << "(1): Find shared key chars (keys on one single layout that share the same character)" << endl;
        utils::qout() << "(2): Back to main menu" << endl;

        // Prompt user for choice
        utils::qin() >> menuChoice;

        switch(menuChoice) {
            case 1: {
                utils::qout() << "Find shared key chars..." << endl;
                findSharedKeyCharsMenu();
                break;
            }

            case 2: {
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

void LayoutsToolMain::findSharedKeyCharsMenu() {
    bool backToMain = false;
    do {
        int menuChoice = 0;

        // Print menu
        utils::clearTerminal();
        utils::qout() << "********** LAYOUTS TOOL - FIND SHARED KEY CHARS **********" << endl;
        utils::qout() << "Finding shared key chars on layout: " << m_FilePath << endl;

        // Show shared key chars
        for (const auto& layout : m_Layouts) {
            // Iterate through each KbdKeyChar in current layout
            for (int i = 0; i < kLayoutLen; i++) {
                int keycode = utils::layoutIndexToKeycode(i);

                // Do some code for non modified KbdKeyChar, then for shifted one
                for (bool shift : {false, true}) {
                    const KbdKeyChar& kbdKeyChar = layout.m_data[i][shift];

                    // Find layout indexes of KbdKeyChars that match current one
                    QList<int> foundMatches;
                    for (int j = 0; j < kLayoutLen && j != i; j++) {
                        if (kbdKeyChar.character == layout.m_data[j][shift].character) {
                            foundMatches.push_back(j);
                        }
                    }

                    if (foundMatches.isEmpty()) continue;

                    // Let's inform the user about the duplicate key(s) we found for this layout
                    for (const int& k : foundMatches) {
                        int otherKeycode = utils::layoutIndexToKeycode(k);

                        utils::qout() << "On layout " << layout.m_name << ", "
                                      << utils::keycodeToKeyname(keycode)
                                      << " is identical to "
                                      << utils::keycodeToKeyname(otherKeycode) << " "
                                      << (shift ? "with" : "without") << " shift. "
                                      << "Character: '" << QChar(kbdKeyChar.character) << "'"
                                      << endl;
                    }
                }
            }
        }

        utils::qout() << endl;
        utils::qout() << "(1): Back to miscellaneous tools" << endl;

        // Prompt user for choice
        utils::qin() >> menuChoice;

        switch(menuChoice) {
            case 1: {
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

void LayoutsToolMain::showLayouts() {
    int i = 0;
    for (const Layout& layout : m_Layouts) {
        utils::qout() << "(" << i++ << ") "
                      << layout.m_variableName << "\t" << layout.m_name;
    }
}
