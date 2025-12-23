#pragma once

#include <QSize>
#include <QStackedLayout>
#include <QWidget>

#include "widget/wbasewidget.h"

class QEvent;

class SizeAwareLayout : public QStackedLayout {
    Q_OBJECT
  public:
    QSize minimumSize() const override;
    int setCurrentIndexForSize(const QSize& s);
};

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
