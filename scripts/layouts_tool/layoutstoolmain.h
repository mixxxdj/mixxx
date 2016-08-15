#ifndef LAYOUTS_TOOL_MAIN_H
#define LAYOUTS_TOOL_MAIN_H
#include <QObject>
#include <QCoreApplication>
#include "layoutsfilehandler.h"

class LayoutsToolMain : public QObject {
Q_OBJECT

public:
    explicit LayoutsToolMain(QObject *parent = 0);
    void quit();

signals:
    void finished();

public slots:
    void run();
    void aboutToQuitApp();

private:
    QCoreApplication *app;
    LayoutsFileHandler *pLayoutsFileHandler;
    QList<Layout> mLayouts;
    QString mFilePath;

    void mainMenu();
    void editLayoutMenu();
    void removeLayoutsMenu();
    void showLayouts();
    void clearScreen();
};
#endif // LAYOUTS_TOOL_MAIN_H
