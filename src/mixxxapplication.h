#ifndef MIXXXAPPLICATION_H
#define MIXXXAPPLICATION_H

#include <QApplication>
#include <QByteArrayData>
#include <QString>

class ControlProxy;
class QEvent;
class QObject;

class MixxxApplication : public QApplication {
    Q_OBJECT
  public:
    MixxxApplication(int& argc, char** argv);
    ~MixxxApplication() override = default;

    bool notify(QObject*, QEvent*) override;

  private:
    bool touchIsRightButton();
    void registerMetaTypes();

    int m_rightPressedButtons;
    ControlProxy* m_pTouchShift;

};

#endif // MIXXXAPPLICATION_H
