#ifndef HISTORYTREEMODEL_H
#define HISTORYTREEMODEL_H

#include <QStringList>

#include "library/treeitemmodel.h"

class LibraryFeature;
class TrackCollection;

class HistoryTreeModel : public TreeItemModel
{
  public:
    HistoryTreeModel(LibraryFeature *pFeature, TrackCollection* pTrackCollection, 
                     QObject* parent = nullptr);

    void reloadListsTree();
    virtual QVariant data(const QModelIndex& index, int role) const;

  private:
    struct HistoryQueryIndex {
        int iID;
        int iDate;
        int iName;
        int iCount;
    };
    
    QList<QVariant> idsFromItem(TreeItem* pTree) const;
    
    LibraryFeature* m_pFeature;
    TrackCollection* m_pTrackCollection;
    QStringList m_columns;
};

#endif // HISTORYTREEMODEL_H
