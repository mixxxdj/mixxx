#ifndef MIXXXAPPLICATION_H
#define MIXXXAPPLICATION_H

#include <QApplication>

class ControlProxy;

class MixxxApplication : public QApplication {
    Q_OBJECT

  public:
    MixxxApplication(int& argc, char** argv);
    virtual ~MixxxApplication();
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    virtual bool notify(QObject*, QEvent*);
#endif

  private:
    bool touchIsRightButton();

    int m_fakeMouseSourcePointId;
    QWidget* m_fakeMouseWidget;
    enum Qt::MouseButton m_activeTouchButton;
    ControlProxy* m_pTouchShift;

};

#endif // MIXXXAPPLICATION_H
