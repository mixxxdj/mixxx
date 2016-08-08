#include <QEvent>
#include <QDebug>
#include <QScrollBar>

#include "wverticalscrollarea.h"

WVerticalScrollArea::WVerticalScrollArea(QWidget* parent)
        : QScrollArea(parent) {
    setWidgetResizable(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignTop | Qt::AlignHCenter);
}

void WVerticalScrollArea::setWidget(QWidget* widget) {
    widget->installEventFilter(this);
    QScrollArea::setWidget(widget);
}

bool WVerticalScrollArea::eventFilter(QObject* o, QEvent* e) {
    if (o == widget() && e->type() == QEvent::Resize) {
        int width = widget()->minimumSizeHint().width();
        int vScrollWidth = verticalScrollBar()->width();
        setFixedWidth(width + vScrollWidth);
    }
    return false;
}
