#include "library/tabledelegates/stardelegate.h"

#include <QTableView>

#include "library/starrating.h"
#include "library/tabledelegates/stareditor.h"
#include "library/tabledelegates/tableitemdelegate.h"
#include "moc_stardelegate.cpp"
#include "util/assert.h"

StarDelegate::StarDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView),
          m_isOneCellInEditMode(false) {
    connect(pTableView, &QTableView::entered, this, &StarDelegate::cellEntered);
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

    // Set the color for the star polygons.
    // Only remaining very minor issue: if the editor is active while the played
    // state changes, the polygon color will not be updated (though that's a
    // confusing situation anyway since changed index (row) data will trigger a
    // paint event which resets the pending (unsaved) rating anyway.
    setTextColor(newOption, index);

    StarEditor* editor =
            new StarEditor(parent, m_pTableView, index, newOption, m_focusBorderColor);
    connect(editor,
            &StarEditor::editingFinished,
            this,
            &StarDelegate::commitAndCloseEditor);
    return editor;
}

void StarDelegate::setEditorData(QWidget* pEditor,
        const QModelIndex& index) const {
    StarEditor* pStarEditor = qobject_cast<StarEditor*>(pEditor);
    VERIFY_OR_DEBUG_ASSERT(pStarEditor) {
        return;
    }
    StarRating starRating = index.data().value<StarRating>();
    pStarEditor->setStarRating(starRating);
}

void StarDelegate::setModelData(QWidget* pEditor,
        QAbstractItemModel* pModel,
        const QModelIndex& index) const {
    StarEditor* pStarEditor = qobject_cast<StarEditor*>(pEditor);
    VERIFY_OR_DEBUG_ASSERT(pStarEditor) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(pModel) {
        return;
    }
    pModel->setData(index, QVariant::fromValue(pStarEditor->starRating()));
}

void StarDelegate::commitAndCloseEditor() {
    StarEditor* pEditor = qobject_cast<StarEditor*>(sender());
    VERIFY_OR_DEBUG_ASSERT(pEditor) {
        return;
    }
    emit commitData(pEditor);
    emit closeEditor(pEditor);
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
        m_currentEditedCellIndex = QModelIndex();
    }
}
