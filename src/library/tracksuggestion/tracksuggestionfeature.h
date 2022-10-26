#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QSqlDatabase>
#include <QStringListModel>
#include <QtConcurrentRun>
#include <fstream>

#include "library/baseexternallibraryfeature.h"
#include "library/baseexternaltrackmodel.h"
#include "library/tracksuggestion/dlgtracksuggestion.h"
#include "library/treeitemmodel.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"

class TrackSuggestionFeature : public BaseExternalLibraryFeature {
    Q_OBJECT
  public:
    TrackSuggestionFeature(Library* pLibrary,
            UserSettingsPointer pConfig);
    ~TrackSuggestionFeature() override;

    QVariant title() override;
    static bool isSupported();
    void bindLibraryWidget(WLibrary* libraryWidget,
            KeyboardEventFilter* keyboard) override;

    TreeItemModel* sidebarModel() const override;

  public slots:
    void activate() override;
    void activateChild(const QModelIndex& index) override;

  private slots:
    void htmlLinkClicked(const QUrl& link);
    void slotTrackChanged(const QString& group, TrackPointer pNewTrack, TrackPointer pOldTrack);
    void playerInfoTrackLoaded(const QString& group, TrackPointer pNewTrack);
    void slotStartFetchingViaButton();
    void slotUpdateTrackModelAfterSuccess(const QList<QMap<QString, QString>>& suggestions);
    void slotTrackSelected(TrackId trackId);

  private:
    BaseSqlTableModel* getPlaylistModelForPlaylist(const QString& playlist) override;
    void emitTrackPropertiesToDialog(TrackPointer pTrack);
    bool lookTrackHasSuggestions(TrackId trackId);
    QString formatRootViewHtml() const;
    QString formatNoSuggestionAvailableHtml() const;

    bool m_isFetchingSuccess;
    parented_ptr<TreeItemModel> m_pSidebarModel;
    parented_ptr<SuggestionFetcher> m_pSuggestionFetcher;

    BaseExternalTrackModel* m_pSuggestionTrackModel;
    BaseExternalPlaylistModel* m_pSuggestionPlaylistModel;
    BaseExternalPlaylistModel* m_DummyModel;
    QSqlDatabase m_database;
    QSharedPointer<BaseTrackCache> m_trackSource;

    TreeItem* treeItemDeckOne;
    TreeItem* treeItemDeckTwo;
    TreeItem* treeItemDeckThree;
    TreeItem* treeItemDeckFour;
    TreeItem* treeItemSelectedTrack;

    TrackId m_selectedTrackId;
    TrackPointer m_pTrack;

    QString m_suggestionFile;
    QString m_title;
    QList<QMap<QString, QString>> m_suggestions;
};
