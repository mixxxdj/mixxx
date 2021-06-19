#pragma once

#include <QApplication>

class ControlProxy;

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
