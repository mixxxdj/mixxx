// library.h
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

// A Library class is a container for all the model-side aspects of the library.
// A library widget can be attached to the Library object by calling bindWidget.

#ifndef LIBRARY_H
#define LIBRARY_H

#include <QAbstractItemModel>
#include <QFont>
#include <QHash>
#include <QList>
#include <QObject>

#include "library/scanner/libraryscanner.h"
#include "preferences/usersettings.h"
#include "recording/recordingmanager.h"
#include "track/track.h"
#include "util/parented_ptr.h"
#include "util/memory.h"
#include "util/db/dbconnectionpool.h"

class AnalysisFeature;
class CrateFeature;
class KeyboardEventFilter;
class LibraryPaneManager;
class LibraryControl;
class LibraryFeature;
class LibrarySidebarExpandedManager;
class LibraryView;
class TracksFeature;
class PlaylistFeature;
class PlayerManagerInterface;
class TrackCollection;
class WBaseLibrary;
class WLibraryPane;
class WLibrarySidebar;
class WLibraryBreadCrumb;
class WButtonBar;
class WSearchLineEdit;
class TreeItem;

class Library : public QObject {
    Q_OBJECT
public:
    enum RemovalType {
        LeaveTracksUnchanged = 0,
        HideTracks,
        PurgeTracks
    };

    static const int kDefaultRowHeightPx;

    static const QString kConfigGroup;

    static const ConfigKey kConfigKeyRepairDatabaseOnNextRestart;

    Library(UserSettingsPointer pConfig,
            mixxx::DbConnectionPoolPtr pDbConnectionPool,
            PlayerManagerInterface* pPlayerManager,
            RecordingManager* pRecordingManager);
    ~Library() override;

    mixxx::DbConnectionPoolPtr dbConnectionPool() const {
        return m_pDbConnectionPool;
    }

    void bindSearchBar(WSearchLineEdit* searchLine, int id);
    void bindSidebarButtons(WButtonBar* sidebar);
    void bindPaneWidget(WLibraryPane *libraryWidget,
                        KeyboardEventFilter* pKeyboard, int paneId);
    void bindSidebarExpanded(WBaseLibrary *expandedPane,
                             KeyboardEventFilter* pKeyboard);
    void bindBreadCrumb(WLibraryBreadCrumb *pBreadCrumb, int paneId);

    void destroyInterface();
    LibraryView* getActiveView();

    void addFeature(LibraryFeature* feature);
    QStringList getDirs();

    void paneCollapsed(int paneId);
    void paneUncollapsed(int paneId);

    inline int getTrackTableRowHeight() const {
        return m_iTrackTableRowHeight;
    }

    inline const QFont& getTrackTableFont() const {
        return m_trackTableFont;
    }

    void switchToFeature(LibraryFeature* pFeature);
    void showBreadCrumb(int paneId, TreeItem* pTree);
    void showBreadCrumb(int paneId, const QString& text, const QIcon& icon);
    void restoreSearch(int paneId, const QString& text);
    void restoreSaveButton(int paneId);
    void paneFocused(LibraryPaneManager *pPane);
    void panePreselected(LibraryPaneManager* pPane, bool value);

    int getFocusedPaneId();
    int getPreselectedPaneId();

    void focusSearch();

  public slots:

    void slotActivateFeature(LibraryFeature* pFeature);
    void slotHoverFeature(LibraryFeature* pFeature);

    // Updates the focus from the feature before changing the view
    void slotLoadTrack(TrackPointer pTrack);
    void slotLoadTrackToPlayer(TrackPointer pTrack, QString group, bool play);
    void slotLoadLocationToPlayer(QString location, QString group);
    void slotRefreshLibraryModels();
    void slotCreatePlaylist();
    void slotCreateCrate();
    void slotRequestAddDir(QString directory);
    void slotRequestRemoveDir(QString directory, Library::RemovalType removalType);
    void slotRequestRelocateDir(QString previousDirectory, QString newDirectory);
    void onSkinLoadFinished();
    void slotSetTrackTableFont(const QFont& font);
    void slotSetTrackTableRowHeight(int rowHeight);

    void slotSetHoveredFeature(LibraryFeature* pFeature);
    void slotResetHoveredFeature(LibraryFeature* pFeature);
    void slotSetFocusedFeature(LibraryFeature* pFeature);
    void slotResetFocusedFeature(LibraryFeature* pFeature);

    void scan() {
        m_scanner.scan();
    }

  signals:
    void loadTrack(TrackPointer pTrack);
    void loadTrackToPlayer(TrackPointer pTrack, QString group, bool play = false);

    // emit this signal to enable/disable the cover art widget
    void enableCoverArtDisplay(bool);
    void trackSelected(TrackPointer pTrack);

    void setTrackTableFont(const QFont& font);
    void setTrackTableRowHeight(int rowHeight);

    // Emitted when a library scan starts and finishes.
    void scanStarted();
    void scanFinished();

  private:

    // If the pane exists returns it, otherwise it creates the pane
    LibraryPaneManager* getOrCreatePane(int paneId);
    LibraryPaneManager* getFocusedPane();
    LibraryPaneManager* getPreselectedPane();

    void createTrackCache();
    void createFeatures(
            UserSettingsPointer pConfig,
            PlayerManagerInterface *pPlayerManager,
            RecordingManager* pRecordingManager);

    void handleFocus();
    void handlePreselection();

    const UserSettingsPointer m_pConfig;

    // The Mixxx database connection pool
    const mixxx::DbConnectionPoolPtr m_pDbConnectionPool;

    TrackCollection* m_pTrackCollection;
    TracksFeature* m_pTracksFeature;
    PlaylistFeature* m_pPlaylistFeature;
    CrateFeature* m_pCrateFeature;
    AnalysisFeature* m_pAnalysisFeature;
    LibraryControl* m_pLibraryControl;
    LibraryScanner m_scanner;
    QFont m_trackTableFont;
    int m_iTrackTableRowHeight;
    QScopedPointer<ControlObject> m_pKeyNotation;

    QHash<int, LibraryPaneManager*> m_panes;
    std::unique_ptr<LibrarySidebarExpandedManager> m_pSidebarExpanded;
    QList<LibraryFeature*> m_features;
    QSet<int> m_collapsedPanes;
    QHash<int, LibraryFeature*> m_savedFeatures;
    // Used to show the preselected pane when the mouse is over the button
    LibraryFeature* m_hoveredFeature;
    LibraryFeature* m_focusedFeature;

    // Can be any integer as it's used with a HashMap
    int m_focusedPaneId;
    int m_preselectedPane;
    int m_previewPreselectedPane;
};

#endif /* LIBRARY_H */
