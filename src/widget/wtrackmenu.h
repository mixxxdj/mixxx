#ifndef WTRACKMENU_H
#define WTRACKMENU_H

#include <QAction>
#include <QMenu>
#include <QWidget>
#include <library/dlgtrackinfo.h>
#include <library/dlgtagfetcher.h>
#include "library/trackcollectionmanager.h"
#include "library/trackmodel.h"
#include "track/track.h"
#include "library/dao/playlistdao.h"
#include "preferences/usersettings.h"
#include "widget/wlibrarytableview.h"
#include "control/controlproxy.h"

#include "widget/wcoverartmenu.h"
#include "widget/wcolorpickeraction.h"

typedef QList<TrackId> TrackIdList;
typedef QList<TrackPointer> TrackPointerList;

class ControlProxy;
class WCoverArtMenu;
class DlgTagFetcher;
class DlgTrackInfo;
class TrackCollectionManager;
class ExternalTrackCollection;

class WTrackMenu : public QMenu {
    Q_OBJECT
  public:
    WTrackMenu(QWidget *parent, UserSettingsPointer pConfig, TrackCollectionManager* pTrackCollectionManager);
    ~WTrackMenu() override;

    void setTrackId(TrackId track);
    void setTrackIds(TrackIdList trackList);
    void setTrackIndexList(QModelIndexList indexList);
    void setTrackModel(TrackModel* trackModel);

  private slots:
    void slotOpenInFileBrowser();
    void slotLockBpm();
    void slotUnlockBpm();
    void slotScaleBpm(int);
    void slotColorPicked(mixxx::RgbColor::optional_t color);

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

    void lockBpm(bool lock);
    void slotNextTrackInfo();
    void slotNextDlgTagFetcher();
    void slotPrevTrackInfo();
    void slotPrevDlgTagFetcher();
    void slotShowTrackInTagFetcher(TrackPointer track);

    void slotPopulatePlaylistMenu();
    void slotPopulateCrateMenu();
    void addSelectionToNewCrate();

    void slotImportTrackMetadataFromFileTags();
    void slotExportTrackMetadataIntoFileTags();
    void slotUpdateExternalTrackCollection(ExternalTrackCollection *externalTrackCollection);
    void slotRemove();
    void slotHide();
    void slotTrackInfoClosed();
    void slotTagFetcherClosed();
    void slotShowTrackInfo();
    void slotShowDlgTagFetcher();

    void slotAddToAutoDJBottom();
    void slotAddToAutoDJTop();
    void slotAddToAutoDJReplace();

    void slotCoverInfoSelected(const CoverInfoRelative &coverInfo);


    void slotReloadCoverArt();

    void slotUnhide();

    void slotPurge();
public:
    signals:
    void loadTrackToPlayer(TrackPointer pTrack, QString group,
            bool play = false);

private:
    void constructMenus();
    void createActions();
    void setupActions();
    void trackIdsToTrackPointers();
    void setTrackPointerList(TrackPointerList trackPointerList);
    TrackPointerList getTrackPointerList();

    bool modelHasCapabilities(TrackModel::CapabilitiesFlags capabilities) const;

    void addSelectionToPlaylist(int iPlaylistId);
    void updateSelectionCrates(QWidget* pWidget);

    void showTrackInfo(QModelIndex index);
    void showDlgTagFetcher(QModelIndex index);

    void addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc);

    void teardownActions();


    void loadSelectionToGroup(QString group, bool play = false);
    void clearTrackSelection();

    ControlProxy* m_pNumSamplers;
    ControlProxy* m_pNumDecks;
    ControlProxy* m_pNumPreviewDecks;

    // The selected tracks for which the context menu is created
    TrackIdList m_pTrackIdList;

    // Context menu machinery
    QMenu *m_pLoadToMenu;
    QMenu *m_pDeckMenu;
    QMenu *m_pSamplerMenu;

    QMenu *m_pPlaylistMenu;
    QMenu *m_pCrateMenu;
    QMenu *m_pMetadataMenu;
    QMenu *m_pMetadataUpdateExternalCollectionsMenu;
    QMenu *m_pClearMetadataMenu;
    QMenu *m_pBPMMenu;
    QMenu *m_pColorMenu;


    WCoverArtMenu* m_pCoverMenu;

    // Reload Track Metadata Action:
    QAction *m_pImportMetadataFromFileAct;
    QAction *m_pImportMetadataFromMusicBrainzAct;

    // Save Track Metadata Action:
    QAction *m_pExportMetadataAct;

    // Load Track to PreviewDeck
    QAction* m_pAddToPreviewDeck;

    // Send to Auto-DJ Action
    QAction *m_pAutoDJBottomAct;
    QAction *m_pAutoDJTopAct;
    QAction *m_pAutoDJReplaceAct;

    // Remove from table
    QAction *m_pRemoveAct;
    QAction *m_pRemovePlaylistAct;
    QAction *m_pRemoveCrateAct;
    QAction *m_pHideAct;
    QAction *m_pUnhideAct;
    QAction *m_pPurgeAct;

    // Show track-editor action
    QAction *m_pPropertiesAct;
    QAction *m_pFileBrowserAct;

    // BPM feature
    QAction *m_pBpmLockAction;
    QAction *m_pBpmUnlockAction;
    QAction *m_pBpmDoubleAction;
    QAction *m_pBpmHalveAction;
    QAction *m_pBpmTwoThirdsAction;
    QAction *m_pBpmThreeFourthsAction;
    QAction *m_pBpmFourThirdsAction;
    QAction *m_pBpmThreeHalvesAction;

    // Track color
    WColorPickerAction *m_pColorPickerAction;

    // Clear track metadata actions
    QAction* m_pClearBeatsAction;
    QAction* m_pClearPlayCountAction;
    QAction* m_pClearMainCueAction;
    QAction* m_pClearHotCuesAction;
    QAction* m_pClearIntroCueAction;
    QAction* m_pClearOutroCueAction;
    QAction* m_pClearLoopAction;
    QAction* m_pClearWaveformAction;
    QAction* m_pClearKeyAction;
    QAction* m_pClearReplayGainAction;
    QAction* m_pClearAllMetadataAction;


    const UserSettingsPointer m_pConfig;

    TrackCollectionManager* m_pTrackCollectionManager;
    TrackPointerList m_pTrackPointerList;
    TrackModel* m_pTrackModel;

    QModelIndexList m_pSelectedTrackIndices;

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

    // Column numbers
    int m_iCoverSourceColumn; // cover art source
    int m_iCoverTypeColumn; // cover art type
    int m_iCoverLocationColumn; // cover art location
    int m_iCoverHashColumn; // cover art hash
    int m_iCoverColumn; // visible cover art
    int m_iTrackLocationColumn;

    void trackIndicesToTrackPointers();
};





#endif // WTRACKMENU_H
