#include <QEvent>
#include <QDebug>
#include <QScrollBar>

#include "wverticalscrollarea.h"

WVerticalScrollArea::WVerticalScrollArea(QWidget* parent)
        : QScrollArea(parent) {
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignTop);
    setWidgetResizable(true);
}

void WVerticalScrollArea::setWidget(QWidget* widget) {
    widget->installEventFilter(this);
    QScrollArea::setWidget(widget);
}

bool WVerticalScrollArea::eventFilter(QObject* o, QEvent* e) {
    if (o == widget() && e->type() == QEvent::Resize) {
        int width = widget()->minimumSizeHint().width();
        int vScrollWidth = verticalScrollBar()->width();
        // + 2 for a Gap between scroll area and bar 
        setFixedWidth(width + vScrollWidth);
    }
    return false;
}
