#pragma once

#include <QMenu>
#include <QModelIndex>
#include <QPointer>
#include <memory>

#include "analyzer/analyzertrack.h"
#include "control/pollingcontrolproxy.h"
#include "library/coverart.h"
#include "library/dao/genredao.h"
#include "library/dao/playlistdao.h"
#include "library/trackprocessing.h"
#include "preferences/usersettings.h"
#include "track/beats.h"
#include "track/track_decl.h"
#include "track/trackref.h"
#include "util/color/rgbcolor.h"
#include "util/parented_ptr.h"

class DlgTagFetcher;
class DlgTrackInfo;
class DlgTrackInfoMulti;
class DlgTrackInfoMultiExperimental;
//class DlgDeleteFilesConfirmation;
class ExternalTrackCollection;
class Library;
class TrackModel;
class WColorPickerAction;
class WCoverArtMenu;
class WFindOnWebMenu;
class WSearchRelatedTracksMenu;
class WStarRatingAction;

/// A context menu for track(s).
/// Can be used with individual track type widgets based on TrackPointer
/// or list/table type track widgets based on QModelIndexList and TrackModel.
/// Desired menu features can be selected by passing Feature enum flags
/// in constructor.
class WTrackMenu : public QMenu {
    Q_OBJECT
  public:
    enum Feature {
        AutoDJ = 1 << 0,
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
        RemoveFromDisk = 1 << 10,
        FileBrowser = 1 << 11,
        Properties = 1 << 12,
        SearchRelated = 1 << 13,
        UpdateReplayGainFromPregain = 1 << 14,
        SelectInLibrary = 1 << 15,
        Analyze = 1 << 16,
        FindOnWeb = 1 << 17,
        Genre = 1 << 18,
        TrackModelFeatures = Remove | HideUnhidePurge,
        All = AutoDJ | LoadTo | Playlist | Crate | Genre | Remove | Metadata | Reset | Analyze |
                BPM | Color | HideUnhidePurge | RemoveFromDisk | FileBrowser |
                Properties | SearchRelated | UpdateReplayGainFromPregain | SelectInLibrary |
                FindOnWeb
    };
    Q_DECLARE_FLAGS(Features, Feature)

    // Make all deck track widgets provide the same features.
    // Used by WTrackProperty, WTrackText & WTrackWidgetGroup.
    static constexpr WTrackMenu::Features kDeckTrackMenuFeatures{
            WTrackMenu::Feature::SearchRelated |
            WTrackMenu::Feature::Playlist |
            WTrackMenu::Feature::Crate |
            WTrackMenu::Feature::Genre |
            WTrackMenu::Feature::Metadata |
            WTrackMenu::Feature::Reset |
            WTrackMenu::Feature::Analyze |
            WTrackMenu::Feature::BPM |
            WTrackMenu::Feature::Color |
            WTrackMenu::Feature::RemoveFromDisk |
            WTrackMenu::Feature::FileBrowser |
            WTrackMenu::Feature::Properties |
            WTrackMenu::Feature::UpdateReplayGainFromPregain |
            WTrackMenu::Feature::FindOnWeb |
            WTrackMenu::Feature::SelectInLibrary};

    WTrackMenu(QWidget* parent,
            UserSettingsPointer pConfig,
            Library* pLibrary,
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
            const TrackPointer& pTrack, const QString& deckGroup);
    /// Set the track property string which can be used to tell DlgTrackInfo/~Multi
    /// to focus a specific metadata editor widget
    void setTrackPropertyName(const QString& property = QString()) {
        m_trackProperty = property;
    }

    // WARNING: This function hides non-virtual QMenu::popup().
    // This has been done on purpose to ensure menu doesn't popup without loaded track(s).
    void popup(const QPoint& pos, QAction* at = nullptr);
    void slotShowDlgTrackInfo();
    void slotShowDlgTrackInfoExperimental();
    // Library management
    void slotRemoveFromDisk();
    const QString getDeckGroup() const;

    void loadGenreData2QVL();

  signals:
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
    void trackMenuVisible(bool visible);
    void saveCurrentViewState();
    void restoreCurrentViewStateOrIndex();

  private slots:
    // File
    void slotOpenInFileBrowser();
    void slotSelectInLibrary();

    // Track rating
    void slotSetRating(int rating);

    // Row color
    void slotColorPicked(const mixxx::RgbColor::optional_t& color);

    // Reset
    void slotClearBeats();
    void slotClearPlayCount();
    void slotClearRating();
    void slotClearComment();
    void slotResetMainCue();
    void slotClearHotCues();
    void slotResetIntroCue();
    void slotResetOutroCue();
    void slotClearLoops();
    void slotClearKey();
    void slotClearReplayGain();
    void slotClearWaveform();
    void slotClearAllMetadata();

    // Analysis
    void slotAnalyze();
    void slotReanalyze();
    void slotReanalyzeWithFixedTempo();
    void slotReanalyzeWithVariableTempo();

    // BPM
    void slotLockBpm();
    void slotUnlockBpm();
    void slotScaleBpm(mixxx::Beats::BpmScale scale);
    void slotUndoBeatsChange();
    void slotTranslateBeatsHalf();

    // Hotcues
    void slotSortHotcuesByPosition(HotcueSortMode sortMode);

    // Info and metadata
    void slotUpdateReplayGainFromPregain();
    void slotShowDlgTagFetcher();
    void slotImportMetadataFromFileTags();
    void slotExportMetadataIntoFileTags();
    void slotUpdateExternalTrackCollection(ExternalTrackCollection* externalTrackCollection);

    // Playlist and crate
    void slotPopulatePlaylistMenu();
    void slotPopulateCrateMenu();
    void addSelectionToNewCrate();

    // Genre
    void slotPopulateGenreMenu();
    void addSelectionToNewGenre();

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
    void closeEvent(QCloseEvent* event) override;
    // This getter verifies that m_pTrackModel is set when
    // invoked.
    const QModelIndexList& getTrackIndices() const;

    TrackIdList getTrackIds() const;
    QList<TrackRef> getTrackRefs() const;

    TrackPointer getFirstTrackPointer() const;
    TrackPointerList getTrackPointers() const;

    std::unique_ptr<mixxx::TrackPointerIterator> newTrackPointerIterator() const;

    /// WARNING: The provided pTrackPointerOperation must ensure NOT
    /// TO MODIFY the underlying m_pTrackModel during the iteration!!!
    /// This might happen not only directly but also indirectly by
    /// handling signals, e.g. TrackDAO::enforceModelUpdate().
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

    void generateTrackLoadMenu(const QString& group,
            const QString& label,
            TrackPointer pTrack,
            QMenu* pParentMenu,
            bool primaryDeck,
            bool enabled = true);

    bool featureIsEnabled(Feature flag) const;

    void addSelectionToPlaylist(int iPlaylistId);
    void updateSelectionCrates(QWidget* pWidget);
    void updateSelectionGenres(QWidget* pWidget);

    void addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc);
    void addToAnalysis(AnalyzerTrack::Options options = AnalyzerTrack::Options());

    void clearBeats();
    void lockBpm(bool lock);

#ifdef __STEM__
    void loadSelectionToGroup(const QString& group,
            mixxx::StemChannelSelection stemMask = mixxx::StemChannelSelection(),
            bool play = false);
#else
    void loadSelectionToGroup(const QString& group,
            bool play = false);
#endif
    void clearTrackSelection();

    std::pair<bool, bool> getTrackBpmLockStates() const;
    bool canUndoBeatsChange() const;

    /// Get the common rating of all selected tracks.
    /// Return 0 if ratings differ.
    int getCommonTrackRating() const;

    /// Get the common track color of all tracks this menu is shown for, or
    /// return `nullopt` if there is no common color. Tracks may have no color
    /// assigned to them. In that case the inner optional is set to `nullopt`.
    std::optional<std::optional<mixxx::RgbColor>> getCommonTrackColor() const;
    CoverInfo getCoverInfoOfLastTrack() const;

    TrackModel* const m_pTrackModel;
    QModelIndexList m_trackIndexList;

    /// Track being referenced when TrackModel is not set.
    TrackPointer m_pTrack;
    /// If the user right clicked on a track in a deck, this will record which
    /// deck made the request.
    QString m_deckGroup;

    // Submenus
    parented_ptr<QMenu> m_pLoadToMenu;
    parented_ptr<QMenu> m_pDeckMenu;
    parented_ptr<QMenu> m_pSamplerMenu;
    parented_ptr<QMenu> m_pPlaylistMenu;
    parented_ptr<QMenu> m_pCrateMenu;
    parented_ptr<QMenu> m_pGenreMenu;
    parented_ptr<QMenu> m_pMetadataMenu;
    parented_ptr<QMenu> m_pMetadataUpdateExternalCollectionsMenu;
    parented_ptr<QMenu> m_pHotcueMenu;
    parented_ptr<QMenu> m_pClearMetadataMenu;
    parented_ptr<QMenu> m_pAnalyzeMenu;
    parented_ptr<QMenu> m_pBPMMenu;
    parented_ptr<QMenu> m_pColorMenu;
    parented_ptr<WCoverArtMenu> m_pCoverMenu;
    parented_ptr<WSearchRelatedTracksMenu> m_pSearchRelatedMenu;
    parented_ptr<WFindOnWebMenu> m_pFindOnWebMenu;
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    QMenu* m_pRemoveFromDiskMenu{};
#endif

    // Update ReplayGain from Track
    parented_ptr<QAction> m_pUpdateReplayGainAct;

    // Reload Track Metadata Action:
    parented_ptr<QAction> m_pImportMetadataFromFileAct;
    parented_ptr<QAction> m_pImportMetadataFromMusicBrainzAct;

    // Save Track Metadata Action:
    parented_ptr<QAction> m_pExportMetadataAct;

    // Send to Auto-DJ Action
    parented_ptr<QAction> m_pAutoDJBottomAct;
    parented_ptr<QAction> m_pAutoDJTopAct;
    parented_ptr<QAction> m_pAutoDJReplaceAct;

    // Remove from table
    parented_ptr<QAction> m_pRemoveAct;
    parented_ptr<QAction> m_pRemovePlaylistAct;
    parented_ptr<QAction> m_pRemoveCrateAct;
    parented_ptr<QAction> m_pRemoveGenreAct;
    parented_ptr<QAction> m_pHideAct;
    parented_ptr<QAction> m_pUnhideAct;
    parented_ptr<QAction> m_pPurgeAct;
    parented_ptr<QAction> m_pRemoveFromDiskAct;

    // Show track-editor action
    parented_ptr<QAction> m_pPropertiesAct;
    parented_ptr<QAction> m_pPropertiesActExp;

    // Open file in default file browser
    parented_ptr<QAction> m_pFileBrowserAct;

    // Select track in library
    parented_ptr<QAction> m_pSelectInLibraryAct;

    // BPM feature
    parented_ptr<QAction> m_pBpmLockAction;
    parented_ptr<QAction> m_pBpmUnlockAction;
    parented_ptr<QAction> m_pBpmDoubleAction;
    parented_ptr<QAction> m_pBpmHalveAction;
    parented_ptr<QAction> m_pBpmTwoThirdsAction;
    parented_ptr<QAction> m_pBpmThreeFourthsAction;
    parented_ptr<QAction> m_pBpmFourThirdsAction;
    parented_ptr<QAction> m_pBpmThreeHalvesAction;
    parented_ptr<QAction> m_pBpmResetAction;
    parented_ptr<QAction> m_pBpmUndoAction;
    parented_ptr<QAction> m_pTranslateBeatsHalf;

    // Track rating and color
    parented_ptr<WStarRatingAction> m_pStarRatingAction;
    parented_ptr<WColorPickerAction> m_pColorPickerAction;

    // Analysis actions
    parented_ptr<QAction> m_pAnalyzeAction;
    parented_ptr<QAction> m_pReanalyzeAction;
    parented_ptr<QAction> m_pReanalyzeConstBpmAction;
    parented_ptr<QAction> m_pReanalyzeVarBpmAction;

    // Clear track metadata actions
    parented_ptr<QAction> m_pClearBeatsAction;
    parented_ptr<QAction> m_pClearPlayCountAction;
    parented_ptr<QAction> m_pClearRatingAction;
    parented_ptr<QAction> m_pClearMainCueAction;
    parented_ptr<QAction> m_pClearHotCuesAction;
    parented_ptr<QAction> m_pClearIntroCueAction;
    parented_ptr<QAction> m_pClearOutroCueAction;
    parented_ptr<QAction> m_pClearLoopsAction;
    parented_ptr<QAction> m_pClearWaveformAction;
    parented_ptr<QAction> m_pClearCommentAction;
    parented_ptr<QAction> m_pClearKeyAction;
    parented_ptr<QAction> m_pClearReplayGainAction;
    parented_ptr<QAction> m_pClearAllMetadataAction;
    parented_ptr<QAction> m_pSortHotcuesByPositionAction{};
    parented_ptr<QAction> m_pSortHotcuesByPositionCompressAction{};

    const UserSettingsPointer m_pConfig;
    Library* const m_pLibrary;

    PollingControlProxy m_pNumSamplers;
    PollingControlProxy m_pNumDecks;
    PollingControlProxy m_pNumPreviewDecks;

    std::unique_ptr<DlgTrackInfo> m_pDlgTrackInfo;
    std::unique_ptr<DlgTrackInfoMulti> m_pDlgTrackInfoMulti;
    std::unique_ptr<DlgTrackInfoMultiExperimental> m_pDlgTrackInfoMultiExperimental;
    std::unique_ptr<DlgTagFetcher> m_pDlgTagFetcher;

    struct UpdateExternalTrackCollection {
        QPointer<ExternalTrackCollection> pExternalTrackCollection;
        QAction* pAction{};
    };

    QList<UpdateExternalTrackCollection> m_updateInExternalTrackCollections;

    bool m_bPlaylistMenuLoaded;
    bool m_bCrateMenuLoaded;
    bool m_bGenreMenuLoaded;

    Features m_eActiveFeatures;
    const Features m_eTrackModelFeatures;

    QString m_trackProperty;

    static bool s_showPurgeSuccessPopup;
    QVariantList m_genreData;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WTrackMenu::Features)
