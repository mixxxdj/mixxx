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
#include "widget/wwidget.h"

namespace {
constexpr int kClearModelStatesLowWatermark = 1000;
constexpr int kClearModelStatesHighWatermark = 1100;
} // namespace

WLibraryTableView::WLibraryTableView(QWidget* parent,
        UserSettingsPointer pConfig,
        const ConfigKey& vScrollBarPosKey)
        : QTableView(parent),
          m_pConfig(pConfig),
          m_vScrollBarPosKey(vScrollBarPosKey) {

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
    QItemSelectionModel* currentSelection = selectionModel();
    clearSelection();
    currentSelection->clearSelection();
    QModelIndex newIndex;
    while (delta != 0) {
        QModelIndex currentIndex = currentSelection->currentIndex();
        //clearSelection();
        //currentSelection->clearSelection();
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
                //selectRow(0);
                newIndex = pModel->index(0, 0);
                delta--;
            } else {
                //selectRow(pModel->rowCount() - 1);
                newIndex = pModel->index(pModel->rowCount() - 1, 0);
                delta++;
            }
        }
        // this wired combination works when switching between
        setCurrentIndex(newIndex);
        currentSelection->select(newIndex, QItemSelectionModel::ClearAndSelect);
        // does not work, why ?
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
    ModelState* state;
    if (m_vModelState.contains(key)) {
        state = m_vModelState[key];
    } else {
        state = new ModelState();
    }
    state->lastChange = QDateTime::currentSecsSinceEpoch();
    // qDebug() << "save: saveTrackModelState:" << key << model << verticalScrollBar()->value() << " | ";
    state->scrollPosition = verticalScrollBar()->value();
    const QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();
    if (!selectedIndexes.isEmpty()) {
        state->selectionIndex = selectedIndexes;
    } else {
        state->selectionIndex = QModelIndexList();
    }
    state->currentIndex = selectionModel()->currentIndex();
    m_vModelState[key] = state;

    if (m_vModelState.size() > kClearModelStatesHighWatermark) {
        clearStateCache();
    }
}

void WLibraryTableView::restoreTrackModelState(
        const QAbstractItemModel* model, const QString& key) {
    updateGeometries();
    // qDebug() << "restoreTrackModelState:" << key << model << m_vModelState.keys();
    if (model == nullptr) {
        return;
    }

    if (!m_vModelState.contains(key)) {
        return;
    }
    ModelState* state = m_vModelState[key];

    verticalScrollBar()->setValue(state->scrollPosition);

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
}

void WLibraryTableView::setTrackTableFont(const QFont& font) {
    setFont(font);
    setTrackTableRowHeight(verticalHeader()->defaultSectionSize());
}

void WLibraryTableView::setTrackTableRowHeight(int rowHeight) {
    QFontMetrics metrics(font());
    int fontHeightPx = metrics.height();
    verticalHeader()->setDefaultSectionSize(math_max(
                                                rowHeight, fontHeightPx));
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

void WLibraryTableView::clearStateCache() {
    auto lru = QMultiMap<qint64, QString>();

    auto i = m_vModelState.constBegin();
    while (i != m_vModelState.constEnd()) {
        lru.insert(i.value()->lastChange, i.key());
        i++;
    }
    QList sortKeys = lru.keys();
    std::sort(sortKeys.begin(),
            sortKeys.end(),
            [](const qint64 a, const qint64 b) { return a < b; });

    while (m_vModelState.size() > kClearModelStatesLowWatermark) {
        const QStringList keys = lru.values(sortKeys.takeFirst());
        for (const auto& key : keys) {
            auto m = m_vModelState.take(key);
            delete m;
        }
    }
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
