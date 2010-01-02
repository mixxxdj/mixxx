// wtracktableviewheader.cpp
// Created 1/2/2010 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "widget/wtracktableviewheader.h"
#include "library/trackmodel.h"

WTrackTableViewHeader::WTrackTableViewHeader(Qt::Orientation orientation,
                                             QWidget* parent)
        : QHeaderView(orientation, parent),
          m_menu(tr("Show or hide columns."), this),
          m_signalMapper(this) {

    // Show the sort indicator (technically redundant since setSortingEnabled()
    // on a View will handle this)
    setSortIndicatorShown(true);
    //Allow the columns to be reordered.
    setMovable(true);

    connect(&m_signalMapper, SIGNAL(mapped(int)),
            this, SLOT(showOrHideColumn(int)));
}

WTrackTableViewHeader::~WTrackTableViewHeader() {

}

void WTrackTableViewHeader::contextMenuEvent(QContextMenuEvent* event) {
    m_menu.popup(event->globalPos());
}

void WTrackTableViewHeader::setModel(QAbstractItemModel* model) {
    // First clear all the context menu actions for the old model.
    clearActions();

    // Now set the header view to show the new model
    QHeaderView::setModel(model);

    // Now build actions for the new TrackModel
    TrackModel* trackModel = dynamic_cast<TrackModel*>(model);

    if (!trackModel) {
        return;
    }

    int columns = model->columnCount();
    for (int i = 0; i < columns; ++i) {
        if (trackModel->isColumnInternal(i))
            continue;

        QString title = model->headerData(i, orientation()).toString();
        QAction* action = new QAction(title, &m_menu);
        action->setCheckable(true);
        action->setChecked(!isSectionHidden(i));

        // Map this action's signals via our QSignalMapper
        m_signalMapper.setMapping(action, i);
        m_columnActions.insert(i, action);
        connect(action, SIGNAL(triggered()),
                &m_signalMapper, SLOT(map()));
        m_menu.addAction(action);
    }
}

void WTrackTableViewHeader::clearActions() {
    // The QActions are parented to the menu, so clearing deletes them. Since
    // they are deleted we don't have to disconnect their signals from the
    // mapper.
    m_columnActions.clear();
    m_menu.clear();
}

void WTrackTableViewHeader::showOrHideColumn(int column) {
    if (!m_columnActions.contains(column)) {
        qDebug() << "WTrackTableViewHeader got invalid column" << column;
        return;
    }

    QAction* action = m_columnActions[column];
    if (action->isChecked()) {
        showSection(column);
    } else {
        hideSection(column);
    }
}
