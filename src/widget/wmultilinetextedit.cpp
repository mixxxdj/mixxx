#include "widget/wmultilinetextedit.h"

#include <QSizeF>
#include <QStyle>
#include <QStyleOption>
#include <QStylePainter>

#include "moc_wmultilinetextedit.cpp"
#include "wmultilinetextedit.h"

WMultiLineTextEdit::WMultiLineTextEdit(QWidget* parent)
        : QPlainTextEdit(parent) {
    // The added viewportMargin is required to ensure the inner
    // content of the text editor does not accidentally overwrite
    // the focus frame of the whole control. The documentMargin is
    // reduced by an equal amount so the total amount of padding
    // stays the same.
    setViewportMargins(4, 4, 4, 4);
    document()->setDocumentMargin(0);
}

QSize WMultiLineTextEdit::minimumSizeHint() const {
    const int minLines = 2;
    return sizeHintImpl(minLines);
}

QSize WMultiLineTextEdit::sizeHint() const {
    const int minLines = 2;
    return sizeHintImpl(minLines);
}

QSize WMultiLineTextEdit::sizeHintImpl(const int minLines) const {
    const auto w = 0.0;
    const auto h = 2 * frameWidth() + 2 * document()->documentMargin() +
            QFontMetrics(font()).lineSpacing() * minLines;

    return QSizeF(w, h).toSize();
}

bool WMultiLineTextEdit::event(QEvent* e) {
    switch (e->type()) {
    case QEvent::Paint: {
        QStylePainter p(this);
        QStyleOptionFrame option;
        initStyleOption(&option);
        if (isReadOnly()) {
            option.state |= QStyle::State_ReadOnly;
        }
        style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, &p, this);
        return true;
    }
    default: {
        return QPlainTextEdit::event(e);
    }
    }
}

void WMultiLineTextEdit::keyPressEvent(QKeyEvent* event) {
    const bool isUp = event->key() == Qt::Key_Up;
    const bool isDown = event->key() == Qt::Key_Down;

    // Do not interpret as navigation if any modifier key is pressed
    const bool anyModifiersPressed = event->modifiers() &
            (Qt::ControlModifier | Qt::AltModifier |
                    Qt::ShiftModifier | Qt::MetaModifier);

    // Holding down a key to scroll all the way to the
    // top or bottom of the text edit should not cause
    // the focus to leave the text edit control.
    const bool isAutoRepeat = event->isAutoRepeat();

    if (!m_upDownChangesFocus || !(isUp || isDown) ||
            isAutoRepeat || anyModifiersPressed) {
        QPlainTextEdit::keyPressEvent(event);
    } else {
        // Up/Down should only cause a focus change when the edge
        // of the text box has been reached i.e. when the text
        // cursor position didn't change due to the keypress.
        auto oldCursor = textCursor();
        QPlainTextEdit::keyPressEvent(event);
        auto newCursor = textCursor();

        const bool cursorHasNotChanged = oldCursor.position() == newCursor.position() &&
                oldCursor.anchor() == newCursor.anchor();

        if (cursorHasNotChanged && isUp && focusPreviousChild()) {
            event->accept();
            return;
        }
        if (cursorHasNotChanged && isDown && focusNextChild()) {
            event->accept();
            return;
        }
    }
}
