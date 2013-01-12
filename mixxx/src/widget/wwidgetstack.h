#ifndef WWIDGETSTACK_H
#define WWIDGETSTACK_H

#include <QWidget>
#include <QObject>
#include <QSignalMapper>
#include <QStackedWidget>

#include "controlobject.h"
#include "controlobjectthreadmain.h"

class WidgetStackControlListener : public QObject {
    Q_OBJECT
  public:
    WidgetStackControlListener(QObject* pParent, ControlObject* pControl,
                               int index);
    virtual ~WidgetStackControlListener();

  signals:
    void switchToWidget();

  public slots:
    void onCurrentWidgetChanged(int index);

  private slots:
    void slotValueChanged(double v);

  private:
    ControlObjectThreadMain m_control;
    const int m_index;
};

class WWidgetStack : public QStackedWidget {
    Q_OBJECT
  public:
    WWidgetStack(QWidget* pParent,
                 ControlObject* pNextControl,
                 ControlObject* pPrevControl);
    virtual ~WWidgetStack();

    void addWidgetWithControl(QWidget* pWidget, ControlObject* pControl);

  private slots:
    void onNextControlChanged(double v);
    void onPrevControlChanged(double v);

  private:
    QSignalMapper m_mapper;
    ControlObjectThreadMain m_nextControl;
    ControlObjectThreadMain m_prevControl;
};

#endif /* WWIDGETSTACK_H */
