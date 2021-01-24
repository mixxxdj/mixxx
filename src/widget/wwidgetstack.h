#pragma once

#include <QWidget>
#include <QObject>
#include <QStackedWidget>
#include <QEvent>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "widget/wbasewidget.h"

class WidgetStackControlListener : public QObject {
    Q_OBJECT
  public:
    WidgetStackControlListener(QObject* pParent, ControlObject* pControl,
                               int index);

    void setControl(double val) {
        m_control.set(val);
    }

  signals:
    void switchToWidget();
    void hideWidget();

  public slots:
    void onCurrentWidgetChanged(int index);

  private slots:
    void slotValueChanged(double v);

  private:
    ControlProxy m_control;
    const int m_index;
};

class WWidgetStack : public QStackedWidget, public WBaseWidget {
    Q_OBJECT
  public:
    WWidgetStack(QWidget* pParent,
                 const ConfigKey& nextConfigKey,
                 const ConfigKey& prevConfigKey,
                 const ConfigKey& currentPageConfigKey);

    // We don't want to change pages until all the pages have been added,
    // so we override Init and hook up the connection there.
    void Init() override;

    // QStackedWidget sizeHint and minimumSizeHint are the largest of all the
    // widgets in the stack. This is presumably to prevent UI resizes when the
    // stack changes. We explicitly want the UI to change when the stack changes
    // (potentially grow or shrink).
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    // Adds a page to the stack.  If this page is hidden, the the page with the
    // 0-based index given by on_hide_select will be shown.  If this value is
    // -1, the next page on the stack will be shown.
    void addWidgetWithControl(QWidget* pWidget, ControlObject* pControl,
                              int on_hide_select);

  protected:
    bool event(QEvent* pEvent) override;

  private slots:
    void onNextControlChanged(double v);
    void onPrevControlChanged(double v);
    // Fired when the control object tells us to change pages.
    void onCurrentPageControlChanged(double v);
    // Fired when we change pages.
    void onCurrentPageChanged(int);
    void showIndex(int index);
    void hideIndex(int index);
    void showEvent(QShowEvent* event) override;
    void slotSetIndex(int index);

  private:
    ControlProxy m_nextControl;
    ControlProxy m_prevControl;
    ControlProxy m_currentPageControl;

    // Optional map that defines which page to select if a page gets a hide
    // signal.
    QMap<int, int> m_hideMap;
    // A map of the individual page triggers so we can rectify state if needed.
    QMap<int, WidgetStackControlListener*> m_listeners;
};
