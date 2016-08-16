#include <QDebug>
#include <QResizeEvent>
#include <QScrollBar>

#include "wverticalscrollarea.h"

WVerticalScrollArea::WVerticalScrollArea(QWidget* parent)
        : QScrollArea(parent) {
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignTop);
    setWidgetResizable(true);
}

void WVerticalScrollArea::setWidget(QWidget* widget) {
    widget->installEventFilter(this);
    QScrollArea::setWidget(widget);
}

void WVerticalScrollArea::resizeEvent(QResizeEvent *e) {
    QScrollArea::resizeEvent(e);
    
    int width = widget()->minimumSizeHint().width();
    int vScrollWidth = 0;
    
    if (e->size().height() <= widget()->minimumSizeHint().height()) {
        vScrollWidth = verticalScrollBar()->width();
    }
    setFixedWidth(width + vScrollWidth);
}
