#pragma once

#include <QFuture>
#include <QFutureWatcher>
#include <QSqlDatabase>
#include <QStringListModel>
#include <QtConcurrentRun>
#include <fstream>

#include "library/baseexternallibraryfeature.h"
#include "library/baseexternaltrackmodel.h"
#include "library/serato/seratoplaylistmodel.h"
#include "library/tracksuggestion/dlgtracksuggestion.h"
#include "library/treeitemmodel.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"

class Track;

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
    void onSuggestionFileParsed();
    void slotStartFetchingViaButton();
    void slotUpdateTrackModelAfterSuccess(const QString& filePath);
    void slotTrackSelected(TrackId trackId);

  private:
    void parseSuggestionFile();
    void clearTable(const QString& table_name);
    void emitTrackPropertiesToDialog(TrackPointer pTrack);
    QString formatRootViewHtml() const;

    bool m_cancelImport;
    parented_ptr<TreeItemModel> m_pSidebarModel;
    parented_ptr<SuggestionFetcher> m_pSuggestionFetcher;

    BaseExternalTrackModel* m_pSuggestionTrackModel;
    BaseExternalPlaylistModel* m_pSuggestionPlaylistModel;
    QSqlDatabase m_database;
    QSharedPointer<BaseTrackCache> m_trackSource;
    QFutureWatcher<void> m_future_watcher;
    QFuture<void> m_future;

    TreeItem* treeItemDeckOne;
    TreeItem* treeItemDeckTwo;
    TreeItem* treeItemDeckThree;
    TreeItem* treeItemDeckFour;
    TreeItem* treeItemSelectedTrack;

    TrackId m_selectedTrackId;
    TrackPointer m_pTrack;

    QString m_suggestionFile;
    QString m_title;
};
