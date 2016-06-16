#include "wrightclickbutton.h"

WRightClickButton::WRightClickButton(QWidget* parent)
        : QToolButton(parent) {
    connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()));
}

void WRightClickButton::setData(const QString &data) {
    m_data = data;
}



void WRightClickButton::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        emit(rightClicked(event->globalPos()));
    }
    QToolButton::mousePressEvent(event);
}

void WRightClickButton::slotClicked() {
    emit(clicked(m_data));
}

