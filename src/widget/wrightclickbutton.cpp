#include "wrightclickbutton.h"

WRightClickButton::WRightClickButton(QWidget* parent)
        : QToolButton(parent) {

}

void WRightClickButton::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        emit(rightClicked(event->globalPos()));
    }
    QToolButton::mousePressEvent(event);
}

