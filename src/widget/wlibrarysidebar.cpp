#include "widget/wlibrarysidebar.h"

#include <QDebug>
#include <QFileInfo>
#include <QHeaderView>
#include <QMimeData>
#include <QPainter>
#include <QUrl>

#include "library/treeitemmodel.h"
#include "util/dnd.h"

const int expand_time = 250;


WSidebarItemDelegate::WSidebarItemDelegate(QObject* parent)
        : QStyledItemDelegate(parent) {
    
}

void WSidebarItemDelegate::paint(QPainter* painter,
                                 const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const {
    bool divider = index.data(AbstractRole::RoleDivider).toBool();
    if (!divider) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    
    QString text = index.data().toString();
    QRect rect(option.rect);
    QFont font(option.font);
    font.setBold(true);
    // Set small padding left
    rect.setLeft(rect.left() + 3);
    
    QFontMetrics fontMetrics(font);
    QString elidedText = fontMetrics.elidedText(text, Qt::ElideRight, rect.width() - 3);
    
    // Draw the text
    painter->setPen(option.palette.color(QPalette::Text));
    painter->setFont(font);
    painter->drawText(rect, elidedText);
    
    // Draw line under text
    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
}


WLibrarySidebar::WLibrarySidebar(QWidget* parent)
        : QTreeView(parent),
          WBaseWidget(this) {
    setItemDelegate(new WSidebarItemDelegate(this));
    
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
    header()->setResizeMode(QHeaderView::Stretch);

    setFocusPolicy(Qt::StrongFocus);
}

void WLibrarySidebar::contextMenuEvent(QContextMenuEvent *event) {
    //if (event->state() & Qt::RightButton) { //Dis shiz don werk on windowze
    QModelIndex clickedItem = indexAt(event->pos());
    emit(rightClicked(event->globalPos(), clickedItem));
    //}
}

// Drag enter event, happens when a dragged item enters the track sources view
void WLibrarySidebar::dragEnterEvent(QDragEnterEvent * event) {
    //qDebug() << "WLibrarySidebar::dragEnterEvent" << event->mimeData()->formats();
    if (event->mimeData()->hasUrls()) {
        // We don't have a way to ask the LibraryFeatures whether to accept a
        // drag so for now we accept all drags. Since almost every
        // LibraryFeature accepts all files in the drop and accepts playlist
        // drops we default to those flags to DragAndDropHelper.
        QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(
                event->mimeData()->urls(), false, true);
        if (!files.isEmpty()) {
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
        QList<QUrl> urls(event->mimeData()->urls());
        // Drag and drop within this widget
        if ((event->source() == this)
                && (event->possibleActions() & Qt::MoveAction)) {
            // Do nothing.
            event->ignore();
        } else {
            bool accepted = true;
            TreeItemModel* treeModel = dynamic_cast<TreeItemModel*>(model());
            if (treeModel) {
                accepted = false;
                for (const QUrl& url : urls) {
                    QModelIndex destIndex = this->indexAt(event->pos());
                    if (treeModel->dragMoveAccept(destIndex, url)) {
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
            TreeItemModel* pTreeModel = dynamic_cast<TreeItemModel*>(model());
            if (pTreeModel) {
                QModelIndex destIndex = indexAt(event->pos());
                QList<QUrl> urls(event->mimeData()->urls());
                if (pTreeModel->dropAccept(destIndex, urls, event->source())) {
                    event->acceptProposedAction();
                } else {
                    event->ignore();
                }
            }
        }
        //emit(trackDropped(name));
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
        emit(pressed(index));
        // Expand or collapse the item as necessary.
        setExpanded(index, !isExpanded(index));
    }
}

void WLibrarySidebar::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        toggleSelectedItem();
        return;
    }

    // Fall through to default handler.
    QTreeView::keyPressEvent(event);
}

void WLibrarySidebar::selectIndex(const QModelIndex& index) {
    auto pModel = new QItemSelectionModel(model());
    pModel->select(index, QItemSelectionModel::Select);
    setSelectionModel(pModel);

    if (index.parent().isValid()) {
        expand(index.parent());
    }
    scrollTo(index);
}

bool WLibrarySidebar::event(QEvent* pEvent) {
    if (pEvent->type() == QEvent::ToolTip) {
        updateTooltip();
    }
    return QTreeView::event(pEvent);
}

void WLibrarySidebar::slotSetFont(const QFont& font) {
    setFont(font);
}
