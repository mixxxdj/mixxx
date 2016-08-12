// wlibrarytableview.cpp
// Created 10/19/2009 by RJ Ryan (rryan@mit.edu)

#include <QHeaderView>
#include <QPalette>
#include <QScrollBar>
#include <QFontMetrics>

#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "widget/wlibrarytableview.h"
#include "util/math.h"

WLibraryTableView::WLibraryTableView(QWidget* parent,
                                     UserSettingsPointer pConfig,
                                     ConfigKey vScrollBarPosKey)
        : QTableView(parent),
          m_pConfig(pConfig),
          m_vScrollBarPosKey(vScrollBarPosKey),
          m_savedSortColumn(-1),
          m_savedSortOrder(Qt::AscendingOrder) {

    // Setup properties for table

    // Editing starts when clicking on an already selected item.
    setEditTriggers(QAbstractItemView::SelectedClicked);

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

    loadVScrollBarPosState();

    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SIGNAL(scrollValueChanged(int)));

    setTabKeyNavigation(false);
}

WLibraryTableView::~WLibraryTableView() {
    saveVScrollBarPosState();
}

void WLibraryTableView::loadVScrollBarPosState() {
    // TODO(rryan) I'm not sure I understand the value in saving the v-scrollbar
    // position across restarts of Mixxx. Now that we have different views for
    // each mode, the views should just maintain their scrollbar position when
    // you switch views. We should discuss this.
    m_iSavedVScrollBarPos = m_pConfig->getValueString(m_vScrollBarPosKey).toInt();
}

void WLibraryTableView::restoreView() {
    // Restore the scrollbar's position (scroll to that spot), and sorting order
    // when the search has been cleared
    updateGeometries();
    verticalScrollBar()->setValue(m_iSavedVScrollBarPos);
    horizontalHeader()->setSortIndicator(m_savedSortColumn, m_savedSortOrder);
}

void WLibraryTableView::saveView() {
    // Save the scrollbar's position and sorting order so we can return here 
    // after a search is cleared.
    m_iSavedVScrollBarPos = verticalScrollBar()->value();
    m_savedSortColumn = horizontalHeader()->sortIndicatorSection();
    m_savedSortOrder = horizontalHeader()->sortIndicatorOrder();
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

void WLibraryTableView::restoreQuery(const SavedSearchQuery& query) {
    verticalScrollBar()->setValue(query.vScrollBarPos);
    
    Qt::SortOrder order;
    if (query.sortAscendingOrder) {
        order = Qt::AscendingOrder;
    } else {
        order = Qt::DescendingOrder;
    }
    
    horizontalHeader()->setSortIndicator(query.sortColumn, order);
}

SavedSearchQuery WLibraryTableView::saveQuery(SavedSearchQuery query) const {
    query.vScrollBarPos = verticalScrollBar()->value();
    query.sortColumn = horizontalHeader()->sortIndicatorSection();
    query.sortAscendingOrder = 
            horizontalHeader()->sortIndicatorOrder() == Qt::AscendingOrder;
    return query;
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
