#include <QDate>

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
}

void HistoryTreeModel::reloadListsTree() {
    TreeItem* pRootItem = new TreeItem();
    pRootItem->setLibraryFeature(m_pFeature);
    setRootItem(pRootItem);
    
    QString queryStr = "SELECT %1 FROM playlists WHERE %2=2 ORDER BY %3";
    queryStr = queryStr.arg(m_columns.join(","),
                            PLAYLISTTABLE_HIDDEN,
                            PLAYLISTTABLE_DATECREATED);
    
    QSqlQuery query(m_pTrackCollection->getDatabase());
    if (!query.exec(queryStr)) {
        LOG_FAILED_QUERY(query);
        return;
    }
    
    QSqlRecord record = query.record();
    HistoryQueryIndex ind;
    ind.iID = record.indexOf(PLAYLISTTABLE_ID);
    ind.iDate = record.indexOf(PLAYLISTTABLE_DATECREATED);
    ind.iName = record.indexOf(PLAYLISTTABLE_NAME);
    
    TreeItem* lastYear = nullptr;
    TreeItem* lastMonth = nullptr;
    TreeItem* lastDay = nullptr;
    TreeItem* lastPlaylist = nullptr;
    int count = 0;
    QDate lastDate;
    while (query.next()) {
        QDate auxDate = query.value(ind.iDate).toDate();
        
        if (auxDate.year() != lastDate.year() || count == 0) {
            count = 0;
            QString year = QString::number(auxDate.year());
            lastYear = new TreeItem(year, year, m_pFeature, pRootItem);
            pRootItem->appendChild(lastYear);
        }
        
        if (auxDate.month() != lastDate.month() || count == 0) {
            count = 0;
            QString month = QDate::longMonthName(auxDate.month(), QDate::StandaloneFormat);
            lastMonth = new TreeItem(month, month, m_pFeature, lastYear);
            lastYear->appendChild(lastMonth);
        }
        
        if (auxDate.day() != lastDate.day() || count == 0) {
            count = 0;
            QString day = QString::number(auxDate.day());
            lastDay = new TreeItem(day, day, m_pFeature, lastMonth);
            lastMonth->appendChild(lastDay);
        } 
        
        ++count;
        QString sCount = QString::number(count);
        QString pId = query.value(ind.iID).toString();
        
        lastPlaylist = new TreeItem(sCount, pId, m_pFeature, lastDay);
        lastDay->appendChild(lastPlaylist);
        lastDate = auxDate;
    }
    
    triggerRepaint();
}
