#ifndef WBASEWIDGET_H
#define WBASEWIDGET_H

#include <QWidget>

class WBaseWidget {
  public:
    WBaseWidget(QWidget* pWidget);
    virtual ~WBaseWidget();

    QWidget* toQWidget() {
        return m_pWidget;
    }

  private:
    QWidget* m_pWidget;
};

#endif /* WBASEWIDGET_H */
