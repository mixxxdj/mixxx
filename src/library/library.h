#pragma once

#include <QFont>
#include <QList>
#include <QObject>
#include <QPointer>

#include "analyzer/trackanalysisscheduler.h"
#include "library/library_decl.h"
#ifdef __ENGINEPRIME__
#include "library/trackset/crate/crateid.h"
#endif
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "util/db/dbconnectionpool.h"
#include "util/parented_ptr.h"

class AnalysisFeature;
class AutoDJFeature;
class BrowseFeature;
class ControlObject;
class CrateFeature;
class LibraryControl;
class LibraryFeature;
class LibraryTableModel;
class KeyboardEventFilter;
class MixxxLibraryFeature;
class PlayerManager;
class PlaylistFeature;
class GroupedPlaylistsFeature;
class RecordingManager;
class SidebarModel;
class TrackCollectionManager;
class WSearchLineEdit;
class WLibrarySidebar;
class WLibrary;
class QAbstractItemModel;

#ifdef __ENGINEPRIME__
namespace mixxx {
class LibraryExporter;
} // namespace mixxx
#endif

// A Library class is a container for all the model-side aspects of the library.
// A library widget can be attached to the Library object by calling bindLibraryWidget.
class Library: public QObject {
    Q_OBJECT

  public:
    Library(QObject* parent,
            UserSettingsPointer pConfig,
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            TrackCollectionManager* pTrackCollectionManager,
            PlayerManager* pPlayerManager,
            RecordingManager* pRecordingManager);
    ~Library() override;

    void stopPendingTasks();

    const mixxx::DbConnectionPoolPtr& dbConnectionPool() const {
        return m_pDbConnectionPool;
    }

    TrackCollectionManager* trackCollectionManager() const;

    TrackAnalysisScheduler::Pointer createTrackAnalysisScheduler(
            int numWorkerThreads,
            AnalyzerModeFlags modeFlags) const;

    void bindSearchboxWidget(WSearchLineEdit* pSearchboxWidget);
    void bindSidebarWidget(WLibrarySidebar* sidebarWidget);
    void bindLibraryWidget(WLibrary* libraryWidget,
                    KeyboardEventFilter* pKeyboard);

    void addFeature(LibraryFeature* feature);

    /// Needed for exposing models to QML
    LibraryTableModel* trackTableModel() const;

    bool isTrackIdInCurrentLibraryView(const TrackId& trackId);

    int getTrackTableRowHeight() const {
        return m_iTrackTableRowHeight;
    }

    const QFont& getTrackTableFont() const {
        return m_trackTableFont;
    }

    bool selectedClickEnabled() const {
        return m_editMetadataSelectedClick;
    }

    //static Library* buildDefaultLibrary();

    static const int kDefaultRowHeightPx;

    void setFont(const QFont& font);
    void setRowHeight(int rowHeight);
    void setEditMetadataSelectedClick(bool enable);

    /// Switches to the internal track collection view
    /// and focuses the search box.
    void searchTracksInCollection();

    /// Triggers a new search in the internal track collection
    /// and shows the results by switching the view.
    void searchTracksInCollection(const QString& query);
    void showAutoDJ();

    static const QString kAutoDJViewName;

    bool requestAddDir(const QString& directory);
    bool requestRemoveDir(const QString& directory, LibraryRemovalType removalType);
    bool requestRelocateDir(const QString& previousDirectory, const QString& newDirectory);

#ifdef __ENGINEPRIME__
    std::unique_ptr<mixxx::LibraryExporter> makeLibraryExporter(QWidget* parent);
#endif

  public slots:
    void slotShowTrackModel(QAbstractItemModel* model);
    void slotSwitchToView(const QString& view);
    void slotLoadTrack(TrackPointer pTrack);
#ifdef __STEM__
    void slotLoadTrackToPlayer(TrackPointer pTrack,
            const QString& group,
            mixxx::StemChannelSelection stemMask,
            bool play);
#else
    void slotLoadTrackToPlayer(TrackPointer pTrack, const QString& group, bool play);
#endif
    void slotLoadLocationToPlayer(const QString& location, const QString& group, bool play);
    void slotRefreshLibraryModels();
    void slotCreatePlaylist();
    void slotCreateCrate();
    void slotSearchInCurrentView();
    void slotSearchInAllTracks();
    void onSkinLoadFinished();
    void slotSaveCurrentViewState() const;
    void slotRestoreCurrentViewState() const;

  signals:
    void showTrackModel(QAbstractItemModel* model, bool restoreState = true);
    void switchToView(const QString& view);
    void loadTrack(TrackPointer pTrack);
#ifdef __STEM__
    void loadTrackToPlayer(TrackPointer pTrack,
            const QString& group,
            mixxx::StemChannelSelection stemMask,
            bool play = false);
#else
    void loadTrackToPlayer(TrackPointer pTrack,
            const QString& group,
            bool play = false);
#endif
    void restoreSearch(const QString&);
    void search(const QString& text);
    void disableSearch();
    void pasteFromSidebar();
    // emit this signal to enable/disable the cover art widget
    void enableCoverArtDisplay(bool);
    void selectTrack(const TrackId&);
    void trackSelected(TrackPointer pTrack);
    void analyzeTracks(const QList<AnalyzerScheduledTrack>& tracks);
#ifdef __ENGINEPRIME__
    void exportLibrary();
    void exportCrate(CrateId crateId);
    void exportPlaylist(int playlistId);
#endif
    void saveModelState();
    void restoreModelState();

    void setTrackTableFont(const QFont& font);
    void setTrackTableRowHeight(int rowHeight);
    void setSelectedClick(bool enable);

    void onTrackAnalyzerProgress(TrackId trackId, AnalyzerProgress analyzerProgress);

  private slots:
      void onPlayerManagerTrackAnalyzerProgress(TrackId trackId, AnalyzerProgress analyzerProgress);
      void onPlayerManagerTrackAnalyzerIdle();

  private:
    const UserSettingsPointer m_pConfig;

    // The Mixxx database connection pool
    const mixxx::DbConnectionPoolPtr m_pDbConnectionPool;

    const QPointer<TrackCollectionManager> m_pTrackCollectionManager;

    parented_ptr<SidebarModel> m_pSidebarModel;
    parented_ptr<LibraryControl> m_pLibraryControl;

    QList<LibraryFeature*> m_features;
    const static QString m_sTrackViewName;
    WLibrary* m_pLibraryWidget;
    MixxxLibraryFeature* m_pMixxxLibraryFeature;
    AutoDJFeature* m_pAutoDJFeature;
    PlaylistFeature* m_pPlaylistFeature;
    GroupedPlaylistsFeature* m_pGroupedPlaylistsFeature;
    CrateFeature* m_pCrateFeature;
    AnalysisFeature* m_pAnalysisFeature;
    BrowseFeature* m_pBrowseFeature;
    QFont m_trackTableFont;
    int m_iTrackTableRowHeight;
    bool m_editMetadataSelectedClick;
    QScopedPointer<ControlObject> m_pKeyNotation;
};
