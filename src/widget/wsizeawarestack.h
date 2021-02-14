#pragma once

#include <QWidget>
#include <QEvent>

#include "widget/wbasewidget.h"

class SizeAwareLayout;

class WSizeAwareStack : public QWidget, public WBaseWidget {
    Q_OBJECT
  public:
    explicit WSizeAwareStack(QWidget* parent = nullptr);

    int addWidget(QWidget* widget);

  protected:
    void resizeEvent(QResizeEvent* event) override;
    bool event(QEvent* pEvent) override;

  private:
    SizeAwareLayout* m_layout;
};
