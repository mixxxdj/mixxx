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
                                     ConfigKey headerStateKey,
                                     ConfigKey vScrollBarPosKey)
        : QTableView(parent),
          m_pConfig(pConfig),
          m_headerStateKey(headerStateKey),
          m_vScrollBarPosKey(vScrollBarPosKey) {

    //Setup properties for table

    //Enable selection by rows and extended selection (ctrl/shift click)
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    setWordWrap(false);
    setShowGrid(false);
    setCornerButtonEnabled(false);
    setSortingEnabled(true);

    verticalHeader()->hide();
    verticalHeader()->setDefaultSectionSize(20);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    loadHeaderState();
    loadVScrollBarPosState();
}

WLibraryTableView::~WLibraryTableView() {
    qDebug() << "~WLibraryTableView";
    saveHeaderState();
    saveVScrollBarPosState();
}

void WLibraryTableView::setup(QDomNode node) {
    QPalette pal = palette();

    // Row colors
    if (!WWidget::selectNode(node, "BgColorRowEven").isNull() &&
        !WWidget::selectNode(node, "BgColorRowUneven").isNull()) {
        QColor r1;
        r1.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowEven"));
        r1 = WSkinColor::getCorrectColor(r1);
        QColor r2;
        r2.setNamedColor(WWidget::selectNodeQString(node, "BgColorRowUneven"));
        r2 = WSkinColor::getCorrectColor(r2);

        // For now make text the inverse of the background so it's readable In
        // the future this should be configurable from the skin with this as the
        // fallback option
        QColor text(255 - r1.red(), 255 - r1.green(), 255 - r1.blue());

        setAlternatingRowColors ( true );

        pal.setColor(QPalette::Base, r1);
        pal.setColor(QPalette::AlternateBase, r2);
        pal.setColor(QPalette::Text, text);
    }

    setPalette(pal);
}

void WLibraryTableView::loadHeaderState() {
    //Attempt to load and restore the state of the table's header (eg. column
    //positions/sizes).
    QString base64HeaderState = m_pConfig->getValueString(m_headerStateKey);
    QByteArray headerState = base64HeaderState.toAscii();
    headerState = QByteArray::fromBase64(headerState);
    horizontalHeader()->restoreState(headerState);
}

void WLibraryTableView::saveHeaderState() {
    QByteArray headerState = this->horizontalHeader()->saveState();
    QByteArray headerStateBase64 = headerState.toBase64();
    QString headerStateString = QString(headerStateBase64);
    m_pConfig->set(m_headerStateKey, ConfigValue(headerStateString));
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

