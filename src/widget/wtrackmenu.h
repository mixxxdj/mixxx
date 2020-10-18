#pragma once

#include <QMenu>
#include <QModelIndex>
#include <QPointer>
#include <memory>

#include "library/coverart.h"
#include "library/dao/playlistdao.h"
#include "library/trackprocessing.h"
#include "preferences/usersettings.h"
#include "track/trackref.h"
#include "util/color/rgbcolor.h"

class ControlProxy;
class DlgTagFetcher;
class DlgTrackInfo;
class ExternalTrackCollection;
class TrackCollectionManager;
class TrackModel;
class WColorPickerAction;
class WCoverArtMenu;

/// A context menu for track(s).
/// Can be used with individual track type widgets based on TrackPointer
/// or list/table type track widgets based on QModelIndexList and TrackModel.
/// Desired menu features can be selected by passing Feature enum flags
/// in constructor.
class WTrackMenu : public QMenu {
    Q_OBJECT
  public:
    enum Feature {
        AutoDJ = 1,
        // The loadTrackToPlayer signal emitted from this class must be handled to make LoadTo work.
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
        TrackModelFeatures = Remove | HideUnhidePurge,
        All = AutoDJ | LoadTo | Playlist | Crate | Remove | Metadata | Reset |
                BPM | Color | HideUnhidePurge | FileBrowser | Properties
    };
    Q_DECLARE_FLAGS(Features, Feature)

    WTrackMenu(QWidget* parent,
            UserSettingsPointer pConfig,
            TrackCollectionManager* pTrackCollectionManager,
            Features flags = Feature::All,
            TrackModel* trackModel = nullptr);
    ~WTrackMenu() override;

    void loadTrackModelIndex(
            const QModelIndex& trackIndex) {
        loadTrackModelIndices(QModelIndexList{trackIndex});
    }
    void loadTrackModelIndices(
            const QModelIndexList& trackIndexList);

    void loadTrack(
            const TrackPointer& pTrack);

    // WARNING: This function hides non-virtual QMenu::popup().
    // This has been done on purpose to ensure menu doesn't popup without loaded track(s).
    void popup(const QPoint& pos, QAction* at = nullptr);
    void slotShowDlgTrackInfo();

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
    void slotShowDlgTagFetcher();
    void slotImportMetadataFromFileTags();
    void slotExportMetadataIntoFileTags();
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
    void slotCoverInfoSelected(CoverInfoRelative coverInfo);
    void slotReloadCoverArt();

    // Library management
    void slotRemove();
    void slotHide();
    void slotUnhide();
    void slotPurge();

  private:
    // This getter verifies that m_pTrackModel is set when
    // invoked.
    const QModelIndexList& getTrackIndices() const;

    TrackIdList getTrackIds() const;
    QList<TrackRef> getTrackRefs() const;

    TrackPointer getFirstTrackPointer() const;

    std::unique_ptr<mixxx::TrackPointerIterator> newTrackPointerIterator() const;

    int applyTrackPointerOperation(
            const QString& progressLabelText,
            const mixxx::TrackPointerOperation* pTrackPointerOperation,
            mixxx::ModalTrackBatchOperationProcessor::Mode operationMode =
                    mixxx::ModalTrackBatchOperationProcessor::Mode::Apply) const;

    bool isEmpty() const {
        return getTrackCount() == 0;
    }
    int getTrackCount() const;

    void createMenus();
    void createActions();
    void setupActions();
    void updateMenus();

    bool featureIsEnabled(Feature flag) const;

    void addSelectionToPlaylist(int iPlaylistId);
    void updateSelectionCrates(QWidget* pWidget);

    void addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc);

    void lockBpm(bool lock);

    void loadSelectionToGroup(QString group, bool play = false);
    void clearTrackSelection();

    bool isAnyTrackBpmLocked() const;

    /// Get the common track color of all tracks this menu is shown for, or
    /// return `nullopt` if there is no common color. Tracks may have no color
    /// assigned to them. In that case the inner optional is set to `nullopt`.
    std::optional<std::optional<mixxx::RgbColor>> getCommonTrackColor() const;
    CoverInfo getCoverInfoOfLastTrack() const;

    TrackModel* m_pTrackModel{};
    QModelIndexList m_trackIndexList;

    // Source of track list when TrackModel is not set.
    TrackPointerList m_trackPointerList;

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
    QAction* m_pBpmResetAction{};

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

    std::unique_ptr<DlgTrackInfo> m_pDlgTrackInfo;
    std::unique_ptr<DlgTagFetcher> m_pDlgTagFetcher;

    struct UpdateExternalTrackCollection {
        QPointer<ExternalTrackCollection> externalTrackCollection;
        QAction* action{};
        };
        QList<UpdateExternalTrackCollection> m_updateInExternalTrackCollections;

        bool m_bPlaylistMenuLoaded;
        bool m_bCrateMenuLoaded;

        Features m_eActiveFeatures;
        const Features m_eTrackModelFeatures;
    };

Q_DECLARE_OPERATORS_FOR_FLAGS(WTrackMenu::Features)
