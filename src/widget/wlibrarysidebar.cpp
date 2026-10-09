#include "widget/wlibrarysidebar.h"

#include <QHeaderView>
#include <QUrl>
#include <QtDebug>

#include "library/library_prefs.h"
#include "library/sidebarmodel.h"
#include "moc_wlibrarysidebar.cpp"
#include "util/defs.h"
#include "util/dnd.h"

WLibrarySidebar::WLibrarySidebar(QWidget* parent)
        : QTreeView(parent),
          WBaseWidget(this),
          m_hoverExpandDelay(mixxx::library::prefs::kSidebarHoverExpandDelayDefault) {
    qRegisterMetaType<FocusWidget>("FocusWidget");
    //Set some properties
    setHeaderHidden(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    //Drag and drop setup
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragDropOverwriteMode(true);
    setDropIndicatorShown(true);
    setAutoScroll(true);
    setAutoExpandDelay(m_hoverExpandDelay);
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

void WLibrarySidebar::setSourceOfCurrentDragDropEvent(QObject* pSource) {
    // pEvent->source() will be NULL if something is dropped
    // from a different application. This knowledge is used
    // inside the LibraryFeature implementations.
    SidebarModel* pSidebarModel = qobject_cast<SidebarModel*>(model());
    if (pSidebarModel) {
        pSidebarModel->setSourceOfCurrentDragDropEvent(pSource);
    }
}

/// Drag enter event, happens when a dragged item enters the track sources view
void WLibrarySidebar::dragEnterEvent(QDragEnterEvent* pEvent) {
    qDebug() << "WLibrarySidebar::dragEnterEvent" << pEvent->mimeData()->formats();
    toggleDragHoverPropertyAndUpdateStyle(true);

    // QTreeView::dragEnterEvent will, through some indirection,
    // call SidebarModel::mimeTypes() and use it to decide whether
    // we could potentially support the drag data at all. In practice,
    // this checks whether the drag data contains a list of URLs.
    //
    // As documented in the Qt source code, the actual check whether any
    // of the URLs are actually valid/supported is deferred until the
    // dragMoveEvent (see below).
    //
    // Note: pEvent->source() will be NULL if something is dropped
    // from a different application. This knowledge is used
    // inside the LibraryFeature implementations.
    setSourceOfCurrentDragDropEvent(pEvent->source());
    QTreeView::dragEnterEvent(pEvent);
    setSourceOfCurrentDragDropEvent(nullptr);

    if (pEvent->isAccepted()) {
        pEvent->acceptProposedAction();
    }
}

/// Drag leave event, happens when the dragged item leaves the track sources view
/// or when the drag is aborted through Escape or other means.
void WLibrarySidebar::dragLeaveEvent(QDragLeaveEvent* pEvent) {
    // qDebug() << "WLibrarySidebar::dragLeaveEvent";
    m_autoExpandIndex = QModelIndex();
    toggleDragHoverPropertyAndUpdateStyle(false);

    QTreeView::dragLeaveEvent(pEvent);
}

/// Drag move event, happens when a dragged item hovers over the track sources view...
void WLibrarySidebar::dragMoveEvent(QDragMoveEvent* pEvent) {
    // qDebug() << "WLibrarySidebar::dragMoveEvent" << pEvent->mimeData()->formats();

    // QTreeView::dragMoveEvent will, through some indirection,
    // call SidebarModel::canDropMimeData, which will call one of either
    // LibraryFeature::dragMoveAccept or LibraryFeature::dragMoveAcceptChild,
    // depending on the item over which the drag event occurred.
    //
    // This is where the LibraryFeature subclasses check whether any of
    // actual data being dragged is supported, e.g. whether it is
    // a list of valid track URLs.
    //
    // Note: We go through QTreeView/QAbstractItemView here, instead of
    // directly calling SidebarModel, to retain other useful features
    // from the base class, like e.g. auto-scroll behavior when the mouse
    // cursor reaches the boundaries of the tree view.
    //
    // Note: pEvent->source() will be NULL if something is dropped
    // from a different application. This knowledge is used
    // inside the LibraryFeature implementations.

    // ========================================================================
    // Fix autoExpand timer reset behavior (workaround for bug in Qt framework)
    //
    // Starting with at least Qt 5.0.0 (released in 2011) and still present
    // in current versions of Qt (Qt 6.8.0 at the time of this commit), there
    // is a bug in the implementation of QTreeView::dragMoveEvent and autoExpandDelay:
    //
    // QT BUG DESCRIPTION
    //
    // Instead of resetting the delay timer whenever the mouse moves to a
    // new item, it is reset on every little mouse movement, which makes
    // autoExpand useless e.g. on laptop touchpads.
    //
    // OUR WORKAROUND
    //
    // Only reset the delay timer whenever the mouse has moved to a new item,
    // by bypassing QTreeView::dragMoveEvent() and directly calling
    // QAbstractItemView::dragMoveEvent() instead unless the mouse
    // has moved to a new item.
    // ========================================================================
    const QPoint pos = pEvent->position().toPoint();
    const QModelIndex index = indexAt(pos);

    if (m_autoExpandIndex != index) {
        m_autoExpandIndex = index;
        // QTreeView::dragMoveEvent just restarts the autoExpand timer
        // and then calls QAbstractItemView::dragMoveEvent
        setSourceOfCurrentDragDropEvent(pEvent->source());
        QTreeView::dragMoveEvent(pEvent);
        setSourceOfCurrentDragDropEvent(nullptr);
    } else {
        // Skip resetting the autoExpand timer (see above)
        // because we are still hovering over the same item
        setSourceOfCurrentDragDropEvent(pEvent->source());
        QAbstractItemView::dragMoveEvent(pEvent);
        setSourceOfCurrentDragDropEvent(nullptr);
    }
}

// Drag-and-drop "drop" event. Occurs when something is dropped onto the track sources view
void WLibrarySidebar::dropEvent(QDropEvent* pEvent) {
    // qDebug() << "WLibrarySidebar::dropEvent";
    m_autoExpandIndex = QModelIndex();
    toggleDragHoverPropertyAndUpdateStyle(false);

    // QTreeView::dropEvent will, through some indirection, call
    // SidebarModel::dropMimeData, which will call one of either
    // LibraryFeature::dropAccept or LibraryFeature::dropAcceptChild,
    // depending on where the drop occurred.
    //
    // Note: We go through QTreeView here instead of directly calling
    // SidebarModel to retain other useful features from the base class,
    // like e.g. auto-scroll behavior when the mouse cursor reaches
    // the boundaries of the tree view.
    //
    // Note: pEvent->source() will be NULL if something is dropped
    // from a different application. This knowledge is used
    // inside the LibraryFeature implementations.
    setSourceOfCurrentDragDropEvent(pEvent->source());
    QTreeView::dropEvent(pEvent);
    setSourceOfCurrentDragDropEvent(nullptr);
}

void WLibrarySidebar::toggleDragHoverPropertyAndUpdateStyle(bool enabled) {
    // Set a custom QWidget property that allows to style drag-hovered items.
    // WLibrarySidebar[dragHover="true"]::item:hover {
    //   border: 1px solid white;
    // }
    // Then force-refresh the style.
    setProperty("dragHover", enabled);
    style()->unpolish(this);
    style()->polish(this);
    update();
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

void WLibrarySidebar::setChildIndexExpanded(const QModelIndex& index, bool expand) {
    // qDebug() << "WLibrarySidebar::setChildIndexExpanded" << index << expand;
    QModelIndex selIndex = selectedIndex();
    if (!selIndex.isValid()) {
        return;
    }
    SidebarModel* sidebarModel = qobject_cast<SidebarModel*>(model());
    VERIFY_OR_DEBUG_ASSERT(sidebarModel) {
        // qDebug() << " >> model() is not SidebarModel";
        return;
    }
    QModelIndex translated = sidebarModel->translateChildIndex(index);
    if (!translated.isValid()) {
        // qDebug() << " >> index can't be translated";
        return;
    }
    setExpanded(translated, expand);
}

bool WLibrarySidebar::isChildIndexExpanded(const QModelIndex& index) {
    // qDebug() << "WLibrarySidebar::isChildIndexExpanded" << index;
    QModelIndex selIndex = selectedIndex();
    if (!selIndex.isValid()) {
        return false;
    }
    SidebarModel* sidebarModel = qobject_cast<SidebarModel*>(model());
    VERIFY_OR_DEBUG_ASSERT(sidebarModel) {
        // qDebug() << " >> model() is not SidebarModel";
        return false;
    }
    QModelIndex translated = sidebarModel->translateChildIndex(index);
    if (!translated.isValid()) {
        // qDebug() << " >> index can't be translated";
        return false;
    }
    return isExpanded(translated);
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

void WLibrarySidebar::slotSetExpandOnHoverDelay(int delay) {
    m_hoverExpandDelay = delay;
    setAutoExpandDelay(m_hoverExpandDelay);
}
