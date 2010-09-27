// wlibrarytableview.cpp
// Created 10/19/2009 by RJ Ryan (rryan@mit.edu)

#include <QHeaderView>
#include <QPalette>
#include <QScrollBar>

#include "widget/wwidget.h"
#include "widget/wskincolor.h"
#include "widget/wlibrarytableview.h"

WLibraryTableView::WLibraryTableView(QWidget* parent,
                                     ConfigObject<ConfigValue>* pConfig,
                                     ConfigKey vScrollBarPosKey)
        : QTableView(parent),
          m_pConfig(pConfig),
          m_vScrollBarPosKey(vScrollBarPosKey) {

    //Setup properties for table

    //Enable selection by rows and extended selection (ctrl/shift click)
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setWordWrap(false);
    setShowGrid(false);
    setCornerButtonEnabled(false);
    setSortingEnabled(true);
    //Work around a Qt bug that lets you make your columns so wide you
    //can't reach the divider to make them small again.
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    verticalHeader()->hide();
    verticalHeader()->setDefaultSectionSize(20);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setAlternatingRowColors(true);

    loadVScrollBarPosState();
}

WLibraryTableView::~WLibraryTableView() {
    qDebug() << "~WLibraryTableView";
    saveVScrollBarPosState();
}

void WLibraryTableView::setup(QDomNode node) {
}

void WLibraryTableView::loadVScrollBarPosState() {
    // TODO(rryan) I'm not sure I understand the value in saving the v-scrollbar
    // position across restarts of Mixxx. Now that we have different views for
    // each mode, the views should just maintain their scrollbar position when
    // you switch views. We should discuss this.
    m_iSavedVScrollBarPos = m_pConfig->getValueString(m_vScrollBarPosKey).toInt();
}

void WLibraryTableView::restoreVScrollBarPos() {
    //Restore the scrollbar's position (scroll to that spot)
    //when the search has been cleared
    verticalScrollBar()->setValue(m_iSavedVScrollBarPos);
}

void WLibraryTableView::saveVScrollBarPos() {
    //Save the scrollbar's position so we can return here after
    //a search is cleared.
    m_iSavedVScrollBarPos = verticalScrollBar()->value();
}


void WLibraryTableView::saveVScrollBarPosState() {
    //Save the vertical scrollbar position.
    int scrollbarPosition = verticalScrollBar()->value();
    m_pConfig->set(m_vScrollBarPosKey, ConfigValue(scrollbarPosition));
}

