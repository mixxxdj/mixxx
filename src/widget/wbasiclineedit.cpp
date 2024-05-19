#include "widget/wbasiclineedit.h"

#include <QKeyEvent>

#include "moc_wbasiclineedit.cpp"

WBasicLineEdit::WBasicLineEdit(QWidget* parent)
        : QLineEdit(parent) {
}

WBasicLineEdit::WBasicLineEdit(const QString& text, QWidget* parent)
        : QLineEdit(text, parent) {
}

void WBasicLineEdit::keyPressEvent(QKeyEvent* event) {
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

    QLineEdit::keyPressEvent(event);
}
