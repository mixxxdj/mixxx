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

WLibraryTableView::WLibraryTableView(QWidget* parent,
        UserSettingsPointer pConfig,
        const ConfigKey& vScrollBarPosKey)
        : QTableView(parent),
          m_pConfig(pConfig),
          m_vScrollBarPosKey(vScrollBarPosKey) {
    loadVScrollBarPosState();

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

void WLibraryTableView::loadVScrollBarPosState() {
    // TODO(rryan) I'm not sure I understand the value in saving the v-scrollbar
    // position across restarts of Mixxx. Now that we have different views for
    // each mode, the views should just maintain their scrollbar position when
    // you switch views. We should discuss this.
    m_noSearchVScrollBarPos = m_pConfig->getValueString(m_vScrollBarPosKey).toInt();
}

void WLibraryTableView::restoreNoSearchVScrollBarPos() {
    // Restore the scrollbar's position (scroll to that spot)
    // when the search has been cleared
    //qDebug() << "restoreNoSearchVScrollBarPos()" << m_noSearchVScrollBarPos;
    updateGeometries();
    verticalScrollBar()->setValue(m_noSearchVScrollBarPos);
}

void WLibraryTableView::saveNoSearchVScrollBarPos() {
    // Save the scrollbar's position so we can return here after
    // a search is cleared.
    //qDebug() << "saveNoSearchVScrollBarPos()" << m_noSearchVScrollBarPos;
    m_noSearchVScrollBarPos = verticalScrollBar()->value();
}


void WLibraryTableView::saveVScrollBarPosState() {
    //Save the vertical scrollbar position.
    int scrollbarPosition = verticalScrollBar()->value();
    m_pConfig->set(m_vScrollBarPosKey, ConfigValue(scrollbarPosition));
}

void WLibraryTableView::moveSelection(int delta) {
    QAbstractItemModel* pModel = model();

    if (pModel == nullptr) {
        return;
    }

    while(delta != 0) {
        // TODO(rryan) what happens if there is nothing selected?
        QModelIndex current = currentIndex();
        if(delta > 0) {
            // i is positive, so we want to move the highlight down
            int row = current.row();
            if (row + 1 < pModel->rowCount()) {
                selectRow(row + 1);
            }

            delta--;
        } else {
            // i is negative, so we want to move the highlight up
            int row = current.row();
            if (row - 1 >= 0) {
                selectRow(row - 1);
            }

            delta++;
        }
    }
}

void WLibraryTableView::saveVScrollBarPos(TrackModel* key){
    m_vScrollBarPosValues[key] = verticalScrollBar()->value();
}

void WLibraryTableView::restoreVScrollBarPos(TrackModel* key){
    updateGeometries();

    if (m_vScrollBarPosValues.contains(key)){
        verticalScrollBar()->setValue(m_vScrollBarPosValues[key]);
    }else{
        m_vScrollBarPosValues[key] = 0;
        verticalScrollBar()->setValue(0);
    }
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
