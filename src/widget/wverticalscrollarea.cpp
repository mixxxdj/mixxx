#include <QEvent>
#include <QDebug>
#include <QScrollBar>

#include "wverticalscrollarea.h"

WVerticalScrollArea::WVerticalScrollArea(QWidget* parent)
        : QScrollArea(parent) {
    setWidgetResizable(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
}

void WVerticalScrollArea::setWidget(QWidget* widget) {
    widget->installEventFilter(this);
    QScrollArea::setWidget(widget);
}

bool WVerticalScrollArea::eventFilter(QObject* o, QEvent* e) {
    if (o == widget() && e->type() == QEvent::Resize) {
        int vScrollWidth = verticalScrollBar()->width();
        int width = widget()->minimumSizeHint().width();
        setFixedWidth(width);
        widget()->setContentsMargins(0, 0, vScrollWidth, 0);
    }
    return false;
}
