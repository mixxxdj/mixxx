#include "util/widgethider.h"

#include <QSizePolicy>

WidgetHider::WidgetHider(QObject* parent)
        : QObject(parent) {
}

bool WidgetHider::eventFilter(QObject* watched, QEvent* event) {
    return QObject::eventFilter(watched,event);
}

void WidgetHider::retainSizeFor(QWidget* widget)
{
    QSizePolicy sp_retain = widget->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    widget->setSizePolicy(sp_retain);
}

void WidgetHider::hideWidget(QWidget* widget) {
    widget->setVisible(false);
}

void WidgetHider::showWidget(QWidget* widget)
{
    widget->setVisible(true);
}
