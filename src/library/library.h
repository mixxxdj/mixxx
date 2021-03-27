#pragma once

#include <QAbstractItemModel>
#include <QFont>
#include <QList>
#include <QObject>
#include <QPointer>

#include "analyzer/analyzerprogress.h"
#ifdef __ENGINEPRIME__
#include "library/trackset/crate/crateid.h"
#endif
#include "preferences/usersettings.h"
#include "track/track_decl.h"
#include "track/trackid.h"
#include "util/db/dbconnectionpool.h"
#include "util/parented_ptr.h"

class AnalysisFeature;
class ControlObject;
class CrateFeature;
class ExternalTrackCollection;
class LibraryControl;
class LibraryFeature;
class LibraryTableModel;
class KeyboardEventFilter;
class MixxxLibraryFeature;
class PlayerManager;
class PlaylistFeature;
class RecordingManager;
class SidebarModel;
class TrackCollection;
class TrackCollectionManager;
class TrackModel;
class WSearchLineEdit;
class WLibrarySidebar;
class WLibrary;

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
    static const QString kConfigGroup;

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

    TrackCollectionManager* trackCollections() const;

    // Deprecated: Obtain directly from TrackCollectionManager
    TrackCollection& trackCollection();

    void bindSearchboxWidget(WSearchLineEdit* pSearchboxWidget);
    void bindSidebarWidget(WLibrarySidebar* sidebarWidget);
    void bindLibraryWidget(WLibrary* libraryWidget,
                    KeyboardEventFilter* pKeyboard);

    void addFeature(LibraryFeature* feature);
    QStringList getDirs();

    int getTrackTableRowHeight() const {
        return m_iTrackTableRowHeight;
    }

    const QFont& getTrackTableFont() const {
        return m_trackTableFont;
    }

    //static Library* buildDefaultLibrary();

    enum class RemovalType {
        KeepTracks,
        HideTracks,
        PurgeTracks
    };

    static const int kDefaultRowHeightPx;

    void setFont(const QFont& font);
    void setRowHeight(int rowHeight);
    void setEditMedatataSelectedClick(bool enable);

    /// Triggers a new search in the internal track collection
    /// and shows the results by switching the view.
    void searchTracksInCollection(const QString& query);

#ifdef __ENGINEPRIME__
    std::unique_ptr<mixxx::LibraryExporter> makeLibraryExporter(QWidget* parent);
#endif

  public slots:
    void slotShowTrackModel(QAbstractItemModel* model);
    void slotSwitchToView(const QString& view);
    void slotLoadTrack(TrackPointer pTrack);
    void slotLoadTrackToPlayer(TrackPointer pTrack, const QString& group, bool play);
    void slotLoadLocationToPlayer(const QString& location, const QString& group);
    void slotRefreshLibraryModels();
    void slotCreatePlaylist();
    void slotCreateCrate();
    void slotRequestAddDir(const QString& directory);
    void slotRequestRemoveDir(const QString& directory, Library::RemovalType removalType);
    void slotRequestRelocateDir(const QString& previousDirectory, const QString& newDirectory);
    void onSkinLoadFinished();

  signals:
    void showTrackModel(QAbstractItemModel* model);
    void switchToView(const QString& view);
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, const QString& group, bool play = false);
    void restoreSearch(const QString&);
    void search(const QString& text);
    void disableSearch();
    // emit this signal to enable/disable the cover art widget
    void enableCoverArtDisplay(bool);
    void trackSelected(TrackPointer pTrack);
#ifdef __ENGINEPRIME__
    void exportLibrary();
    void exportCrate(CrateId crateId);
#endif

    void setTrackTableFont(const QFont& font);
    void setTrackTableRowHeight(int rowHeight);
    void setSelectedClick(bool enable);

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
    const static QString m_sAutoDJViewName;
    MixxxLibraryFeature* m_pMixxxLibraryFeature;
    PlaylistFeature* m_pPlaylistFeature;
    CrateFeature* m_pCrateFeature;
    AnalysisFeature* m_pAnalysisFeature;
    QFont m_trackTableFont;
    int m_iTrackTableRowHeight;
    bool m_editMetadataSelectedClick;
    QScopedPointer<ControlObject> m_pKeyNotation;
};
