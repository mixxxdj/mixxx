#ifndef HISTORYTREEMODEL_H
#define HISTORYTREEMODEL_H

#include <QStringList>

#include "library/treeitemmodel.h"

class HistoryFeature;
class TrackCollection;

class HistoryTreeModel : public TreeItemModel
{
  public:
    HistoryTreeModel(HistoryFeature* pFeature, TrackCollection* pTrackCollection, 
                     QObject* parent = nullptr);

    QModelIndex reloadListsTree(int playlistId);
    QModelIndex indexFromPlaylistId(int playlistId);
    QVariant data(const QModelIndex& index, int role) const override;

  private:
    struct HistoryQueryIndex {
        int iID;
        int iDate;
        int iName;
        int iCount;
    };
    
    QList<QVariant> idsFromItem(TreeItem* pTree) const;
    TreeItem* findItemFromPlaylistId(TreeItem* pTree, int playlistId, int& row) const;
    
    HistoryFeature* m_pFeature;
    TrackCollection* m_pTrackCollection;
    QStringList m_columns;
};

#endif // HISTORYTREEMODEL_H
