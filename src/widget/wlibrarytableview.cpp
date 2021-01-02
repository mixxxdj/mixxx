#include "widget/wlibrarytableview.h"

#include <QFocusEvent>
#include <QFontMetrics>
#include <QHeaderView>
#include <QMultiMap>
#include <QPalette>
#include <QScrollBar>

#include "library/trackmodel.h"
#include "moc_wlibrarytableview.cpp"
#include "util/math.h"
#include "widget/wskincolor.h"
#include "widget/wtracktableviewheader.h"
#include "widget/wwidget.h"

namespace {
// number of entries in the model cache
constexpr int kModelCacheSize = 1000;
} // namespace

WLibraryTableView::WLibraryTableView(QWidget* parent,
        UserSettingsPointer pConfig)
        : QTableView(parent),
          m_pConfig(pConfig),
          m_modelStateCache(kModelCacheSize) {
    // Setup properties for table

    // Editing starts when clicking on an already selected item.
    setEditTriggers(QAbstractItemView::SelectedClicked|QAbstractItemView::EditKeyPressed);

    //Enable selection by rows and extended selection (ctrl/shift click)
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setWordWrap(false);
    setShowGrid(false);
    setCornerButtonEnabled(false);
    setSortingEnabled(true);
    // Used by delegates (e.g. StarDelegate) to tell when the mouse enters a
    // cell.
    setMouseTracking(true);
    //Work around a Qt bug that lets you make your columns so wide you
    //can't reach the divider to make them small again.
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    verticalHeader()->hide();
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setAlternatingRowColors(true);

    connect(verticalScrollBar(),
            &QScrollBar::valueChanged,
            this,
            &WLibraryTableView::scrollValueChanged);

    setTabKeyNavigation(false);
}

WLibraryTableView::~WLibraryTableView() {
    m_modelStateCache.clear();
}


void WLibraryTableView::moveSelection(int delta) {
    QAbstractItemModel* pModel = model();

    if (pModel == nullptr) {
        return;
    }
    QItemSelectionModel* currentSelection = selectionModel();
    clearSelection();
    currentSelection->clearSelection();
    QModelIndex newIndex;
    while (delta != 0) {
        QModelIndex currentIndex = currentSelection->currentIndex();
        if (currentIndex.isValid()) {
            int row = currentIndex.row();
            if (delta > 0) {
                // i is positive, so we want to move the highlight down
                if (row + 1 < pModel->rowCount()) {
                    newIndex = currentIndex.sibling(row + 1, 0);
                } else {
                    // we wrap around at the end of the list so it is faster to get
                    // to the top of the list again
                    newIndex = currentIndex.sibling(0, 0);
                }
                delta--;
            } else {
                if (row - 1 >= 0) {
                    newIndex = currentIndex.sibling(row - 1, 0);
                } else {
                    newIndex = pModel->index(pModel->rowCount() - 1, 0);
                }
                delta++;
            }
        } else {
            // no selection, so select the first or last element depending on delta
            if (delta > 0) {
                newIndex = pModel->index(0, 0);
                delta--;
            } else {
                newIndex = pModel->index(pModel->rowCount() - 1, 0);
                delta++;
            }
        }
        // this wired combination works when switching between models
        setCurrentIndex(newIndex);
        currentSelection->select(newIndex, QItemSelectionModel::ClearAndSelect);
        // why does scrollTo not work ?
        // scrollTo(newIndex);
        selectRow(newIndex.row());
    }
}

void WLibraryTableView::saveTrackModelState(const QAbstractItemModel* model, const QString& key) {
    VERIFY_OR_DEBUG_ASSERT(model) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(!key.isEmpty()) {
        return;
    }
    ModelState* state = m_modelStateCache.take(key);
    if (!state) {
        state = new ModelState();
    }
    // qDebug() << "save: saveTrackModelState:" << key << model << verticalScrollBar()->value() << " | ";
    state->verticalScrollPosition = verticalScrollBar()->value();
    state->horizontalScrollPosition = horizontalScrollBar()->value();
    if (!selectionModel()->selectedIndexes().isEmpty()) {
        state->selectionIndex = selectionModel()->selectedIndexes();
    } else {
        state->selectionIndex = QModelIndexList();
    }
    if (selectionModel()->currentIndex().isValid()) {
        state->currentIndex = selectionModel()->currentIndex();
    } else {
        state->currentIndex = QModelIndex();
    }
    m_modelStateCache.insert(key, state, 1);

    WTrackTableViewHeader* pHeader = qobject_cast<WTrackTableViewHeader*>(horizontalHeader());
    if (pHeader) {
        pHeader->saveHeaderState();
    }
}

void WLibraryTableView::restoreTrackModelState(
        const QAbstractItemModel* model, const QString& key) {
    updateGeometries();
    //qDebug() << "restoreTrackModelState:" << key << model << m_vModelState.keys();
    if (model == nullptr) {
        return;
    }

    WTrackTableViewHeader* pHeader = qobject_cast<WTrackTableViewHeader*>(horizontalHeader());
    if (pHeader) {
        pHeader->restoreHeaderState();
    }

    ModelState* state = m_modelStateCache.take(key);
    if (!state) {
        return;
    }

    verticalScrollBar()->setValue(state->verticalScrollPosition);
    horizontalScrollBar()->setValue(state->horizontalScrollPosition);

    auto selection = selectionModel();
    selection->clearSelection();
    if (!state->selectionIndex.isEmpty()) {
        for (auto index : qAsConst(state->selectionIndex)) {
            selection->select(index, QItemSelectionModel::Select);
        }
    }
    if (state->currentIndex.isValid()) {
        selection->setCurrentIndex(state->currentIndex, QItemSelectionModel::NoUpdate);
    }
    // reinsert the state into the cache
    m_modelStateCache.insert(key, state, 1);
}

void WLibraryTableView::setTrackTableFont(const QFont& font) {
    setFont(font);
    QFontMetrics metrics(font);
    verticalHeader()->setMinimumSectionSize(metrics.height());
}

void WLibraryTableView::setTrackTableRowHeight(int rowHeight) {
    verticalHeader()->setDefaultSectionSize(math_max(
            rowHeight, verticalHeader()->minimumSectionSize()));
}

void WLibraryTableView::setSelectedClick(bool enable) {
    if (enable) {
        setEditTriggers(QAbstractItemView::SelectedClicked|QAbstractItemView::EditKeyPressed);
    } else {
        setEditTriggers(QAbstractItemView::EditKeyPressed);
    }
}

void WLibraryTableView::saveCurrentViewState() {
    const QAbstractItemModel* currentModel = model();
    QString key = getStateKey();
    if (!currentModel || key.isEmpty()) {
        return;
    }
    saveTrackModelState(currentModel, key);
}

void WLibraryTableView::restoreCurrentViewState() {
    const QAbstractItemModel* currentModel = model();
    QString key = getStateKey();
    if (!currentModel || key.isEmpty()) {
        return;
    }
    restoreTrackModelState(currentModel, key);
}

void WLibraryTableView::focusInEvent(QFocusEvent* event) {
    QTableView::focusInEvent(event);

    if (event->reason() == Qt::TabFocusReason ||
            event->reason() == Qt::BacktabFocusReason) {
        // On FocusIn caused by a tab action with no focused item, select the
        // current or first track which can then instantly be loaded to a deck.
        // This is especially helpful if the table has only one track, which can
        // not be selected with up/down buttons, either physical or emulated via
        // [Library],MoveVertical controls. See lp:1808632
        if (model()->rowCount() > 0) {
            if (selectionModel()->hasSelection()) {
                DEBUG_ASSERT(!selectionModel()->selectedIndexes().isEmpty());
                if (!currentIndex().isValid() ||
                        !selectionModel()->isSelected(currentIndex())) {
                    // Reselect the first selected index
                    selectRow(selectionModel()->selectedIndexes().first().row());
                }
            } else {
                if (!currentIndex().isValid()) {
                    // Select the first row if no row is focused
                    selectRow(0);
                    DEBUG_ASSERT(currentIndex().row() == 0);
                } else {
                    // Select the row of the currently focused index.
                    // For some reason selectRow(currentIndex().row()) would not
                    // select for the first Qt::BacktabFocusReason in a session
                    // even though currentIndex() is valid.
                    selectionModel()->select(currentIndex(),
                            QItemSelectionModel::Select | QItemSelectionModel::Rows);
                }
            }
            DEBUG_ASSERT(currentIndex().isValid());
            DEBUG_ASSERT(selectionModel()->isSelected(currentIndex()));
            // scrollTo() doesn't always seem to work!?
            scrollTo(currentIndex());
        }
    }
}
