#include "widget/rightclickpushbutton.h"

#include <QMouseEvent>

#include "moc_rightclickpushbutton.cpp"

RightClickPushButton::RightClickPushButton(const QString& text,
        QWidget* parent)
        : QPushButton(text, parent) {
}

void RightClickPushButton::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        emit rightClicked();
    }
}
