#include "widget/wbasiclabel.h"

#include <QKeyEvent>

#include "moc_wbasiclabel.cpp"

WBasicLabel::WBasicLabel(QWidget* parent, Qt::WindowFlags f)
        : QLabel(parent, f) {
}
WBasicLabel::WBasicLabel(const QString& text, QWidget* parent, Qt::WindowFlags f)
        : QLabel(text, parent, f) {
}

void WBasicLabel::keyPressEvent(QKeyEvent* event) {
    bool noModifiersPressed = !(event->modifiers() &
            (Qt::ControlModifier | Qt::AltModifier |
                    Qt::ShiftModifier | Qt::MetaModifier));

    if (event->key() == Qt::Key_Up && noModifiersPressed) {
        if (focusPreviousChild()) {
            event->accept();
            return;
        }
    } else if (event->key() == Qt::Key_Down && noModifiersPressed) {
        if (focusNextChild()) {
            event->accept();
            return;
        }
    }

    QLabel::keyPressEvent(event);
}
