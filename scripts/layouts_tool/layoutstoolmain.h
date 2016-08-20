#ifndef LAYOUTS_TOOL_MAIN_H
#define LAYOUTS_TOOL_MAIN_H

#include <QObject>
#include <QApplication>
#include "layoutsfilehandler.h"

class LayoutsToolMain : public QObject {
  Q_OBJECT

  public:
    explicit LayoutsToolMain(QObject* parent = 0);
    void quit();

  signals:
    void finished();

  public slots:
    void run();
    void aboutToQuitApp();

  private:
    LayoutsFileHandler *m_pLayoutsFileHandler;
    QList<Layout> m_Layouts;
    QString m_FilePath;

    void mainMenu();
    void editLayoutMenu();
    void addLayoutMenu();
    void removeLayoutMenu();
    void miscToolsMenu();
    void showLayouts();

    void findSharedKeyCharsMenu();
};

#endif // LAYOUTS_TOOL_MAIN_H
