#pragma once

#include <QApplication>

class ControlProxy;

class MixxxApplication : public QApplication {
    Q_OBJECT
  public:
    MixxxApplication(int& argc, char** argv);
    ~MixxxApplication() override = default;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    bool notify(QObject*, QEvent*) override;
#endif

  private:
    bool touchIsRightButton();
    void registerMetaTypes();

    int m_rightPressedButtons;
    ControlProxy* m_pTouchShift;

};
