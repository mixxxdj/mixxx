#include "widget/wlibrarysidebar.h"

#include <QFileInfo>
#include <QHeaderView>
#include <QMimeData>
#include <QUrl>
#include <QtDebug>

#include "library/sidebarmodel.h"
#include "moc_wlibrarysidebar.cpp"
#include "util/dnd.h"

const int expand_time = 250;

WLibrarySidebar::WLibrarySidebar(QWidget* parent)
        : QTreeView(parent),
          WBaseWidget(this) {
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

void WLibrarySidebar::contextMenuEvent(QContextMenuEvent *event) {
    //if (event->state() & Qt::RightButton) { //Dis shiz don werk on windowze
    QModelIndex clickedItem = indexAt(event->pos());
    emit rightClicked(event->globalPos(), clickedItem);
    //}
}

// Drag enter event, happens when a dragged item enters the track sources view
void WLibrarySidebar::dragEnterEvent(QDragEnterEvent * event) {
    qDebug() << "WLibrarySidebar::dragEnterEvent" << event->mimeData()->formats();
    if (event->mimeData()->hasUrls()) {
        // We don't have a way to ask the LibraryFeatures whether to accept a
        // drag so for now we accept all drags. Since almost every
        // LibraryFeature accepts all files in the drop and accepts playlist
        // drops we default to those flags to DragAndDropHelper.
        QList<mixxx::FileInfo> fileInfos = DragAndDropHelper::supportedTracksFromUrls(
                event->mimeData()->urls(), false, true);
        if (!fileInfos.isEmpty()) {
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
    //QTreeView::dragEnterEvent(event);
}

// Drag move event, happens when a dragged item hovers over the track sources view...
void WLibrarySidebar::dragMoveEvent(QDragMoveEvent * event) {
    //qDebug() << "dragMoveEvent" << event->mimeData()->formats();
    // Start a timer to auto-expand sections the user hovers on.
    QPoint pos = event->pos();
    QModelIndex index = indexAt(pos);
    if (m_hoverIndex != index) {
        m_expandTimer.stop();
        m_hoverIndex = index;
        m_expandTimer.start(expand_time, this);
    }
    // This has to be here instead of after, otherwise all drags will be
    // rejected -- rryan 3/2011
    QTreeView::dragMoveEvent(event);
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        // Drag and drop within this widget
        if ((event->source() == this)
                && (event->possibleActions() & Qt::MoveAction)) {
            // Do nothing.
            event->ignore();
        } else {
            SidebarModel* sidebarModel = qobject_cast<SidebarModel*>(model());
            bool accepted = true;
            if (sidebarModel) {
                accepted = false;
                for (const QUrl& url : urls) {
                    QModelIndex destIndex = indexAt(event->pos());
                    if (sidebarModel->dragMoveAccept(destIndex, url)) {
                        // We only need one URL to be valid for us
                        // to accept the whole drag...
                        // consider we have a long list of valid files, checking all will
                        // take a lot of time that stales Mixxx and this makes the drop feature useless
                        // Eg. you may have tried to drag two MP3's and an EXE, the drop is accepted here,
                        // but the EXE is sorted out later after dropping
                        accepted = true;
                        break;
                    }
                }
            }
            if (accepted) {
                event->acceptProposedAction();
            } else {
                event->ignore();
            }
        }
    } else {
        event->ignore();
    }
}

void WLibrarySidebar::timerEvent(QTimerEvent *event) {
    if (event->timerId() == m_expandTimer.timerId()) {
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
    QTreeView::timerEvent(event);
}

// Drag-and-drop "drop" event. Occurs when something is dropped onto the track sources view
void WLibrarySidebar::dropEvent(QDropEvent * event) {
    if (event->mimeData()->hasUrls()) {
        // Drag and drop within this widget
        if ((event->source() == this)
                && (event->possibleActions() & Qt::MoveAction)) {
            // Do nothing.
            event->ignore();
        } else {
            //Reset the selected items (if you had anything highlighted, it clears it)
            //this->selectionModel()->clear();
            //Drag-and-drop from an external application or the track table widget
            //eg. dragging a track from Windows Explorer onto the sidebar
            SidebarModel* sidebarModel = qobject_cast<SidebarModel*>(model());
            if (sidebarModel) {
                QModelIndex destIndex = indexAt(event->pos());
                // event->source() will return NULL if something is dropped from
                // a different application
                const QList<QUrl> urls = event->mimeData()->urls();
                if (sidebarModel->dropAccept(destIndex, urls, event->source())) {
                    event->acceptProposedAction();
                } else {
                    event->ignore();
                }
            }
        }
        //emit trackDropped(name);
        //repaintEverything();
    } else {
        event->ignore();
    }
}


void WLibrarySidebar::toggleSelectedItem() {
    QModelIndexList selectedIndices = this->selectionModel()->selectedRows();
    if (selectedIndices.size() > 0) {
        QModelIndex index = selectedIndices.at(0);
        // Activate the item so its content shows in the main library.
        emit pressed(index);
        // Expand or collapse the item as necessary.
        setExpanded(index, !isExpanded(index));
    }
}

bool WLibrarySidebar::isLeafNodeSelected() {
    QModelIndexList selectedIndices = this->selectionModel()->selectedRows();
    if (selectedIndices.size() > 0) {
        QModelIndex index = selectedIndices.at(0);
        if(!index.model()->hasChildren(index)) {
            return true;
        }
        const SidebarModel* sidebarModel = qobject_cast<const SidebarModel*>(index.model());
        if (sidebarModel) {
            return sidebarModel->hasTrackTable(index);
        }
    }
    return false;
}

void WLibrarySidebar::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return) {
        toggleSelectedItem();
        return;
    } else if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up) {
        // Let the tree view move up and down for us.
        QTreeView::keyPressEvent(event);

        // But force the index to be activated/clicked after the selection
        // changes. (Saves you from having to push "enter" after changing the
        // selection.)
        QModelIndexList selectedIndices = this->selectionModel()->selectedRows();

        //Note: have to get the selected indices _after_ QTreeView::keyPressEvent()
        if (selectedIndices.size() > 0) {
            QModelIndex index = selectedIndices.at(0);
            emit pressed(index);
        }
        return;
    //} else if (event->key() == Qt::Key_Enter && (event->modifiers() & Qt::AltModifier)) {
    //    // encoder click via "GoToItem"
    //    qDebug() << "GoToItem";
    //    TODO(xxx) decide what todo here instead of in librarycontrol
    }

    // Fall through to default handler.
    QTreeView::keyPressEvent(event);
}

void WLibrarySidebar::selectIndex(const QModelIndex& index) {
    auto* pModel = new QItemSelectionModel(model());
    pModel->select(index, QItemSelectionModel::Select);
    if (selectionModel()) {
        selectionModel()->deleteLater();
    }
    setSelectionModel(pModel);
    if (index.parent().isValid()) {
        expand(index.parent());
    }
    scrollTo(index);
}

/// Selects a child index from a feature and ensures visibility
void WLibrarySidebar::selectChildIndex(const QModelIndex& index, bool selectItem) {
    SidebarModel* sidebarModel = qobject_cast<SidebarModel*>(model());
    VERIFY_OR_DEBUG_ASSERT(sidebarModel) {
        qDebug() << "model() is not SidebarModel";
        return;
    }
    QModelIndex translated = sidebarModel->translateChildIndex(index);

    if (selectItem) {
        auto* pModel = new QItemSelectionModel(sidebarModel);
        pModel->select(translated, QItemSelectionModel::Select);
        if (selectionModel()) {
            selectionModel()->deleteLater();
        }
        setSelectionModel(pModel);
    }

    QModelIndex parentIndex = translated.parent();
    while (parentIndex.isValid()) {
        expand(parentIndex);
        parentIndex = parentIndex.parent();
    }
    scrollTo(translated, EnsureVisible);
}

bool WLibrarySidebar::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QTreeView::event(pEvent);
}

void WLibrarySidebar::slotSetFont(const QFont& font) {
    setFont(font);
    // Resize the feature icons to be a bit taller than the label's capital
    int iconSize = static_cast<int>(QFontMetrics(font).height() * 0.8);
    setIconSize(QSize(iconSize, iconSize));
}
