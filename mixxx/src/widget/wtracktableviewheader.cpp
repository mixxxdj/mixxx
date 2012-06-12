// wtracktableviewheader.cpp
// Created 1/2/2010 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "widget/wtracktableviewheader.h"
#include "library/trackmodel.h"

#define WTTVH_MINIMUM_SECTION_SIZE 20

WTrackTableViewHeader::WTrackTableViewHeader(Qt::Orientation orientation,
                                             QWidget* parent)
        : QHeaderView(orientation, parent),
          m_menu(tr("Show or hide columns."), this),
          m_signalMapper(this) {
    connect(&m_signalMapper, SIGNAL(mapped(int)),
            this, SLOT(showOrHideColumn(int)));
}

WTrackTableViewHeader::~WTrackTableViewHeader() {
}

void WTrackTableViewHeader::contextMenuEvent(QContextMenuEvent* event) {
    m_menu.popup(event->globalPos());
}

void WTrackTableViewHeader::setModel(QAbstractItemModel* model) {
    TrackModel* oldTrackModel = getTrackModel();

    if (dynamic_cast<QAbstractItemModel*>(oldTrackModel) == model) {
        // If the models are the same, do nothing but the redundant call.
        QHeaderView::setModel(model);
        return;
    }

    // Won't happen in practice since the WTrackTableView new's a new
    // WTrackTableViewHeader each time a new TrackModel is loaded.
    // if (oldTrackModel) {
    //     saveHeaderState();
    // }

    // First clear all the context menu actions for the old model.
    clearActions();

    // Now set the header view to show the new model
    QHeaderView::setModel(model);

    // Now build actions for the new TrackModel
    TrackModel* trackModel = dynamic_cast<TrackModel*>(model);

    if (!trackModel) {
        return;
    }

    // Restore saved header state to get sizes, column positioning, etc. back.
    restoreHeaderState();

    // Here we can override values to prevent restoring corrupt values from database
    setMovable(true);

    // Setting true in the next line causes Bug #925619 at least with Qt 4.6.1
    setCascadingSectionResizes(false);

    setMinimumSectionSize(WTTVH_MINIMUM_SECTION_SIZE);

    int columns = model->columnCount();
    for (int i = 0; i < columns; ++i) {
        if (trackModel->isColumnInternal(i)) {
            continue;
        }

        QString title = model->headerData(i, orientation()).toString();
        QAction* action = new QAction(title, &m_menu);
        action->setCheckable(true);

        /* If Mixxx starts the first time or the header states have been cleared
         * due to database schema evolution we gonna hide all columns that may
         * contain a potential large number of NULL values.  Here we uncheck
         * item in the context menu that are hidden by defualt (e.g., key
         * column)
         */
        if (!hasPersistedHeaderState() &&
            trackModel->isColumnHiddenByDefault(i)) {
            action->setChecked(false);
        } else {
            action->setChecked(!isSectionHidden(i));
        }

        // Map this action's signals via our QSignalMapper
        m_signalMapper.setMapping(action, i);
        m_columnActions.insert(i, action);
        connect(action, SIGNAL(triggered()),
                &m_signalMapper, SLOT(map()));
        m_menu.addAction(action);

        // force the section size to be a least WTTVH_MINIMUM_SECTION_SIZE
        if (sectionSize(i) <  WTTVH_MINIMUM_SECTION_SIZE) {
            // This might happen if  WTTVH_MINIMUM_SECTION_SIZ has changed or
            // the header state from database was corrupt
            resizeSection(i,WTTVH_MINIMUM_SECTION_SIZE);
        }
    }

    // Safety check against someone getting stuck with all columns hidden
    // (produces an empty library table). Just re-show them all.
    if (hiddenCount() == columns) {
        for (int i = 0; i < columns; ++i) {
            showSection(i);
        }
    }
}

void WTrackTableViewHeader::saveHeaderState() {
    TrackModel* track_model = getTrackModel();
    if (!track_model) {
        return;
    }
    // Convert the QByteArray to a Base64 string and save it.
    QString headerState = QString(saveState().toBase64());
    //bool result =
    track_model->setModelSetting("header_state", headerState);
    //qDebug() << "Saving old header state:" << result << headerState;
}

void WTrackTableViewHeader::restoreHeaderState() {
    TrackModel* track_model = getTrackModel();

    if (!track_model) {
        return;
    }

    QString headerStateString = track_model->getModelSetting("header_state");
    if (!headerStateString.isNull()) {
        // Load the previous header state (stored as a Base 64 string). Decode
        // it and restore it.
        //qDebug() << "Restoring header state" << headerStateString;
        QByteArray headerState = headerStateString.toAscii();
        headerState = QByteArray::fromBase64(headerState);
        restoreState(headerState);
    }
}

bool WTrackTableViewHeader::hasPersistedHeaderState() {
    TrackModel* track_model = getTrackModel();
    if (!track_model) {
        return false;
    }
    QString headerStateString = track_model->getModelSetting("header_state");

    if (!headerStateString.isNull()) return true;
    return false;

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
        // If the user hides every column then the table will disappear. This
        // guards against that. NB: hiddenCount reflects checked QAction's so
        // size-hiddenCount will be zero the moment they uncheck the last
        // section.
        if (m_columnActions.size() - hiddenCount() > 0) {
            hideSection(column);
        } else {
            // Otherwise, ignore the request and re-check this QAction.
            action->setChecked(true);
        }
    }
}

int WTrackTableViewHeader::hiddenCount() {
    int count = 0;
    for (QMap<int, QAction*>::iterator it = m_columnActions.begin();
         it != m_columnActions.end(); it++) {
        QAction* pAction = *it;
        if (!pAction->isChecked())
            count += 1;
    }
    return count;
}

TrackModel* WTrackTableViewHeader::getTrackModel() {
    TrackModel* trackModel = dynamic_cast<TrackModel*>(model());
    return trackModel;
}
