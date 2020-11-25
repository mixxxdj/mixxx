#pragma once

#include <QWidget>

#include "util/widgethelper.h"

class WNotificationMenuPopup : public QWidget {
    Q_OBJECT
  public:
    WNotificationMenuPopup(QWidget* parent = nullptr);

    ~WNotificationMenuPopup() {
    }

    void popup(const QPoint& p) {
        auto parentWidget = qobject_cast<QWidget*>(parent());
        QPoint topLeft = mixxx::widgethelper::mapPopupToScreen(*parentWidget, p, size());
        move(topLeft);
        show();
    }

  signals:
    void aboutToHide();
    void aboutToShow();

  protected:
    void closeEvent(QCloseEvent* event) override;
};
