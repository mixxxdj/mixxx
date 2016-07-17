#include <QDateTime>

#include "library/libraryfeature.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/treeitem.h"

#include "library/historytreemodel.h"

HistoryTreeModel::HistoryTreeModel(LibraryFeature* pFeature,
                                   TrackCollection* pTrackCollection,
                                   QObject* parent)
        : TreeItemModel(parent),
          m_pFeature(pFeature), 
          m_pTrackCollection(pTrackCollection){
    m_columns << PLAYLISTTABLE_ID
              << PLAYLISTTABLE_DATECREATED
              << PLAYLISTTABLE_NAME;
    
    for (QString& s : m_columns) {
        s = "playlists." + s;
    }
}

void HistoryTreeModel::reloadListsTree() {
    TreeItem* pRootItem = new TreeItem();
    pRootItem->setLibraryFeature(m_pFeature);
    setRootItem(pRootItem);
    QString trackCountName = "TrackCount";
    
    QString queryStr = "SELECT %1,COUNT(%2) AS %7 "
                       "FROM playlists LEFT JOIN PlaylistTracks "
                       "ON %3=%4 "
                       "WHERE %5=2  "
                       "GROUP BY %4 "
                       "ORDER BY %6";
    queryStr = queryStr.arg(m_columns.join(","),
                            "PlaylistTracks." + PLAYLISTTRACKSTABLE_TRACKID,
                            "PlaylistTracks." + PLAYLISTTRACKSTABLE_PLAYLISTID,
                            "Playlists." + PLAYLISTTABLE_ID,
                            PLAYLISTTABLE_HIDDEN,
                            PLAYLISTTABLE_DATECREATED,
                            trackCountName);
    
    QSqlQuery query(m_pTrackCollection->getDatabase());
    if (!query.exec(queryStr)) {
        qDebug() << queryStr;
        LOG_FAILED_QUERY(query);
        return;
    }
    
    QSqlRecord record = query.record();
    HistoryQueryIndex ind;
    ind.iID = record.indexOf(PLAYLISTTABLE_ID);
    ind.iDate = record.indexOf(PLAYLISTTABLE_DATECREATED);
    ind.iName = record.indexOf(PLAYLISTTABLE_NAME);
    ind.iCount = record.indexOf(trackCountName);
    
    TreeItem* lastYear = nullptr;
    TreeItem* lastMonth = nullptr;
    TreeItem* lastPlaylist = nullptr;
    bool change = true;
    QDate lastDate;
    while (query.next()) {
        QDateTime dTime = query.value(ind.iDate).toDateTime();
        QDate auxDate = dTime.date();
        
        if (auxDate.year() != lastDate.year() || change) {
            change = true;
            QString year = QString::number(auxDate.year());
            lastYear = new TreeItem(year, year, m_pFeature, pRootItem);
            pRootItem->appendChild(lastYear);
        }
        
        if (auxDate.month() != lastDate.month() || change) {
            QString month = QDate::longMonthName(auxDate.month(), QDate::StandaloneFormat);
            QString monthNum = QString::number(auxDate.month());
            lastMonth = new TreeItem(month, monthNum, m_pFeature, lastYear);
            lastYear->appendChild(lastMonth);
        }
        
        QString sTime = dTime.toString("d hh:mm");
        sTime += QString(" (%1)").arg(query.value(ind.iCount).toString());
        QString pId = query.value(ind.iID).toString();
        
        lastPlaylist = new TreeItem(sTime, pId, m_pFeature, lastMonth);
        lastMonth->appendChild(lastPlaylist);
        lastDate = auxDate;
        change = false;
    }
    
    triggerRepaint();
}

QVariant HistoryTreeModel::data(const QModelIndex &index, int role) const {
    if (role != TreeItemModel::RoleQuery) {
        return TreeItemModel::data(index, role);
    }
    
    TreeItem* pTree = static_cast<TreeItem*>(index.internalPointer());
    DEBUG_ASSERT_AND_HANDLE(pTree) {
        return QVariant();
    }
    
    // We need to know the depth of the item to apply the filter
    // depth == 0 => Year
    // depth == 1 => Month
    // depth == 2 => A single playlist
    int depth = 0;
    TreeItem* pAux = pTree;
    while (pAux->parent() != nullptr && pAux->parent() != m_pRootItem) {
        pAux = pAux->parent();
        ++depth;
    }
    
    return QVariant();
}
