#ifndef LAYOUTS_TOOL_MAIN_H
#define LAYOUTS_TOOL_MAIN_H
#include <QObject>
#include <QCoreApplication>
class LayoutsToolMain : public QObject {
Q_OBJECT

private:
    QCoreApplication *app;

public:
    explicit LayoutsToolMain(QObject *parent = 0);
    void quit();

signals:
    void finished();

public slots:
    void run();
    void aboutToQuitApp();
};
#endif // LAYOUTS_TOOL_MAIN_H
