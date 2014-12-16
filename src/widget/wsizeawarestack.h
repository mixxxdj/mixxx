#ifndef WSIZEAWARESTACK_H
#define WSIZEAWARESTACK_H

#include <QWidget>
#include <QEvent>

#include "widget/wbasewidget.h"

class SizeAwareLayout;

class WSizeAwareStack : public QWidget, public WBaseWidget {
    Q_OBJECT
  public:
    WSizeAwareStack(QWidget* pParent = NULL);
    virtual ~WSizeAwareStack();

    int addWidget(QWidget* pWidget);

  protected:
    virtual void resizeEvent(QResizeEvent* event);
    bool event(QEvent* pEvent);

  private:
    SizeAwareLayout* m_layout;
};

#endif /* WSIZEAWARESTACK_H */
