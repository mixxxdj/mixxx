#ifndef WSIZEAWARESTACK_H
#define WSIZEAWARESTACK_H

#include <QWidget>
#include <QEvent>

#include "widget/wbasewidget.h"

class ControlObject;
class ControlObjectSlave;
class SizeAwareLayout;

class WSizeAwareStack : public QWidget, public WBaseWidget {
    Q_OBJECT
  public:
    WSizeAwareStack(QWidget* pParent = NULL,
                    ControlObject* pCurrentPageControl = NULL);
    virtual ~WSizeAwareStack();

    int addWidget(QWidget* pWidget);

  protected:
    virtual void resizeEvent(QResizeEvent* event);
    bool event(QEvent* pEvent);

  private:
    SizeAwareLayout* m_layout;
    QScopedPointer<ControlObjectSlave> m_pCurrentPageControl;
};

#endif /* WSIZEAWARESTACK_H */
