#include <QObject>

#include "preparelibrarytablemodel.h"
#include "library/trackcollection.h"

const QString RECENT_FILTER = "datetime_added > datetime('now', '-7 days')";

PrepareLibraryTableModel::PrepareLibraryTableModel(QObject* parent,
                                                   TrackCollection* pTrackCollection)
        : LibraryTableModel(parent, pTrackCollection,
                            "mixxx.db.model.prepare") {
    m_bShowRecentSongs = true;
    slotSearch("");
    select();

    connect(this, SIGNAL(doSearch(const QString&)),
            this, SLOT(slotSearch(const QString&)));
}


PrepareLibraryTableModel::~PrepareLibraryTableModel() {
}

bool PrepareLibraryTableModel::isColumnInternal(int column) {
    bool result = false;

    if ((column == fieldIndex(LIBRARYTABLE_DATETIMEADDED))) {
        result = false;
    } else {
        result = LibraryTableModel::isColumnInternal(column);
    }

    return result;
}

void PrepareLibraryTableModel::search(const QString& searchText) {
    // qDebug() << "PrepareLibraryTableModel::search()" << searchText
    //          << QThread::currentThread();
    emit(doSearch(searchText));
}

void PrepareLibraryTableModel::slotSearch(const QString& searchText) {
    BaseSqlTableModel::search(searchText,
                              m_bShowRecentSongs ? RECENT_FILTER : QString());
}

void PrepareLibraryTableModel::showRecentSongs() {
   m_bShowRecentSongs = true;
   search(currentSearch());
}

void PrepareLibraryTableModel::showAllSongs() {
    m_bShowRecentSongs = false;
    search(currentSearch());
}
