#include "library/tabledelegates/stardelegate.h"

#include <QTableView>
#include <QTimer>

#include "library/starrating.h"
#include "library/tabledelegates/stareditor.h"
#include "library/tabledelegates/tableitemdelegate.h"
#include "moc_stardelegate.cpp"
#include "widget/wtracktableview.h"

StarDelegate::StarDelegate(QTableView* pTableView)
        : TableItemDelegate(pTableView),
          m_persistentEditorState(PersistentEditor_NotOpen) {
    connect(this, &QAbstractItemDelegate::closeEditor, this, &StarDelegate::closingEditor);
    connect(pTableView, &QTableView::entered, this, &StarDelegate::cellEntered);
    connect(pTableView, &QTableView::viewportEntered, this, &StarDelegate::cursorNotOverAnyCell);

    auto* pTrackTableView = qobject_cast<WTrackTableView*>(pTableView);
    if (pTrackTableView) {
        connect(pTrackTableView,
                &WTrackTableView::viewportLeaving,
                this,
                &StarDelegate::cursorNotOverAnyCell);
        connect(pTrackTableView,
                &WTrackTableView::editRequested,
                this,
                &StarDelegate::editRequested);
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

    StarEditor* editor = new StarEditor(parent,
            m_pTableView,
            index,
            newOption,
            m_persistentEditorState != PersistentEditor_Opening);

    editor->setObjectName("LibraryStarEditor");
    editor->ensurePolished();

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

void StarDelegate::closingEditor(QWidget* pWidget, QAbstractItemDelegate::EndEditHint) {

    auto* pEditor = qobject_cast<StarEditor*>(pWidget);
    VERIFY_OR_DEBUG_ASSERT(pEditor) {
        return;
    }

    restorePersistentRatingEditor(pEditor->getModelIndex());
}

void StarDelegate::editRequested(const QModelIndex& index,
        QAbstractItemView::EditTrigger trigger,
        QEvent*) {

    // This slot is called when an edit is requested for ANY cell on the
    // QTableView but the code should only be executed on a column with a
    // StarRating.
    if (trigger == QAbstractItemView::EditTrigger::EditKeyPressed &&
            m_persistentEditorState == PersistentEditor_Open &&
            index.data().canConvert<StarRating>() &&
            m_currentEditedCellIndex == index) {
        // Close the (implicit) persistent editor for the current cell,
        // so that a new explicit editor can be opened instead.
        closeCurrentPersistentRatingEditor(true);
    }
}

void StarDelegate::cellEntered(const QModelIndex& index) {
    // This slot is called if the mouse pointer enters ANY cell on the
    // QTableView but the code should only be executed on a column with a
    // StarRating.
    if (index.data().canConvert<StarRating>()) {
        openPersistentRatingEditor(index);
    } else {
        closeCurrentPersistentRatingEditor(false);
    }
}

void StarDelegate::cursorNotOverAnyCell() {
    // Invoked when the mouse cursor is not over any specific cell,
    // or when the mouse cursor has left the table area
    closeCurrentPersistentRatingEditor(false);
}

void StarDelegate::openPersistentRatingEditor(const QModelIndex& index) {
    // Qt6: Check whether a non-persistent editor exists at index.
    // QTableView::closePersistentEditor() would also close
    // a non-persistent editor, so we have to make sure to
    // not call it if an editor already exists.
    if (m_pTableView->indexWidget(index)) {
        return;
    }

    // Close the previously open persistent rating editor
    if (m_persistentEditorState == PersistentEditor_Open) {
        // Don't close other editors when hovering the stars cell!
        m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
    }

    m_persistentEditorState = PersistentEditor_NotOpen;
    m_persistentEditorState = PersistentEditor_Opening;
    m_pTableView->openPersistentEditor(index);
    m_persistentEditorState = PersistentEditor_Open;
    m_currentEditedCellIndex = index;
}

void StarDelegate::closeCurrentPersistentRatingEditor(bool rememberForRestore) {
    if (m_persistentEditorState == PersistentEditor_Open) {
        m_pTableView->closePersistentEditor(m_currentEditedCellIndex);
    }

    if (rememberForRestore &&
            (m_persistentEditorState == PersistentEditor_Open ||
                    m_persistentEditorState == PersistentEditor_ShouldRestore)) {
        // Keep m_currentEditedCellIndex so the persistent editor
        // can be restored when the currently active explicit editor
        // is closed.
        m_persistentEditorState = PersistentEditor_ShouldRestore;
    } else {
        m_persistentEditorState = PersistentEditor_NotOpen;
        m_currentEditedCellIndex = QPersistentModelIndex();
    }
}

void StarDelegate::restorePersistentRatingEditor(const QModelIndex& index) {
    if (m_persistentEditorState == PersistentEditor_ShouldRestore &&
            index.isValid() && m_currentEditedCellIndex == index) {
        // Avoid race conditions by deferring the restore until
        // we have returned to the event loop, and the closing of
        // the current editor has been finished. Otherwise,
        // the internal state of QAbstractItemView may become
        // inconsistent.
        m_persistentEditorState = PersistentEditor_InDeferredRestore;
        QTimer::singleShot(0, this, &StarDelegate::restorePersistentRatingEditorNow);
    }
}

void StarDelegate::restorePersistentRatingEditorNow() {
    if (m_persistentEditorState == PersistentEditor_InDeferredRestore &&
            m_currentEditedCellIndex.isValid()) {
        openPersistentRatingEditor(m_currentEditedCellIndex);
    }
}
