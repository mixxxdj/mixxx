// Created 3/17/2012 by Keith Salisbury (keithsalisbury@gmail.com)

#include <QObject>

#include "selectorlibrarytablemodel.h"
#include "library/trackcollection.h"

const QString RECENT_FILTER = "datetime_added > datetime('now', '-7 days')";

SelectorLibraryTableModel::SelectorLibraryTableModel(QObject* parent,
                                                   TrackCollection* pTrackCollection)
        : LibraryTableModel(parent, pTrackCollection,
                            "mixxx.db.model.prepare") {
    m_bShowRecentSongs = true;
    setSearch("", m_bShowRecentSongs ? RECENT_FILTER : QString());
    select();

    connect(this, SIGNAL(doSearch(const QString&)),
            this, SLOT(slotSearch(const QString&)));
}


SelectorLibraryTableModel::~SelectorLibraryTableModel() {
}

bool SelectorLibraryTableModel::isColumnInternal(int column) {
    return LibraryTableModel::isColumnInternal(column);
}

void SelectorLibraryTableModel::search(const QString& searchText) {
    // qDebug() << "SelectorLibraryTableModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void SelectorLibraryTableModel::slotSearch(const QString& searchText) {
    BaseSqlTableModel::search(searchText,
                              m_bShowRecentSongs ? RECENT_FILTER : QString());
}

void SelectorLibraryTableModel::filterByGenre() {
   m_bShowRecentSongs = true;
   search(currentSearch());
}

void SelectorLibraryTableModel::filterByBpm() {
    m_bShowRecentSongs = false;
    search(currentSearch());
}
