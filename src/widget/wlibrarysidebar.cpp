#include "widget/wlibrarysidebar.h"

#include <QHeaderView>
#include <QUrl>
#include <QtDebug>

#include "library/sidebarmodel.h"
#include "moc_wlibrarysidebar.cpp"
#include "util/defs.h"
#include "util/dnd.h"

constexpr int expand_time = 250;

WLibrarySidebar::WLibrarySidebar(QWidget* parent)
        : QTreeView(parent),
          WBaseWidget(this),
          m_lastDragMoveAccepted(false) {
    qRegisterMetaType<FocusWidget>("FocusWidget");
    //Set some properties
    setHeaderHidden(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    //Drag and drop setup
    setDragEnabled(false);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(true);
    setAcceptDrops(true);
    setAutoScroll(true);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    header()->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
}

void WLibrarySidebar::contextMenuEvent(QContextMenuEvent* pEvent) {
    // if (pEvent->state() & Qt::RightButton) { //Dis shiz don werk on windowze
    QModelIndex clickedIndex = indexAt(pEvent->pos());
    if (!clickedIndex.isValid()) {
        return;
    }
    // Use this instead of setCurrentIndex() to keep current selection
    selectionModel()->setCurrentIndex(clickedIndex, QItemSelectionModel::NoUpdate);
    pEvent->accept();
    emit rightClicked(pEvent->globalPos(), clickedIndex);
    //}
}

/// Drag enter event, happens when a dragged item enters the track sources view
void WLibrarySidebar::dragEnterEvent(QDragEnterEvent* pEvent) {
    qDebug() << "WLibrarySidebar::dragEnterEvent" << pEvent->mimeData()->formats();
    resetHoverIndexAndDragMoveResult();
    if (pEvent->mimeData()->hasUrls()) {
        // We don't have a way to ask the LibraryFeatures whether to accept a
        // drag so for now we accept all drags. Since almost every
        // LibraryFeature accepts all files in the drop and accepts playlist
        // drops we default to those flags to DragAndDropHelper.
        // FIXME Unless the cursor is steady after entering the sidebar (which
        // is veryhard to achieve for humans) QDragEnterEvent is followed by one
        // or more QDragMoveEvent, so don't check here at all and rely on dragMove?
        if (DragAndDropHelper::urlsContainSupportedTrackFiles(pEvent->mimeData()->urls(), true)) {
            pEvent->acceptProposedAction();
            return;
        }
    }
    pEvent->ignore();
    // QTreeView::dragEnterEvent(pEvent);
}

/// Drag move event, happens when a dragged item hovers over the track sources view...
void WLibrarySidebar::dragMoveEvent(QDragMoveEvent* pEvent) {
    // qDebug() << "dragMoveEvent" << pEvent->mimeData()->formats();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPoint pos = pEvent->position().toPoint();
#else
    QPoint pos = pEvent->pos();
#endif
    const QModelIndex index = indexAt(pos);
    if (m_hoverIndex == index) {
        m_lastDragMoveAccepted ? pEvent->acceptProposedAction() : pEvent->ignore();
        return;
    }

    // Start a timer to auto-expand sections the user hovers on
    m_expandTimer.stop();
    m_hoverIndex = index;
    m_expandTimer.start(expand_time, this);

    // This has to be here instead of after, otherwise all drags will be
    // rejected -- rryan 3/2011
    QTreeView::dragMoveEvent(pEvent);
    if (!pEvent->mimeData()->hasUrls()) {
        pEvent->ignore();
        m_lastDragMoveAccepted = false;
        return;
    }

    const QList<QUrl> urls = pEvent->mimeData()->urls();
    // Drag and drop within this widget
    if ((pEvent->source() == this) && (pEvent->possibleActions() & Qt::MoveAction)) {
        // Do nothing.
        m_lastDragMoveAccepted = false;
        pEvent->ignore();
        return;
    }

    SidebarModel* pSidebarModel = qobject_cast<SidebarModel*>(model());
    VERIFY_OR_DEBUG_ASSERT(pSidebarModel) {
        m_lastDragMoveAccepted = false;
        pEvent->ignore();
        return;
    }
    if (pSidebarModel->dragMoveAccept(index, urls)) {
        m_lastDragMoveAccepted = true;
        pEvent->acceptProposedAction();
    } else {
        m_lastDragMoveAccepted = false;
        pEvent->ignore();
    }
}

void WLibrarySidebar::timerEvent(QTimerEvent* pEvent) {
    if (pEvent->timerId() == m_expandTimer.timerId()) {
        QPoint pos = viewport()->mapFromGlobal(QCursor::pos());
        if (viewport()->rect().contains(pos)) {
            QModelIndex index = indexAt(pos);
            if (m_hoverIndex == index) {
                setExpanded(index, !isExpanded(index));
            }
        }
        m_expandTimer.stop();
        return;
    }
    QTreeView::timerEvent(pEvent);
}

// Drag-and-drop "drop" event. Occurs when something is dropped onto the track sources view
void WLibrarySidebar::dropEvent(QDropEvent* pEvent) {
    resetHoverIndexAndDragMoveResult();
    if (!pEvent->mimeData()->hasUrls()) {
        pEvent->ignore();
        return;
    }
    // Drag and drop within this widget
    if ((pEvent->source() == this) && (pEvent->possibleActions() & Qt::MoveAction)) {
        // Do nothing.
        pEvent->ignore();
        return;
    }
    // Drag-and-drop from an external application (eg. a file manager) or the
    // track table widget onto the sidebar.
    // Reset the selected items (if you had anything highlighted, it clears it)
    // this->selectionModel()->clear();
    SidebarModel* pSidebarModel = qobject_cast<SidebarModel*>(model());
    VERIFY_OR_DEBUG_ASSERT(pSidebarModel) {
        pEvent->ignore();
        return;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPoint pos = pEvent->position().toPoint();
#else
    QPoint pos = pEvent->pos();
#endif

    const QModelIndex destIndex = indexAt(pos);
    // pEvent->source() will return NULL if something is dropped from
    // a different application
    const QList<QUrl> urls = pEvent->mimeData()->urls();
    if (pSidebarModel->dropAccept(destIndex, urls, pEvent->source())) {
        pEvent->acceptProposedAction();
    } else {
        pEvent->ignore();
    }
}

void WLibrarySidebar::resetHoverIndexAndDragMoveResult() {
    m_hoverIndex = QModelIndex();
    m_lastDragMoveAccepted = false;
}

void WLibrarySidebar::renameSelectedItem() {
    // Rename crate or playlist (internal, external, history)
    QModelIndex selIndex = selectedIndex();
    if (!selIndex.isValid()) {
        return;
    }
    emit renameItem(selIndex);
    return;
}

void WLibrarySidebar::toggleSelectedItem() {
    QModelIndex index = selectedIndex();
    if (index.isValid()) {
        // Activate the item so its content shows in the main library.
        emit clicked(index);
        // Expand or collapse the item as necessary.
        setExpanded(index, !isExpanded(index));
    }
}

bool WLibrarySidebar::isLeafNodeSelected() {
    QModelIndex index = selectedIndex();
    if (index.isValid()) {
        if(!index.model()->hasChildren(index)) {
            return true;
        }
        const SidebarModel* pSidebarModel = qobject_cast<const SidebarModel*>(index.model());
        if (pSidebarModel) {
            return pSidebarModel->hasTrackTable(index);
        }
    }
    return false;
}

bool WLibrarySidebar::isChildIndexSelected(const QModelIndex& index) {
    // qDebug() << "WLibrarySidebar::isChildIndexSelected" << index;
    QModelIndex selIndex = selectedIndex();
    if (!selIndex.isValid()) {
        return false;
    }
    SidebarModel* pSidebarModel = qobject_cast<SidebarModel*>(model());
    VERIFY_OR_DEBUG_ASSERT(pSidebarModel) {
        // qDebug() << " >> model() is not SidebarModel";
        return false;
    }
    QModelIndex translated = pSidebarModel->translateChildIndex(index);
    if (!translated.isValid()) {
        // qDebug() << " >> index can't be translated";
        return false;
    }
    return translated == selIndex;
}

bool WLibrarySidebar::isFeatureRootIndexSelected(LibraryFeature* pFeature) {
    // qDebug() << "WLibrarySidebar::isFeatureRootIndexSelected";
    QModelIndex selIndex = selectedIndex();
    if (!selIndex.isValid()) {
        return false;
    }
    SidebarModel* pSidebarModel = qobject_cast<SidebarModel*>(model());
    VERIFY_OR_DEBUG_ASSERT(pSidebarModel) {
        return false;
    }
    const QModelIndex rootIndex = pSidebarModel->getFeatureRootIndex(pFeature);
    return rootIndex == selIndex;
}

/// Invoked by actual keypresses (requires widget focus) and emulated keypresses
/// sent by LibraryControl
void WLibrarySidebar::keyPressEvent(QKeyEvent* pEvent) {
    // TODO(XXX) Should first keyEvent ensure previous item has focus? I.e. if the selected
    // item is not focused, require second press to perform the desired action.

    SidebarModel* pSidebarModel = qobject_cast<SidebarModel*>(model());
    QModelIndex selIndex = selectedIndex();
    if (pSidebarModel && selIndex.isValid() && pEvent->matches(QKeySequence::Paste)) {
        pSidebarModel->paste(selIndex);
        return;
    }

    focusSelectedIndex();

    switch (pEvent->key()) {
    case Qt::Key_Return:
        toggleSelectedItem();
        return;
    case Qt::Key_Down:
    case Qt::Key_Up:
    case Qt::Key_PageDown:
    case Qt::Key_PageUp:
    case Qt::Key_End:
    case Qt::Key_Home: {
        // Let the tree view move up and down for us.
        QTreeView::keyPressEvent(pEvent);
        // After the selection changed force-activate (click) the newly selected
        // item to save us from having to push "Enter".
        QModelIndex selIndex = selectedIndex();
        if (!selIndex.isValid()) {
            return;
        }
        // Ensure the new selection is visible even if it was already selected/
        // focused, like when the topmost item was selected but out of sight and
        // we pressed Up, Home or PageUp.
        scrollTo(selIndex);
        emit pressed(selIndex);
        return;
    }
    case Qt::Key_Right: {
        if (pEvent->modifiers() & Qt::ControlModifier) {
            emit setLibraryFocus(FocusWidget::TracksTable);
        } else {
            QTreeView::keyPressEvent(pEvent);
        }
        return;
    }
    case Qt::Key_Left: {
        // If an expanded item is selected let QTreeView collapse it
        QModelIndex selIndex = selectedIndex();
        if (!selIndex.isValid()) {
            return;
        }
        // collapse knot
        if (isExpanded(selIndex)) {
            QTreeView::keyPressEvent(pEvent);
            return;
        }
        // Else jump to its parent and activate it
        QModelIndex parentIndex = selIndex.parent();
        if (parentIndex.isValid()) {
            selectIndex(parentIndex);
            emit pressed(parentIndex);
        }
        return;
    }
    case Qt::Key_Escape:
        // Focus tracks table
        emit setLibraryFocus(FocusWidget::TracksTable);
        return;
    case kRenameSidebarItemShortcutKey: { // F2
        renameSelectedItem();
        return;
    }
    case kHideRemoveShortcutKey: { // Del (macOS: Cmd+Backspace)
        // Delete crate or playlist (internal, external, history)
        if (pEvent->modifiers() != kHideRemoveShortcutModifier) {
            return;
        }
        QModelIndex selIndex = selectedIndex();
        if (!selIndex.isValid()) {
            return;
        }
        emit deleteItem(selIndex);
        return;
    }
    default:
        QTreeView::keyPressEvent(pEvent);
    }
}

void WLibrarySidebar::mousePressEvent(QMouseEvent* pEvent) {
    // handle right click only in contextMenuEvent() to not select the clicked index
    if (pEvent->buttons().testFlag(Qt::RightButton)) {
        return;
    }
    QTreeView::mousePressEvent(pEvent);
}

void WLibrarySidebar::focusInEvent(QFocusEvent* pEvent) {
    // Clear the current index, i.e. remove the focus indicator
    selectionModel()->clearCurrentIndex();
    QTreeView::focusInEvent(pEvent);
}

void WLibrarySidebar::selectIndex(const QModelIndex& index, bool scrollToIndex) {
    // qDebug() << "WLibrarySidebar::selectIndex" << index << scrollToIndex;
    if (!index.isValid()) {
        return;
    }
    auto* pModel = new QItemSelectionModel(model());
    pModel->select(index, QItemSelectionModel::Select);
    if (selectionModel()) {
        selectionModel()->deleteLater();
    }
    if (index.parent().isValid()) {
        expand(index.parent());
    }
    setSelectionModel(pModel);
    if (!scrollToIndex) {
        // With auto-scroll enabled, setCurrentIndex() would scroll there.
        // Disable (and re-enable if we don't want to scroll, e.g. when selecting
        // AutoDJ from the menubar or during startup
        setAutoScroll(false);
    }
    setCurrentIndex(index);
    if (scrollToIndex) {
        scrollTo(index);
    } else {
        setAutoScroll(true);
    }
}

/// Selects a child index from a feature and ensures visibility
void WLibrarySidebar::selectChildIndex(const QModelIndex& index, bool selectItem) {
    SidebarModel* pSidebarModel = qobject_cast<SidebarModel*>(model());
    VERIFY_OR_DEBUG_ASSERT(pSidebarModel) {
        qDebug() << "model() is not SidebarModel";
        return;
    }
    QModelIndex translated = pSidebarModel->translateChildIndex(index);
    if (!translated.isValid()) {
        return;
    }

    if (selectItem) {
        auto* pModel = new QItemSelectionModel(pSidebarModel);
        pModel->select(translated, QItemSelectionModel::Select);
        if (selectionModel()) {
            selectionModel()->deleteLater();
        }
        setSelectionModel(pModel);
        setCurrentIndex(translated);
    }

    QModelIndex parentIndex = translated.parent();
    while (parentIndex.isValid()) {
        expand(parentIndex);
        parentIndex = parentIndex.parent();
    }
    scrollTo(translated, EnsureVisible);
}

QModelIndex WLibrarySidebar::selectedIndex() {
    QModelIndexList selectedIndices = selectionModel()->selectedRows();
    if (selectedIndices.isEmpty()) {
        return QModelIndex();
    }
    QModelIndex selIndex = selectedIndices.first();
    DEBUG_ASSERT(selIndex.isValid());
    return selIndex;
}

/// Refocus the selected item after right-click
void WLibrarySidebar::focusSelectedIndex() {
    // After the context menu was activated (and closed, with or without clicking
    // an action), the currentIndex is the right-clicked item.
    // If if the currentIndex is not selected, make the selection the currentIndex
    QModelIndex selIndex = selectedIndex();
    if (selIndex.isValid() && selIndex != selectionModel()->currentIndex()) {
        setCurrentIndex(selIndex);
    }
}

bool WLibrarySidebar::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    } else if (pEvent->type() == QEvent::LayoutRequest ||
            pEvent->type() == QEvent::Resize) {
        // Force-resize the header to expand the item's clickable area.
        //
        // Reason:
        // Currently, the sidebar header expands to the width of the widest item.
        // If the sidebar is wider than that, there's some space right next to
        // items that does not respond to clicks. This is somewhat frustration as
        // it is perceived inconsistent with the state when e.g. Playlist are
        // expanded and the entire 'Tracks' row responds to clicks.
        //
        // Desired appearance & behavior:
        // * full-width items (for click success)
        // * full item text (no elide)
        // * show horizontal scrollbars as needed
        //
        // Unfortunately, there's no combination of
        //   header()->setStretchLastSection(bool);
        //   header()->setSectionResizeMode(QHeaderView::ResizeMode);
        // to achieve that.
        //
        // Though we can listen to LayoutRequest and adjust the headers minimum
        // section size to viewport width (-1 for section separator?).
        // This event occurs after Show, Resize or model data change.
        header()->setMinimumSectionSize(viewport()->width() - 1);
    }
    return QTreeView::event(pEvent);
}

void WLibrarySidebar::slotSetFont(const QFont& font) {
    setFont(font);
    // Resize the feature icons to be a bit taller than the label's capital
    int iconSize = static_cast<int>(QFontMetrics(font).height() * 0.8);
    setIconSize(QSize(iconSize, iconSize));
}
