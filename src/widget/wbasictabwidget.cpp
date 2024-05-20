#include "widget/wbasictabwidget.h"

#include <QKeyEvent>

#include "moc_wbasictabwidget.cpp"

WBasicTabWidget::WBasicTabWidget(QWidget* parent)
        : QTabWidget(parent) {
}

void WBasicTabWidget::keyPressEvent(QKeyEvent* event) {
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

    QTabWidget::keyPressEvent(event);
}
