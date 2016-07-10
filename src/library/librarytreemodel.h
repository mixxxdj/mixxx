#ifndef LIBRARYTREEMODEL_H
#define LIBRARYTREEMODEL_H

#include <QHash>
#include <QSqlQuery>
#include <QStringList>

#include "library/treeitemmodel.h"

class CoverInfo;
class MixxxLibraryFeature;
class TrackCollection;

class LibraryTreeModel : public TreeItemModel {
  public:
    LibraryTreeModel(MixxxLibraryFeature* pFeature, 
                     TrackCollection* pTrackCollection, 
                     QObject* parent = nullptr);

    virtual QVariant data(const QModelIndex &index, int role);
    
    void setSortOrder(QStringList sortOrder);
    QString getQuery(TreeItem* pTree) const;
    
    void reloadTracksTree();
    
  private slots:
    
    void coverFound(const QObject* requestor, int requestReference, const CoverInfo&,
                    QPixmap pixmap, bool fromCache);
    
  private:    
    void createTracksTree();
    
    MixxxLibraryFeature* m_pFeature;
    TrackCollection* m_pTrackCollection;
    QStringList m_sortOrder;
    QString m_coverQuery;
    
    TreeItem* m_pLibraryItem;
    QHash<quint16, QModelIndex> m_hashToIndex;
};

#endif // LIBRARYTREEMODEL_H
