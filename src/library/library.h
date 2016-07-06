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
#include <QHash>

#include "preferences/usersettings.h"
#include "track/track.h"
#include "recording/recordingmanager.h"
#include "analysisfeature.h"
#include "library/coverartcache.h"
#include "library/historyfeature.h"
#include "library/scanner/libraryscanner.h"

#include "widget/wtracktableview.h"
#include "widget/wfeatureclickbutton.h"

class CrateFeature;
class KeyboardEventFilter;
class LibraryPaneManager;
class LibraryControl;
class LibraryFeature;
class LibraryTableModel;
class LibrarySidebarExpandedManager;
class MixxxLibraryFeature;
class PlaylistFeature;
class PlayerManagerInterface;
class SidebarModel;
class TrackModel;
class TrackCollection;
class WLibrary;
class WLibrarySidebar;
class WLibraryBreadCrumb;
class WButtonBar;
class WSearchLineEdit;

class Library : public QObject {
    Q_OBJECT
public:
    enum RemovalType {
        LeaveTracksUnchanged = 0,
        HideTracks,
        PurgeTracks
    };
    
    static const int kDefaultRowHeightPx;
    
    Library(QObject* parent,
            UserSettingsPointer pConfig,
            PlayerManagerInterface* pPlayerManager,
            RecordingManager* pRecordingManager);
    virtual ~Library();
    
    void bindSearchBar(WSearchLineEdit* searchLine, int id);
    void bindSidebarWidget(WButtonBar* sidebar);
    void bindPaneWidget(WLibrary* libraryWidget,
                        KeyboardEventFilter* pKeyboard, int paneId);
    void bindSidebarExpanded(WBaseLibrary* expandedPane, 
                             KeyboardEventFilter* pKeyboard);
    void bindBreadCrumb(WLibraryBreadCrumb *pBreadCrumb, int paneId);

    void destroyInterface();
    LibraryView* getActiveView();
    
    void addFeature(LibraryFeature* feature);
    QStringList getDirs();
    
    void paneCollapsed(int paneId);
    void paneUncollapsed(int paneId);

    // TODO(rryan) Transitionary only -- the only reason this is here is so the
    // waveform widgets can signal to a player to load a track. This can be
    // fixed by moving the waveform renderers inside player and connecting the
    // signals directly.
    TrackCollection* getTrackCollection() {
        return m_pTrackCollection;
    }

    inline int getTrackTableRowHeight() const {
        return m_iTrackTableRowHeight;
    }

    inline const QFont& getTrackTableFont() const {
        return m_trackTableFont;
    }
    
    void switchToFeature(LibraryFeature* pFeature);
    void showBreadCrumb(TreeItem* pTree);
    void restoreSearch(const QString& text);

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
    void slotPaneFocused(LibraryPaneManager *pPane);
    
    // Updates with the focus feature
    void slotUpdateFocus(LibraryFeature* pFeature);

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
    LibraryPaneManager *getPane(int paneId);
    LibraryPaneManager* getFocusedPane();
    
    void createFeatures(UserSettingsPointer pConfig, PlayerManagerInterface *pPlayerManager);
    void setFocusedPane();
    
    void handleFocus();
    
    UserSettingsPointer m_pConfig;
    SidebarModel* m_pSidebarModel;
    TrackCollection* m_pTrackCollection;
    MixxxLibraryFeature* m_pMixxxLibraryFeature;
    PlaylistFeature* m_pPlaylistFeature;
    CrateFeature* m_pCrateFeature;
    AnalysisFeature* m_pAnalysisFeature;
    LibraryControl* m_pLibraryControl;
    RecordingManager* m_pRecordingManager;
    LibraryScanner m_scanner;
    QFont m_trackTableFont;
    int m_iTrackTableRowHeight;
    
    QHash<int, LibraryPaneManager*> m_panes;
    LibraryPaneManager* m_pSidebarExpanded;
    QList<LibraryFeature*> m_features;
    QSet<int> m_collapsedPanes;
    
    // Can be any integer as it's used with a HashMap
    int m_focusedPane;
};

#endif /* LIBRARY_H */
