/**
* @file widgethider.h
* @author Josep Maria Antolín
* @date Feb 27 2017
* @brief class for hiding widgets without changing other object sizes and positions
*/

#include "util/widgethider.h"

#include <QSizePolicy>

WidgetHider::WidgetHider(QObject * parent)
    : QObject(parent)
{}

#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
bool WidgetHider::eventFilter(QObject *, QEvent * ev) {
    return ev->type() == QEvent::Paint;
}
void WidgetHider::retainSizeFor(QWidget* widget) {}
void WidgetHider::hideWidget(QWidget * w) {
    w->installEventFilter(this);
    w->update();
}
void WidgetHider::showWidget(QWidget * w) {
    w->removeEventFilter(this);
    w->update();
}
#else
bool WidgetHider::eventFilter(QObject *, QEvent *) {}
void WidgetHider::retainSizeFor(QWidget* widget)
{
    QSizePolicy sp_retain = widget->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    widget->setSizePolicy(sp_retain);
}
void WidgetHider::hideWidget(QWidget* widget)
{
    widget->setVisible(false);
}
void WidgetHider::showWidget(QWidget* widget)
{
    widget->setVisible(true);
}
#endif
