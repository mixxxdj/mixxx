#ifndef MIXXXAPPLICATION_H
#define MIXXXAPPLICATION_H

#include <QApplication>

class ControlProxy;

class MixxxApplication : public QApplication {
    Q_OBJECT
  public:
    MixxxApplication(int& argc, char** argv);
    ~MixxxApplication() override;

  private:
    bool touchIsRightButton();
    void registerMetaTypes();

    int m_fakeMouseSourcePointId;
    QWidget* m_fakeMouseWidget;
    enum Qt::MouseButton m_activeTouchButton;
    ControlProxy* m_pTouchShift;

};

#endif // MIXXXAPPLICATION_H
