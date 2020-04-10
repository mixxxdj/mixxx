#pragma once

#include <QMenu>

#include "library/dao/playlistdao.h"
#include "library/dlgtagfetcher.h"
#include "library/dlgtrackinfo.h"
#include "library/trackmodel.h"

typedef QList<TrackId> TrackIdList;
typedef QList<TrackPointer> TrackPointerList;

class ControlProxy;
class DlgTagFetcher;
class DlgTrackInfo;
class ExternalTrackCollection;
class QAction;
class QWidget;
class TrackCollectionManager;
class WColorPickerAction;
class WCoverArtMenu;

class WTrackMenu : public QMenu {
    Q_OBJECT
  public:
    enum Filter {
        All = 0,
        AutoDJ = 1,
        LoadTo = 1 << 1,
        Playlist = 1 << 2,
        Crate = 1 << 3,
        Remove = 1 << 4,
        Metadata = 1 << 5,
        Reset = 1 << 6,
        BPM = 1 << 7,
        Color = 1 << 8,
        HideUnhidePurge = 1 << 9,
        FileBrowser = 1 << 10,
        Properties = 1 << 11,
        IndependentFilters = AutoDJ |
                Playlist |
                Crate |
                Metadata |
                Reset |
                BPM |
                Color |
                FileBrowser |
                Properties
    };
    Q_DECLARE_FLAGS(Filters, Filter)

    WTrackMenu(QWidget* parent,
            UserSettingsPointer pConfig,
            TrackCollectionManager* pTrackCollectionManager,
            Filters flags = Filter::IndependentFilters,
            TrackModel* trackModel = nullptr);
    ~WTrackMenu() {
    }

    void loadTrack(TrackId trackId);
    void loadTrack(QModelIndex index);
    void loadTracks(TrackIdList trackList);
    void loadTracks(QModelIndexList indexList);

    // Override default popup method from class QMenu
    void popup(const QPoint& pos, QAction* at = nullptr);

  signals:
    void loadTrackToPlayer(TrackPointer pTrack, QString group, bool play = false);

  private slots:
    // File
    void slotOpenInFileBrowser();

    // Row color
    void slotColorPicked(mixxx::RgbColor::optional_t color);

    // Reset
    void slotClearBeats();
    void slotClearPlayCount();
    void slotClearMainCue();
    void slotClearHotCues();
    void slotClearIntroCue();
    void slotClearOutroCue();
    void slotClearLoop();
    void slotClearKey();
    void slotClearReplayGain();
    void slotClearWaveform();
    void slotClearAllMetadata();

    // BPM
    void slotLockBpm();
    void slotUnlockBpm();
    void slotScaleBpm(int);

    // Info and metadata
    void slotNextTrackInfo();
    void slotNextDlgTagFetcher();
    void slotPrevTrackInfo();
    void slotPrevDlgTagFetcher();
    void slotShowTrackInTagFetcher(TrackPointer track);
    void slotTrackInfoClosed();
    void slotTagFetcherClosed();
    void slotShowTrackInfo();
    void slotShowDlgTagFetcher();
    void slotImportTrackMetadataFromFileTags();
    void slotExportTrackMetadataIntoFileTags();
    void slotUpdateExternalTrackCollection(ExternalTrackCollection* externalTrackCollection);

    // Playlist and crate
    void slotPopulatePlaylistMenu();
    void slotPopulateCrateMenu();
    void addSelectionToNewCrate();

    // Auto DJ
    void slotAddToAutoDJBottom();
    void slotAddToAutoDJTop();
    void slotAddToAutoDJReplace();

    // Cover
    void slotCoverInfoSelected(const CoverInfoRelative& coverInfo);
    void slotReloadCoverArt();

    // Library management
    void slotRemove();
    void slotHide();
    void slotUnhide();
    void slotPurge();

  private:
    TrackIdList getTrackIds() const;
    TrackPointerList getTrackPointers() const;
    QModelIndexList getTrackIndices() const;
    TrackModel* getTrackModel() const;

    void createMenus();
    void createActions();
    void setupActions();
    void updateMenus();

    bool modelHasCapabilities(TrackModel::CapabilitiesFlags capabilities) const;
    bool optionIsEnabled(Filter flag) const;

    void addSelectionToPlaylist(int iPlaylistId);
    void updateSelectionCrates(QWidget* pWidget);

    void showTrackInfo(QModelIndex index);
    void showTrackInfo(TrackPointer pTrack);
    void showDlgTagFetcher(QModelIndex index);
    void showDlgTagFetcher(TrackPointer pTrack);

    void addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc);

    void lockBpm(bool lock);

    void loadSelectionToGroup(QString group, bool play = false);
    void clearTrackSelection();

    // Selected tracks
    TrackPointerList m_pTrackPointerList;
    QModelIndexList m_pTrackIndexList;

    TrackModel* m_pTrackModel{};

    const ControlProxy* m_pNumSamplers{};
    const ControlProxy* m_pNumDecks{};
    const ControlProxy* m_pNumPreviewDecks{};

    // Submenus
    QMenu* m_pLoadToMenu{};
    QMenu* m_pDeckMenu{};
    QMenu* m_pSamplerMenu{};
    QMenu* m_pPlaylistMenu{};
    QMenu* m_pCrateMenu{};
    QMenu* m_pMetadataMenu{};
    QMenu* m_pMetadataUpdateExternalCollectionsMenu{};
    QMenu* m_pClearMetadataMenu{};
    QMenu* m_pBPMMenu{};
    QMenu* m_pColorMenu{};
    WCoverArtMenu* m_pCoverMenu{};

    // Reload Track Metadata Action:
    QAction* m_pImportMetadataFromFileAct{};
    QAction* m_pImportMetadataFromMusicBrainzAct{};

    // Save Track Metadata Action:
    QAction* m_pExportMetadataAct{};

    // Load Track to PreviewDeck
    QAction* m_pAddToPreviewDeck{};

    // Send to Auto-DJ Action
    QAction* m_pAutoDJBottomAct{};
    QAction* m_pAutoDJTopAct{};
    QAction* m_pAutoDJReplaceAct{};

    // Remove from table
    QAction* m_pRemoveAct{};
    QAction* m_pRemovePlaylistAct{};
    QAction* m_pRemoveCrateAct{};
    QAction* m_pHideAct{};
    QAction* m_pUnhideAct{};
    QAction* m_pPurgeAct{};

    // Show track-editor action
    QAction* m_pPropertiesAct{};

    // Open file in default file browser
    QAction* m_pFileBrowserAct{};

    // BPM feature
    QAction* m_pBpmLockAction{};
    QAction* m_pBpmUnlockAction{};
    QAction* m_pBpmDoubleAction{};
    QAction* m_pBpmHalveAction{};
    QAction* m_pBpmTwoThirdsAction{};
    QAction* m_pBpmThreeFourthsAction{};
    QAction* m_pBpmFourThirdsAction{};
    QAction* m_pBpmThreeHalvesAction{};

    // Track color
    WColorPickerAction* m_pColorPickerAction{};

    // Clear track metadata actions
    QAction* m_pClearBeatsAction{};
    QAction* m_pClearPlayCountAction{};
    QAction* m_pClearMainCueAction{};
    QAction* m_pClearHotCuesAction{};
    QAction* m_pClearIntroCueAction{};
    QAction* m_pClearOutroCueAction{};
    QAction* m_pClearLoopAction{};
    QAction* m_pClearWaveformAction{};
    QAction* m_pClearKeyAction{};
    QAction* m_pClearReplayGainAction{};
    QAction* m_pClearAllMetadataAction{};

    const UserSettingsPointer m_pConfig;
    TrackCollectionManager* const m_pTrackCollectionManager;

    QScopedPointer<DlgTrackInfo> m_pTrackInfo;
    QScopedPointer<DlgTagFetcher> m_pTagFetcher;

    QModelIndex currentTrackInfoIndex;

    struct UpdateExternalTrackCollection {
        QPointer<ExternalTrackCollection> externalTrackCollection;
        QAction* action{};
    };
    QList<UpdateExternalTrackCollection> m_updateInExternalTrackCollections;

    bool m_bPlaylistMenuLoaded;
    bool m_bCrateMenuLoaded;

    // Filter available options
    const Filters m_eFilters;
    const Filters m_eIndependentFilters;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WTrackMenu::Filters)
