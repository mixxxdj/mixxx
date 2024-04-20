#include "library/tabledelegates/stardelegate.h"

#include <QTableView>

#include "library/starrating.h"
#include "library/tabledelegates/stareditor.h"
#include "library/tabledelegates/tableitemdelegate.h"
#include "moc_stardelegate.cpp"
#include "widget/wtracktableview.h"

StarDelegate::StarDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView),
          m_isOneCellInEditMode(false) {
    connect(pTableView, &QTableView::entered, this, &StarDelegate::cellEntered);
    connect(pTableView, &QTableView::viewportEntered, this, &StarDelegate::cursorNotOverAnyCell);

    auto pTrackTableView = qobject_cast<WTrackTableView*>(pTableView);
    if (pTrackTableView) {
        connect(pTrackTableView,
                &WTrackTableView::viewportLeaving,
                this,
                &StarDelegate::cursorNotOverAnyCell);
    }
}

void StarDelegate::paintItem(
        QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const {
    // Let the editor do the painting if this cell is currently being edited.
    // Note: if required, the focus border will be drawn by the editor.
    if (index == m_currentEditedCellIndex) {
        return;
    }

    paintItemBackground(painter, option, index);

    StarRating starRating = index.data().value<StarRating>();
    starRating.paint(painter, option.rect);
}

QSize StarDelegate::sizeHint(const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
    Q_UNUSED(option);
    StarRating starRating = index.data().value<StarRating>();
    return starRating.sizeHint();
}

QWidget* StarDelegate::createEditor(QWidget* parent,
                                    const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const {
    // Populate the correct colors based on the styling
    QStyleOptionViewItem newOption = option;
    initStyleOption(&newOption, index);

    StarEditor* editor =
            new StarEditor(parent, m_pTableView, index, newOption, m_focusBorderColor);
    connect(editor,
            &StarEditor::editingFinished,
            this,
            &StarDelegate::commitAndCloseEditor);
    return editor;
}

void StarDelegate::setEditorData(QWidget* editor,
                                 const QModelIndex& index) const {
    StarRating starRating = index.data().value<StarRating>();
    StarEditor* starEditor = qobject_cast<StarEditor*>(editor);
    starEditor->setStarRating(starRating);
}

void StarDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                const QModelIndex& index) const {
    StarEditor* starEditor = qobject_cast<StarEditor*>(editor);
    model->setData(index, QVariant::fromValue(starEditor->starRating()));
}

void StarDelegate::commitAndCloseEditor() {
    StarEditor* editor = qobject_cast<StarEditor*>(sender());
    emit commitData(editor);
    emit closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
}

void StarDelegate::cellEntered(const QModelIndex& index) {
    // This slot is called if the mouse pointer enters ANY cell on the
    // QTableView but the code should only be executed on a column with a
    // StarRating.
    if (index.data().canConvert<StarRating>()) {
        if (m_isOneCellInEditMode) {
            // Don't close other editors when hovering the stars cell!
            m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
        }
        m_pTableView->openPersistentEditor(index);
        m_isOneCellInEditMode = true;
        m_currentEditedCellIndex = index;
    } else if (m_isOneCellInEditMode) {
        m_isOneCellInEditMode = false;
        m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
        m_currentEditedCellIndex = QPersistentModelIndex();
    }
}

void StarDelegate::cursorNotOverAnyCell() {
    // Invoked when the mouse cursor is not over any specific cell,
    // or when the mouse cursor has left the table area
    if (m_isOneCellInEditMode) {
        m_isOneCellInEditMode = false;
        m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
        m_currentEditedCellIndex = QPersistentModelIndex();
    }
}
