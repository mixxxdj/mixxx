#ifndef LAYOUTS_TOOL_MAIN_H
#define LAYOUTS_TOOL_MAIN_H

#include <QObject>
#include <QApplication>
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
    QApplication *app;
    LayoutsFileHandler *pLayoutsFileHandler;
    QList<Layout> mLayouts;
    QString mFilePath;
    Display* m_xDisplay;

    void mainMenu();
    void editLayoutMenu();
    void addLayoutMenu();
    void removeLayoutMenu();
    void showLayouts();
};

#endif // LAYOUTS_TOOL_MAIN_H
