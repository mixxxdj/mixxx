#include "library/multilineeditdelegate.h"

#include <QAbstractTextDocumentLayout>
#include <QScrollBar>
#include <cmath>

#include "moc_multilineeditdelegate.cpp"

MultiLineEditor::MultiLineEditor(QWidget* pParent)
        : QPlainTextEdit(pParent) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    // Disable line wrap for a predictable view (like QLineEdit).
    // Horizontal scrollbars show up automatically.
    setLineWrapMode(QPlainTextEdit::NoWrap);
    // Remove ugly content offset, most notable with one-liners
    setContentsMargins(0, 0, 0, 0);
    document()->setDocumentMargin(0);
    // Paint the entire rectangle, i.e. expand document background in order to
    // cover all underlying index text. Seems to be required for one-liners on macOS.
    setBackgroundVisible(true);
    // Add event filter to catch right-clicks and key presses, see eventFilter()
    installEventFilter(this);
};

bool MultiLineEditor::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        // Work around a strange quirk: right-clicks outside the rectangle of
        // the underlying table index are not triggering the context menu.
        // Simply returning true fixes it.
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if (me->button() == Qt::RightButton && rect().contains(me->pos(), false)) {
            return true;
        }
    } else if (event->type() == QEvent::KeyPress) {
        // Finish editing with Return key like in QLineEdit
        QKeyEvent* ke = static_cast<QKeyEvent*>(event);
        if ((ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) &&
                ke->modifiers().testFlag(Qt::NoModifier)) {
            emit editingFinished();
            return false;
        }
    }
    return QPlainTextEdit::eventFilter(obj, event);
}

MultiLineEditDelegate::MultiLineEditDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView) {
}

QWidget* MultiLineEditDelegate::createEditor(QWidget* pParent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    Q_UNUSED(index);
    auto* pEditor = new MultiLineEditor(pParent);
    auto* pDocLayout = pEditor->document()->documentLayout();
    // Adjust height to fit content and maybe shift vertically to fit into the
    // library view. documentSizeChanged() is emitted when text changed, incl.
    // initial fill.
    connect(pDocLayout,
            &QAbstractTextDocumentLayout::documentSizeChanged,
            this,
            [this, pEditor, index](const QSizeF size) {
                // Pass the current geometry of the index so the editor is
                // positioned correctly when the text is edited after the
                // table was scrolled.
                adjustEditor(pEditor, size, m_pTableView->visualRect(index));
            });
    // Also emitted when pressing Return key, see MultiLineEditor::keyPressEvent()
    connect(pEditor,
            &MultiLineEditor::editingFinished,
            this,
            &MultiLineEditDelegate::commitAndCloseEditor);
    return pEditor;
}

void MultiLineEditDelegate::adjustEditor(MultiLineEditor* pEditor,
        const QSizeF size,
        QRect iRect) const {
    // Compared to QTextEdit, size.height() is the line count (Qt speak: blocks)
    int lines = static_cast<int>(round(size.height()));
    // Remove the scrollbars if content is just one line to emulate QLineEdit
    // appearace, else enable auto mode.
    Qt::ScrollBarPolicy pol(lines > 1 ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
    pEditor->setVerticalScrollBarPolicy(pol);
    pEditor->setHorizontalScrollBarPolicy(pol);

    // Calculate the content height
    // Add extra margin so the horizontal scrollbar doesn't obstruct the last
    // line (which also avoids the vertical scrollbar as long as possible)
    lines += lines > 1 ? 1 : 0;

    QFontMetrics fm(pEditor->document()->defaultFont());
    // Don't let the editor shrink smaller than the original height
    // Note: also setBackgroundVisible(true) on the editor to paint the entire
    // rectangle, not just the document.
    int txtH = fm.lineSpacing() * lines;
    // If it's just one line center the text vertically like QLineEdist do.
    // Note that the offset is applied in all directions which, hence with very
    // tall library row spacing the left offset may be disturbing.
    int diffH = (iRect.height() - txtH - pEditor->frameWidth() * 2) / 2;
    if (lines == 1 && diffH > 1) {
        pEditor->document()->setDocumentMargin(diffH);
    } else { // Reset if lines were added
        pEditor->document()->setDocumentMargin(0);
    }

    int newH = std::max(txtH, iRect.height());
    // Limit editor to visible table height
    QRect tableRect = m_pTableView->viewport()->rect();
    int tableH = tableRect.height();
    newH = std::min(newH, tableH);
    // If the editor overflows the table view, move it up so it's not clipped.
    // No need to care about y < 0 or y > (table height - line height) since the
    // table already ensures visibility when the index is selected.
    int newY = iRect.y();
    if ((newY + newH) > tableH) {
        newY = tableH - newH;
    }

    // Calculate the content width
    // Initial size.width() is the factor for tabStopDistance.
    // After first resize the width is in pixels
    // Let the editor expand horizontally like QLineEdit, limit to table width
    int newW = std::max(static_cast<int>(size.width()) + pEditor->frameWidth() * 2,
            iRect.width());

    // Also limit width so scrollbars are visible and table is not scrolled if
    // cursor is moved horizontally.
    int tableW = tableRect.width();
    if (iRect.x() + newW > tableW) {
        newW = tableW - iRect.x();
    }

#ifdef __APPLE__
    // Don't let table view scrollbars overlap the editor
    int scrollW = m_pTableView->verticalScrollBar()->width();
    if (scrollW > 0 && (iRect.x() + newW > tableW - scrollW)) {
        newW -= scrollW;
    }
    int scrollH = m_pTableView->horizontalScrollBar()->height();
    if (scrollH > 0 && (newH > tableH - scrollH)) {
        if (newY >= scrollH) {
            // shift it up
            newY += scrollH;
        } else {
            // reduce height
            newH -= scrollH;
        }
    }
#endif

    pEditor->setGeometry(QRect(iRect.x(), newY, newW, newH));
}

void MultiLineEditDelegate::commitAndCloseEditor() {
    MultiLineEditor* pEditor = qobject_cast<MultiLineEditor*>(sender());
    emit commitData(pEditor);
    emit closeEditor(pEditor);
}
