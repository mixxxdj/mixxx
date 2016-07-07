// mixxxlibraryfeature.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#ifndef MIXXXLIBRARYFEATURE_H
#define MIXXXLIBRARYFEATURE_H

#include <QStringListModel>
#include <QUrl>
#include <QVariant>
#include <QIcon>
#include <QModelIndex>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QStackedWidget>
#include <QPointer>

#include "library/libraryfeature.h"
#include "library/dao/trackdao.h"
#include "treeitemmodel.h"
#include "preferences/usersettings.h"

class DlgHidden;
class DlgMissing;
class Library;
class BaseTrackCache;
class LibraryTableModel;
class TrackCollection;
class WTrackTableView;
class HiddenTableModel;
class MissingTableModel;

class MixxxLibraryFeature : public LibraryFeature {
    Q_OBJECT
    
  public:
    MixxxLibraryFeature(UserSettingsPointer pConfig,
                        Library* pLibrary,
                        QObject* parent,
                        TrackCollection* pTrackCollection);
    virtual ~MixxxLibraryFeature();

    QVariant title();
    QIcon getIcon();
    
    bool dropAccept(QList<QUrl> urls, QObject* pSource);
    bool dragMoveAccept(QUrl url);
    TreeItemModel* getChildModel();

  public slots:
    void activate();
    void activateChild(const QModelIndex& index);
    void refreshLibraryModels();
    
    void selectAll();
    
    
  signals:
    void unhideHidden();
    void purgeHidden();
    void purgeMissing();

  private:
    enum Panes {
        MixxxLibrary = 1,
        Hidden = 2,
        Missing = 3
    };
    
    const QString kLibraryTitle;
    
    QSharedPointer<BaseTrackCache> m_pBaseTrackCache;
    LibraryTableModel* m_pLibraryTableModel;
    TreeItemModel m_childModel;
    TrackDAO& m_trackDao;
    TrackCollection* m_pTrackCollection;
};

#endif /* MIXXXLIBRARYFEATURE_H */
