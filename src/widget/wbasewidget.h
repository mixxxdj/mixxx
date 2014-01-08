#ifndef WBASEWIDGET_H
#define WBASEWIDGET_H

#include <QString>
#include <QWidget>

class WBaseWidget {
  public:
    WBaseWidget(QWidget* pWidget);
    virtual ~WBaseWidget();

    QWidget* toQWidget() {
        return m_pWidget;
    }

    void setBaseTooltip(const QString& tooltip) {
        m_baseTooltip = tooltip;
        m_pWidget->setToolTip(tooltip);
    }

    QString baseTooltip() const {
        return m_baseTooltip;
    }

  private:
    QWidget* m_pWidget;
    QString m_baseTooltip;
};

#endif /* WBASEWIDGET_H */
