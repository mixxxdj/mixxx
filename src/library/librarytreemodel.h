#ifndef LIBRARYTREEMODEL_H
#define LIBRARYTREEMODEL_H

#include <QSqlQuery>
#include <QStringList>

#include "library/treeitemmodel.h"

class MixxxLibraryFeature;
class TrackCollection;

class LibraryTreeModel : public TreeItemModel {
  public:
    LibraryTreeModel(MixxxLibraryFeature* pFeature, 
                     TrackCollection* pTrackCollection, 
                     QObject* parent = nullptr);

    QString getQuery(TreeItem* pTree) const;
    
  private:
    void createTracksTree();
    
    MixxxLibraryFeature* m_pFeature;
    TrackCollection* m_pTrackCollection;
    QStringList m_sortOrder;
    
    TreeItem* m_pLibraryItem;
};

#endif // LIBRARYTREEMODEL_H
