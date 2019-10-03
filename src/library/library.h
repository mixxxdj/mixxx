// library.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

// A Library class is a container for all the model-side aspects of the library.
// A library widget can be attached to the Library object by calling bindWidget.

#ifndef LIBRARY_H
#define LIBRARY_H

#include <QList>
#include <QObject>
#include <QAbstractItemModel>
#include <QFont>

#include "preferences/usersettings.h"
#include "track/globaltrackcache.h"
#include "recording/recordingmanager.h"
#include "analysisfeature.h"
#include "library/coverartcache.h"
#include "library/setlogfeature.h"
#include "library/scanner/libraryscanner.h"
#include "util/db/dbconnectionpool.h"

class TrackModel;
class TrackCollection;
class SidebarModel;
class LibraryFeature;
class LibraryTableModel;
class WLibrarySidebar;
class WLibrary;
class MixxxLibraryFeature;
class PlaylistFeature;
class CrateFeature;
class LibraryControl;
class KeyboardEventFilter;
class PlayerManager;

class ExternalTrackCollection;

class Library: public QObject,
    public virtual /*implements*/ GlobalTrackCacheSaver {
    Q_OBJECT

  public:
    static const QString kConfigGroup;

    static const ConfigKey kConfigKeyRepairDatabaseOnNextRestart;

    Library(QObject* parent,
            UserSettingsPointer pConfig,
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            PlayerManager* pPlayerManager,
            RecordingManager* pRecordingManager);
    ~Library() override;

    void stopFeatures();

    mixxx::DbConnectionPoolPtr dbConnectionPool() const {
        return m_pDbConnectionPool;
    }

    TrackCollection& trackCollection() {
        DEBUG_ASSERT(m_pTrackCollection);
        return *m_pTrackCollection;
    }

    void bindWidget(WLibrary* libraryWidget,
                    KeyboardEventFilter* pKeyboard);
    void bindSidebarWidget(WLibrarySidebar* sidebarWidget);

    void addFeature(LibraryFeature* feature);
    QStringList getDirs();

    inline int getTrackTableRowHeight() const {
        return m_iTrackTableRowHeight;
    }

    inline const QFont& getTrackTableFont() const {
        return m_trackTableFont;
    }

    //static Library* buildDefaultLibrary();

    enum RemovalType {
        LeaveTracksUnchanged = 0,
        HideTracks,
        PurgeTracks
    };

    static const int kDefaultRowHeightPx;

    void setFont(const QFont& font);
    void setRowHeight(int rowHeight);
    void setEditMedatataSelectedClick(bool enable);

    void relocateDirectory(QString oldDir, QString newDir);

    void purgeTracks(const QList<TrackId>& trackIds);
    void purgeAllTracks(const QDir& rootDir);

  public slots:
    void slotShowTrackModel(QAbstractItemModel* model);
    void slotSwitchToView(const QString& view);
    void slotLoadTrack(TrackPointer pTrack);
    void slotLoadTrackToPlayer(TrackPointer pTrack, QString group, bool play);
    void slotLoadLocationToPlayer(QString location, QString group);
    void slotRestoreSearch(const QString& text);
    void slotDisableSearch();
    void slotRefreshLibraryModels();
    void slotCreatePlaylist();
    void slotCreateCrate();
    void slotRequestAddDir(QString directory);
    void slotRequestRemoveDir(QString directory, Library::RemovalType removalType);
    void slotRequestRelocateDir(QString previousDirectory, QString newDirectory);
    void onSkinLoadFinished();

    void scan() {
        m_scanner.scan();
    }
    void slotScanTrackAdded(TrackPointer pTrack);
    void slotScanTracksUpdated(QSet<TrackId> updatedTrackIds);
    void slotScanTracksReplaced(QList<QPair<TrackRef, TrackRef>> replacedTracks);

  signals:
    void showTrackModel(QAbstractItemModel* model);
    void switchToView(const QString& view);
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, QString group, bool play = false);
    void restoreSearch(const QString&);
    void search(const QString& text);
    void disableSearch();
    // emit this signal to enable/disable the cover art widget
    void enableCoverArtDisplay(bool);
    void trackSelected(TrackPointer pTrack);

    void setTrackTableFont(const QFont& font);
    void setTrackTableRowHeight(int rowHeight);
    void setSelectedClick(bool enable);

    // Emitted when a library scan starts and finishes.
    void scanStarted();
    void scanFinished();

  private slots:
      void onPlayerManagerTrackAnalyzerProgress(TrackId trackId, AnalyzerProgress analyzerProgress);
      void onPlayerManagerTrackAnalyzerIdle();

  private:
    // Callback for GlobalTrackCache
    void saveEvictedTrack(Track* pTrack) noexcept override;

    const UserSettingsPointer m_pConfig;

    // The Mixxx database connection pool
    const mixxx::DbConnectionPoolPtr m_pDbConnectionPool;

    SidebarModel* m_pSidebarModel;
    TrackCollection* m_pTrackCollection;

    QList<ExternalTrackCollection*> m_externalTrackCollections;

    LibraryControl* m_pLibraryControl;
    QList<LibraryFeature*> m_features;
    const static QString m_sTrackViewName;
    const static QString m_sAutoDJViewName;
    MixxxLibraryFeature* m_pMixxxLibraryFeature;
    PlaylistFeature* m_pPlaylistFeature;
    CrateFeature* m_pCrateFeature;
    AnalysisFeature* m_pAnalysisFeature;
    LibraryScanner m_scanner;
    QFont m_trackTableFont;
    int m_iTrackTableRowHeight;
    bool m_editMetadataSelectedClick;
    QScopedPointer<ControlObject> m_pKeyNotation;
};

#endif /* LIBRARY_H */
