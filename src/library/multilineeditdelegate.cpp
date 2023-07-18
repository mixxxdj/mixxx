#include "library/multilineeditdelegate.h"

#include <QAbstractTextDocumentLayout>
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
        : TableItemDelegate(pTableView),
          m_lineCount(0) {
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
            [this, pEditor](const QSizeF size) {
                adjustEditor(pEditor, size);
            });
    // Also emitted when pressing Return key, see MultiLineEditor::keyPressEvent()
    connect(pEditor,
            &MultiLineEditor::editingFinished,
            this,
            &MultiLineEditDelegate::commitAndCloseEditor);
    // Store the initial rectangle so we can read the x/y origin and in adjustEditor()
    m_editRect = option.rect;
    return pEditor;
}

void MultiLineEditDelegate::adjustEditor(MultiLineEditor* pEditor, const QSizeF size) const {
    // Compared to QTextEdit, size.height() is the line count (Qt speak: blocks)
    int newLineCount = static_cast<int>(round(size.height()));
    // Only act if line count changed
    if (newLineCount == m_lineCount) {
        return;
    } else {
        m_lineCount = newLineCount;
    }

    // Remove the scrollbars if content is just one line to emulate QLineEdit
    // appearace, else enable auto mode.
    Qt::ScrollBarPolicy pol(m_lineCount > 1 ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
    pEditor->setVerticalScrollBarPolicy(pol);
    pEditor->setHorizontalScrollBarPolicy(pol);

    // Calculate the content height
    int lines = m_lineCount;
    // Add extra margin so the horizontal scrollbar doesn't obstruct the last
    // line (which also avoids the vertical scrollbar as long as possible)
    lines += lines > 1 ? 1 : 0;

    QFontMetrics fm(pEditor->document()->defaultFont());
    int newH = fm.lineSpacing() * lines;

    // Limit editor to visible table height
    int tableH = m_pTableView->viewport()->rect().height();
    newH = std::min(newH, tableH);
    // If the editor overflows the table view, move it up so it's not clipped.
    // No need to care about y < 0 or y > (table height - line height) since the
    // table already ensures visibility when the index is selected.
    int newY = m_editRect.y();
    if ((newY + newH) > tableH) {
        newY = tableH - newH;
    }
    // Also limit width so scrollbars are visible and table is not scrolled if
    // cursor is moved horizontally.
    int newW = std::min(pEditor->width(), m_pTableView->viewport()->rect().width());

    pEditor->setGeometry(QRect(m_editRect.x(), newY, newW, newH));
}

void MultiLineEditDelegate::commitAndCloseEditor() {
    MultiLineEditor* pEditor = qobject_cast<MultiLineEditor*>(sender());
    emit commitData(pEditor);
    emit closeEditor(pEditor);
}

void MultiLineEditDelegate::paintItem(
        QPainter* pPainter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    // Paint the item the default way, i.e. ellipsis after horizontal overflow
    // and first linebreak
    QStyledItemDelegate::paint(pPainter, option, index);
}
