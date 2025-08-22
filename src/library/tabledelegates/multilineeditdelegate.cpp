#include "library/tabledelegates/multilineeditdelegate.h"

#include <QAbstractTextDocumentLayout>
#include <QScrollBar>
#include <QTableView>
#include <cmath>

#include "moc_multilineeditdelegate.cpp"

MultiLineEditor::MultiLineEditor(QWidget* pParent,
        QTableView* pTableView,
        const QModelIndex& index)
        : QPlainTextEdit(pParent),
          m_pTableView(pTableView),
          m_index(index),
          m_fontHeight(m_pTableView->fontMetrics().height()) {
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    // Disable line wrap for a predictable view (like QLineEdit). Horizontal
    // scrollbars will show up automatically.
    setLineWrapMode(QPlainTextEdit::NoWrap);
    // Remove content offset, most notable with one-liners
    document()->setDocumentMargin(0);
    setContentsMargins(0, 0, 0, 0);
    setCenterOnScroll(false);
    // Add event filter to catch right-clicks and key presses, see eventFilter()
    installEventFilter(this);

    // Explicitly set the font, otherwise the font might be reset
    // after first edit.
    const auto font = m_pTableView->font();
    setFont(font);

    // Adjust size to fit content and maybe shift vertically to fit into the
    // library view. documentSizeChanged() is emitted when the layout has been
    // adjusted according to text changes, incl. initial fill.
    auto* pDocLayout = document()->documentLayout();
    connect(pDocLayout,
            &QAbstractTextDocumentLayout::documentSizeChanged,
            this,
            &MultiLineEditor::adjustSize);

    // Also adjust size if the table is scrolled: maybe we can now expand horizontally
    // to show all content, or need to shift the editor vertically.
    connect(m_pTableView->horizontalScrollBar(),
            &QScrollBar::valueChanged,
            this,
            [this]() {
                adjustSize(document()->size());
            });
    connect(m_pTableView->verticalScrollBar(),
            &QScrollBar::valueChanged,
            this,
            [this]() {
                adjustSize(document()->size());
            });
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
        }
    }
    return QPlainTextEdit::eventFilter(obj, event);
}

// The editor can grow vertically and to the right to show all content and avoid
// scrollbars as long as possible. It may be shifted up/down if it would exceed
// the table view. Size and position are adjusted if the table view is scrolled.
// The only constraints are:
// * minimum rectangle is the index rectangle
// * it's Left edge is always the left edge of the index
// * the editor must always include the index rectangle, hence it may be scrolled
//   out of view along with the table content (it remains open)
void MultiLineEditor::adjustSize(const QSizeF size) {
    // Compared to QTextEdit, size.height() is the paragraph/line count (Qt speak: blocks)
    int lines = static_cast<int>(size.height());
    int docW = static_cast<int>(std::ceil(size.width()));
    // Note: frameWidth() doesn't return the actual frame width set by qss.
    // Assume 1px like in official skins
    int frameW = 1;
    const QRect indexRect = m_pTableView->visualRect(m_index);
    const QRect tableRect = m_pTableView->viewport()->rect();

    // Remove the scrollbars if content is just one line to emulate QLineEdit
    // appearance, else enable auto mode.
    Qt::ScrollBarPolicy pol(lines > 1 ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(pol);
    setHorizontalScrollBarPolicy(pol);

    // If we have more than one line, add extra bottom margin so the horizontal
    // scrollbar that may pop up will not obstruct the last line (which also avoids the
    // vertical scrollbar as long as possible)
    lines += lines > 1 ? 1 : 0;

    // Height
    // Don't let the editor shrink smaller than the height of the table index.
    int optH = lines * m_fontHeight + frameW * 2;
    int newH = std::max(optH, indexRect.height());
    // If it's just one line center the text vertically like in QLineEdit.
    int vOffset = (indexRect.height() - optH) / 2;
    if (lines == 1 && vOffset > 0) {
        setStyleSheet(QStringLiteral(
                "QPlainTextEdit {"
                "padding-top: %1px;"
                "}")
                              .arg(vOffset));
    } else { // Reset if lines were added
        setStyleSheet(QStringLiteral(
                "QPlainTextEdit {"
                "padding-top: 0px;"
                "}"));
    }
    // Avoid clipping if the editor overflows the table view at the bottom
    int newY = indexRect.y();
    bool vScrollbarVisible = false;
    int tableH = tableRect.height();
    if (newY + newH > tableH) {
        // First, try to shift the editor up
        if (newY >= 0) {
            newY = std::max(0, tableH - newH); // Keep top edge inside table view
        }
    }
    if (newY + newH > tableH) {
        // If that doesn't suffice reduce height
        newH = tableH - newY;
        vScrollbarVisible = true;
    }
    // The editor must always include the index rectangle
    if (newY + newH < indexRect.bottom()) {
        newY = indexRect.bottom() - newH;
    }

    // Width
    // Let the editor expand horizontally like QLineEdit (max. to right table edge,
    // to not scroll the table horizontally if the cursor is moved), but don't
    // shrink smaller than the index width.
    int vScrollW = vScrollbarVisible ? verticalScrollBar()->width() : 0;
    // TODO For some reason the width isn't enough for all content, h-scrollbars show up
    // BUT after v- or h-scroll, the document is suddenly 8px wider, no idea where those
    // are coming from. Adding these magic 8px fixes it.
    int optW = docW + frameW * 2 + 8 + vScrollW;
    int newW = std::max(indexRect.width(), optW);
    int tableW = tableRect.width();
    if (indexRect.x() + newW > tableW) {
        newW = std::max(indexRect.width(), tableW - indexRect.x());
    }

#ifdef __APPLE__
    // macOS' transient (table view) scrollbars are drawn inside the table, hence
    // the cover content. Don't let them cover the editor, instead shrink or shift
    // it as required.
    int tableVScrollW = m_pTableView->verticalScrollBar()->width();
    if (tableVScrollW > 0 && (indexRect.x() + newW > tableW - tableVScrollW)) {
        newW -= tableVScrollW;
    }
    int tableHscrollW = m_pTableView->horizontalScrollBar()->height();
    if (tableHscrollW > 0 && newY + newH > tableH - tableHscrollW) {
        if (newY >= tableHscrollW) {
            // shift it up
            newY -= tableHscrollW;
        } else {
            // reduce height
            newH -= tableHscrollW;
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
    Q_UNUSED(option);
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
