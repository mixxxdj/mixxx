#include <QDateTime>

#include "library/features/history/historyfeature.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "util/assert.h"

#include "library/features/history/historytreemodel.h"

HistoryTreeModel::HistoryTreeModel(HistoryFeature* pFeature,
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

QModelIndex HistoryTreeModel::reloadListsTree(int playlistId) {
    VERIFY_OR_DEBUG_ASSERT (m_pFeature) {
        return QModelIndex();
    }

    TreeItem* pRootItem = setRootItem(std::make_unique<TreeItem>(m_pFeature.data()));
    QString trackCountName = "TrackCount";

    QString queryStr = "SELECT %1,COUNT(%2) AS %7 "
                       "FROM playlists LEFT JOIN PlaylistTracks "
                       "ON %3=%4 "
                       "WHERE %5=2  "
                       "GROUP BY %4 "
                       "ORDER BY %6 DESC";
    queryStr = queryStr.arg(m_columns.join(","),
                            "PlaylistTracks." + PLAYLISTTRACKSTABLE_TRACKID,
                            "PlaylistTracks." + PLAYLISTTRACKSTABLE_PLAYLISTID,
                            "Playlists." + PLAYLISTTABLE_ID,
                            PLAYLISTTABLE_HIDDEN,
                            PLAYLISTTABLE_DATECREATED,
                            trackCountName);

    QSqlQuery query(m_pTrackCollection->database());
    if (!query.exec(queryStr)) {
        qDebug() << queryStr;
        LOG_FAILED_QUERY(query);
        return QModelIndex();
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
    int row = 0;
    int selectedRow = -1;
    TreeItem* selectedItem = nullptr;

    while (query.next()) {
        QDateTime dTime = query.value(ind.iDate).toDateTime();
        QDate auxDate = dTime.date();

        if (auxDate.year() != lastDate.year() || change) {
            row = 0;
            change = true;
            QString year = QString::number(auxDate.year());
            lastYear = pRootItem->appendChild(year, -1);
        }

        if (auxDate.month() != lastDate.month() || change) {
            row = 0;
            QString month = QDate::longMonthName(auxDate.month(), QDate::StandaloneFormat);
            lastMonth = lastYear->appendChild(month, -1);
        }

        QString sData = query.value(ind.iName).toString();
        sData += QString(" (%1)").arg(query.value(ind.iCount).toString());

        int id = query.value(ind.iID).toInt();

        lastPlaylist = lastMonth->appendChild(sData, id);
        m_pFeature->decorateChild(lastPlaylist, id);
        if (id == playlistId) {
            selectedRow = row;
            selectedItem = lastPlaylist;
        }
        lastDate = auxDate;
        change = false;
        ++row;
    }

    triggerRepaint();
    if (selectedRow < 0) return QModelIndex();
    return createIndex(selectedRow, 0, selectedItem);
}

QModelIndex HistoryTreeModel::indexFromPlaylistId(int playlistId) {
    int row = -1;
    TreeItem* pItem = findItemFromPlaylistId(getRootItem(), playlistId, row);
    return createIndex(row, 0, pItem);
}

QVariant HistoryTreeModel::data(const QModelIndex& index, int role) const {
    if (role != AbstractRole::RoleQuery) {
        return TreeItemModel::data(index, role);
    }

    TreeItem* pTree = static_cast<TreeItem*>(index.internalPointer());
    VERIFY_OR_DEBUG_ASSERT(pTree) {
        return QVariant();
    }

    return idsFromItem(pTree);
}

QList<QVariant> HistoryTreeModel::idsFromItem(TreeItem* pTree) const {
    if (pTree->childRows() <= 0) {
        bool ok;
        int value = pTree->getData().toInt(&ok);
        QList<QVariant> ret;
        if (!ok) {
            return ret;
        }
        ret.append(value);
        return ret;
    }

    QList<QVariant> res;
    int size = pTree->childRows();
    for (int i = 0; i < size; ++i) {
        QList<QVariant> aux = idsFromItem(pTree->child(i));
        res.append(aux);
    }
    return res;
}

TreeItem* HistoryTreeModel::findItemFromPlaylistId(
		TreeItem* pTree, int playlistId, int& row) const {
    int size = pTree->childRows();
    if (size <= 0) {
        bool ok = false;
        int value = pTree->getData().toInt(&ok);
        if (ok && value == playlistId) {
            return pTree;
        }
        return nullptr;
    }
    row = 0;
    for (int i = 0; i < size; ++i) {
        TreeItem* child = pTree->child(i);
        TreeItem* result = findItemFromPlaylistId(child, playlistId, row);
        if (result != nullptr) {
            return result;
        }
        ++row;
    }
    return nullptr;
}
