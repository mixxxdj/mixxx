#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QStringListModel>
#include <QtConcurrentRun>
#include <QtSql>

#include "library/baseexternallibraryfeature.h"
#include "library/clementine/clementinedbconnection.h"
#include "library/clementine/clementineplaylistmodel.h"
#include "library/treeitem.h"
#include "library/treeitemmodel.h"

class ClementineFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    ClementineFeature(Library* pLibrary, UserSettingsPointer pConfig);
    ~ClementineFeature() override;
    static bool isSupported();

    QVariant title() override;

    TreeItemModel* sidebarModel() const override;

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;

  private:
    void appendTrackIdsFromRightClickIndex(QList<TrackId>* trackIds, QString* pPlaylist) override;

    //a new DB connection for the worker thread
    std::shared_ptr<ClementineDbConnection> m_connection;
    bool m_isActivated;

    parented_ptr<ClementinePlaylistModel> m_pClementinePlaylistModel;
    parented_ptr<TreeItemModel> m_pSidebarModel;
    QStringList m_playlists;

    QFuture<TreeItem*> m_future;
    QString m_title;
    QIcon m_icon;
};
