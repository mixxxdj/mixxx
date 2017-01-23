#ifndef LIBRARYFOLDERMODEL_H
#define LIBRARYFOLDERMODEL_H

#include <QSqlQuery>

#include "library/features/mixxxlibrary/mixxxlibrarytreemodel.h"

class LibraryFeature;
class TrackCollection;

const QString LIBRARYFOLDERMODEL_FOLDER = "$FOLDER$";
const QString LIBRARYFOLDERMODEL_RECURSIVE = "FolderRecursive";

class LibraryFolderModel : public MixxxLibraryTreeModel
{
  public:
    LibraryFolderModel(LibraryFeature* pFeature, 
                       TrackCollection* pTrackCollection, 
                       UserSettingsPointer pConfig, 
                       QObject* parent = nullptr);
    
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
    virtual QVariant data(const QModelIndex &index, int role) const;

  protected:
    void createTracksTree() override;
    QString getGroupingOptions() override;
    
  private:
    void createTreeForLibraryDir(const QString& dir, QSqlQuery& query);
    
    bool m_folderRecursive;
    bool m_showFolders;
};

#endif // LIBRARYFOLDERMODEL_H
