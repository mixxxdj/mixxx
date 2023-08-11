#include "library/multilineeditdelegate.h"

#include <QAbstractTextDocumentLayout>
#include <QDebug>
#include <QScrollBar>
#include <cmath>

#include "moc_multilineeditdelegate.cpp"

MultiLineEditor::MultiLineEditor(QWidget* pParent,
        QTableView* pTableView,
        const QModelIndex& index)
        : QPlainTextEdit(pParent),
          m_pTableView(pTableView),
          m_index(index) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    // Disable line wrap for a predictable view (like QLineEdit). Horizontal
    // scrollbars will show up automatically.
    setLineWrapMode(QPlainTextEdit::NoWrap);
    // Remove content offset, most notable with one-liners
    document()->setDocumentMargin(0);
    // Paint the entire rectangle, i.e. expand document background in order to
    // cover all underlying index text. Seems to be required for one-liners on macOS.
    setBackgroundVisible(true);
    // Add event filter to catch right-clicks and key presses, see eventFilter()
    installEventFilter(this);

    // Adjust height to fit content and maybe shift vertically to fit into the
    // library view. documentSizeChanged() is emitted when the text waschanged,
    // incl. initial fill.
    auto* pDocLayout = document()->documentLayout();
    connect(pDocLayout,
            &QAbstractTextDocumentLayout::documentSizeChanged,
            this,
            &MultiLineEditor::adjustSize);
};

bool MultiLineEditor::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::MouseButtonPress) {
        // Work around a strange quirk: right-clicks outside the rectangle of the
        // underlying table index are not triggering the document context menu.
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

void MultiLineEditor::adjustSize(const QSizeF size) {
    // Compared to QTextEdit, size.height() is the paragraph/line count (Qt speak: blocks)
    int lines = static_cast<int>(size.height());
    // Remove the scrollbars if content is just one line to emulate QLineEdit
    // appearance, else enable auto mode.
    Qt::ScrollBarPolicy pol(lines > 1 ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(pol);
    setHorizontalScrollBarPolicy(pol);

    // If we have more than one line, add extra bottom margin so the horizontal
    // scrollbar that may pop up will not obstruct the last line (which also avoids the
    // vertical scrollbar as long as possible)
    lines += lines > 1 ? 1 : 0;

    // Calculate the editor height /////////////////////////////////////////////
    QFontMetrics fm(document()->defaultFont());
    // Don't let the editor shrink smaller than the height of the table index.
    const QRect indexRect = m_pTableView->visualRect(m_index);
    int txtH = lines * fm.height();
    int newH = std::max(txtH, indexRect.height());
    // If it's just one line center the text vertically like in QLineEdit.
    int diffH = (indexRect.height() - txtH - frameWidth() * 2) / 2;
    if (lines == 1 && diffH > 0) {
        setContentsMargins(0, diffH, 0, diffH); // left/right > 0 are not applied
    } else { // Reset if lines were added
        setContentsMargins(0, 0, 0, 0);
    }
    // Limit editor to visible table height
    QRect tableRect = m_pTableView->viewport()->rect();
    int tableH = tableRect.height();
    newH = std::min(newH, tableH);
    // If the editor overflows the table view at the bottom, move it up so it's
    // not clipped. No need to care about y < 0 or y > (table height - line height)
    // since the table already ensures visibility when the index is selected
    // and manual scrolling should simply move the editor with the table.
    int newY = indexRect.y();
    if ((newY + newH) > tableH) {
        newY = tableH - newH;
    }

    // Calculate the editor width //////////////////////////////////////////////
    // Let the editor expand horizontally like QLineEdit, limit to table width.
    auto cm = contentsMargins();
    int newW = std::max(indexRect.width(),
            static_cast<int>(std::ceil(size.width())) + frameWidth() * 2 + cm.left() + cm.right());
    // Also limit width so scrollbars are visible and table is not scrolled if
    // cursor is moved horizontally.
    int tableW = tableRect.width();
    if (indexRect.x() + newW > tableW) {
        newW = tableW - indexRect.x();
    }

#ifdef __APPLE__
    // Don't let table view scrollbars overlap the editor, shrink or shift as required
    int scrollW = m_pTableView->verticalScrollBar()->width();
    if (scrollW > 0 && (indexRect.x() + newW > tableW - scrollW)) {
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

    setGeometry(QRect(indexRect.x(), newY, newW, newH));
}

MultiLineEditDelegate::MultiLineEditDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView) {
}

QWidget* MultiLineEditDelegate::createEditor(QWidget* pParent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    Q_UNUSED(index);
    auto* pEditor = new MultiLineEditor(pParent, m_pTableView, index);
    // Also emitted when pressing Return key, see MultiLineEditor::keyPressEvent()
    connect(pEditor,
            &MultiLineEditor::editingFinished,
            this,
            &MultiLineEditDelegate::commitAndCloseEditor);
    return pEditor;
}

void MultiLineEditDelegate::commitAndCloseEditor() {
    MultiLineEditor* pEditor = qobject_cast<MultiLineEditor*>(sender());
    emit commitData(pEditor);
    emit closeEditor(pEditor);
}
