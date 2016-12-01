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
#include "library/scanner/libraryscanner.h"

class AnalysisFeature;
class CrateFeature;
class KeyboardEventFilter;
class LibraryPaneManager;
class LibraryControl;
class LibraryFeature;
class LibraryTableModel;
class LibrarySidebarExpandedManager;
class LibraryView;
class MixxxLibraryFeature;
class PlaylistFeature;
class PlayerManagerInterface;
class SidebarModel;
class TrackModel;
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
    
    Library(UserSettingsPointer pConfig,
            PlayerManagerInterface* pPlayerManager,
            RecordingManager* pRecordingManager);
    virtual ~Library();
    
    void bindSearchBar(WSearchLineEdit* searchLine, int id);
    void bindSidebarButtons(WButtonBar* sidebar);
    void bindPaneWidget(WLibraryPane* libraryWidget,
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
    
    void createFeatures(UserSettingsPointer pConfig, PlayerManagerInterface *pPlayerManager);
    
    void handleFocus();
    void handlePreselection();
    
    UserSettingsPointer m_pConfig;
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
    QScopedPointer<ControlObject> m_pKeyNotation;
    
    QHash<int, LibraryPaneManager*> m_panes;
    LibraryPaneManager* m_pSidebarExpanded;
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
