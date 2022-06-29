#include "widget/wlibrarytableview.h"

#include <QFocusEvent>
#include <QFontMetrics>
#include <QHeaderView>
#include <QPalette>
#include <QScrollBar>

#include "library/trackmodel.h"
#include "moc_wlibrarytableview.cpp"
#include "util/math.h"
#include "widget/wskincolor.h"
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
}


void WLibraryTableView::moveSelection(int delta) {
    QAbstractItemModel* pModel = model();

    if (pModel == nullptr) {
        return;
    }

    while (delta != 0) {
        QItemSelectionModel* currentSelection = selectionModel();
        if (currentSelection->selectedRows().length() > 0) {
            if (delta > 0) {
                // i is positive, so we want to move the highlight down
                int row = currentSelection->selectedRows().last().row();
                if (row + 1 < pModel->rowCount()) {
                    selectRow(row + 1);
                } else {
                    // we wrap around at the end of the list so it is faster to get
                    // to the top of the list again
                    selectRow(0);
                }

                delta--;
            } else {
                // i is negative, so move down
                int row = currentSelection->selectedRows().first().row();
                if (row - 1 >= 0) {
                    selectRow(row - 1);
                } else {
                    selectRow(pModel->rowCount() - 1);
                }

                delta++;
            }
        } else {
            // no selection, so select the first or last element depending on delta
            if (delta > 0) {
                selectRow(0);
                delta--;
            } else {
                selectRow(pModel->rowCount() - 1);
                delta++;
            }
        }
    }
}

void WLibraryTableView::saveTrackModelState(
        const QAbstractItemModel* model, const QString& key) {
    //qDebug() << "saveTrackModelState:" << model << key;
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

    state->verticalScrollPosition = verticalScrollBar()->value();
    state->horizontalScrollPosition = horizontalScrollBar()->value();

    state->selectedRows = selectionModel()->selectedRows();

    const QModelIndex currIndex = selectionModel()->currentIndex();
    if (currIndex.isValid()) {
        state->currentIndex = currIndex;
    } else {
        state->currentIndex = QModelIndex();
    }

    m_modelStateCache.insert(key, state, 1);
}

bool WLibraryTableView::restoreTrackModelState(
        const QAbstractItemModel* model, const QString& key) {
    //qDebug() << "restoreTrackModelState:" << model << key;
    //qDebug() << m_modelStateCache.keys();
    if (model == nullptr) {
        return false;
    }

    ModelState* state = m_modelStateCache.take(key);
    if (!state) {
        // No previous state for model key,
        // reset scroll bars and current index
        verticalScrollBar()->setValue(0);
        horizontalScrollBar()->setValue(0);
        setCurrentIndex(QModelIndex());
        return false;
    }

    verticalScrollBar()->setValue(state->verticalScrollPosition);
    horizontalScrollBar()->setValue(state->horizontalScrollPosition);

    auto selection = selectionModel();
    selection->clearSelection();
    QModelIndexList selectedRows = state->selectedRows;
    if (!selectedRows.isEmpty()) {
        for (auto index : qAsConst(selectedRows)) {
            selection->select(index,
                    QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }

    QModelIndex currIndex = state->currentIndex;
    if (currIndex.isValid()) {
        selection->setCurrentIndex(currIndex, QItemSelectionModel::NoUpdate);
    }

    // reinsert the state into the cache
    m_modelStateCache.insert(key, state, 1);
    return true;
}

void WLibraryTableView::setTrackTableFont(const QFont& font) {
    setFont(font);
    QFontMetrics metrics(font);
    verticalHeader()->setMinimumSectionSize(metrics.height());
    // Resize the 'Played' checkbox and the BPM lock icon.
    // Note: this works well for library font sizes up to ~200% of the original
    // system font's size (that set with Qt5 Settings respectively). Above that,
    // the indicators start to overlap the item text because the painter rectangle
    // is not resized (also depending on the system font size).
    setStyleSheet(QStringLiteral(
            "WTrackTableView::indicator,"
            "#LibraryBPMButton::indicator {"
            "height: %1px;"
            "width: %1px;}")
                          .arg(metrics.height() * 0.7));
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
    QString key = getModelStateKey();
    if (!currentModel || key.isEmpty()) {
        return;
    }
    saveTrackModelState(currentModel, key);
}

bool WLibraryTableView::restoreCurrentViewState() {
    const QAbstractItemModel* currentModel = model();
    QString key = getModelStateKey();
    if (!currentModel || key.isEmpty()) {
        return false;
    }
    return restoreTrackModelState(currentModel, key);
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
