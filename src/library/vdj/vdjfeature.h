#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QStringListModel>
#include <QXmlStreamReader>
#include <QtConcurrentRun>
#include <atomic>

#include "library/baseexternallibraryfeature.h"
#include "library/baseexternalplaylistmodel.h"
#include "library/baseexternaltrackmodel.h"
#include "library/treeitemmodel.h"

class TrackCollectionManager;
class BaseExternalPlaylistModel;

class VdjPlaylistModel : public BaseExternalPlaylistModel {
    Q_OBJECT
  public:
    VdjPlaylistModel(QObject* parent,
            TrackCollectionManager* pTrackCollectionManager,
            QSharedPointer<BaseTrackCache> trackSource);
    TrackPointer getTrack(const QModelIndex& index) const override;
    bool isColumnHiddenByDefault(int column) override;

protected:
    void initSortColumnMapping() override;
};


class VdjFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    VdjFeature(Library* pLibrary, UserSettingsPointer pConfig);
    virtual ~VdjFeature();

    QVariant title() override;
    static bool isSupported();
    
    void bindLibraryWidget(WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;
    TreeItemModel* sidebarModel() const override;

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;
    void refreshLibraryModels();
    void onVdjDevicesFound();
    void onTracksFound();

  private slots:  
    void htmlLinkClicked(const QUrl& link);

  private:
    QString formatRootViewHtml() const;
    std::unique_ptr<BaseSqlTableModel> createPlaylistModelForPlaylist(
            const QString& playlist) override;

    // private fields
    parented_ptr<TreeItemModel> m_pSidebarModel;
    parented_ptr<VdjPlaylistModel> m_pVdjPlaylistModel;

    QFutureWatcher<QList<TreeItem*>> m_devicesFutureWatcher;
    QFuture<QList<TreeItem*>> m_devicesFuture;

    QFutureWatcher<QString> m_tracksFutureWatcher;
    QFuture<QString> m_tracksFuture;

    bool m_isActivated;
    std::atomic<bool> m_cancelImport;
    QString m_title;

    QSharedPointer<BaseTrackCache> m_trackSource;
};
