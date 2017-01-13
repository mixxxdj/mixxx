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
    setFocusPolicy(Qt::NoFocus);
}

void WVerticalScrollArea::setWidget(QWidget* widget) {
    widget->installEventFilter(this);
    QScrollArea::setWidget(widget);
}

bool WVerticalScrollArea::eventFilter(QObject* o, QEvent* e) {
    if (o == widget() && e->type() == QEvent::Resize) {
        calcSize();
    }
    return false;
}

void WVerticalScrollArea::resizeEvent(QResizeEvent *e) {
    QScrollArea::resizeEvent(e);
    calcSize();
}

void WVerticalScrollArea::calcSize() {
    int width = widget()->minimumSizeHint().width();
    int vScrollWidth = 0;
    
    if (height() <= widget()->minimumSizeHint().height()) {
        vScrollWidth = verticalScrollBar()->width();
    }
    setFixedWidth(width + frameWidth() * 2 + vScrollWidth);
}

void WVerticalScrollArea::slotEnsureVisible(QWidget* widget) {
    //qDebug() << "WVerticalScrollArea::slotEnsureVisible";
    ensureWidgetVisible(widget, 0, 0);
}

bool WVerticalScrollArea::focusNextPrevChild(bool next) {
    // QScrollArea::focusNextPrevChild scrolls to center
    // of focused child. Sip scrolling here.
    return QWidget::focusNextPrevChild(next);
}
