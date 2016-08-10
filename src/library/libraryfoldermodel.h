#ifndef LIBRARYFOLDERMODEL_H
#define LIBRARYFOLDERMODEL_H

#include <QSqlQuery>

#include "library/treeitemmodel.h"

class LibraryFeature;
class TrackCollection;

class LibraryFolderModel : public TreeItemModel
{
  public:
    LibraryFolderModel(LibraryFeature *pFeature, 
                       TrackCollection *pTrackCollection, 
                       UserSettingsPointer pConfig, 
                       QObject *parent);
    
    virtual QVariant data(const QModelIndex &index, int role) const;

    void setFolderRecursive(bool recursive);
    bool getFolderRecursive();

  public slots:
    void reloadFoldersTree();

  private:
    
    void createTreeFromSource(const QString& dir, QSqlQuery& query);
    
    LibraryFeature* m_pFeature;
    TrackCollection* m_pTrackCollection;
    UserSettingsPointer m_pConfig;
    
    TreeItem* m_pShowAllItem;
    
    bool m_folderRecursive;
};

#endif // LIBRARYFOLDERMODEL_H
