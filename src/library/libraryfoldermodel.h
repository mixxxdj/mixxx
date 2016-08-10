#ifndef LIBRARYFOLDERMODEL_H
#define LIBRARYFOLDERMODEL_H

#include <QSqlQuery>

#include "library/treeitemmodel.h"
#include "preferences/usersettings.h"

class LibraryFeature;
class TrackCollection;

class LibraryFolderModel : public TreeItemModel
{
  public:
    LibraryFolderModel(LibraryFeature* pFeature, 
                       TrackCollection* pTrackCollection, 
                       UserSettingsPointer pConfig, 
                       QObject* parent = nullptr);
    
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
    virtual QVariant data(const QModelIndex &index, int role) const;
    
  public slots:
    void reloadTree();

  private:
    
    void createTreeForLibraryDir(const QString& dir, QSqlQuery& query);
    
    LibraryFeature* m_pFeature;
    TrackCollection* m_pTrackCollection;
    UserSettingsPointer m_pConfig;
    
    TreeItem* m_pShowAllItem;
    
    bool m_folderRecursive;
};

#endif // LIBRARYFOLDERMODEL_H
