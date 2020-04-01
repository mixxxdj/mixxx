#ifndef WTRACKMENU_H
#define WTRACKMENU_H

#include <QAction>
#include <QMenu>
#include <QWidget>
#include <library/trackcollectionmanager.h>


#include "widget/wcoverartmenu.h"
#include "widget/wcolorpickeraction.h"

typedef QList<TrackId> TrackIdList;
typedef QList<TrackPointer> TrackPointerList;

class TrackCollectionManager;
class ExternalTrackCollection;

class WTrackMenu : public QMenu {
    Q_OBJECT
  public:
    WTrackMenu(QWidget *parent, TrackCollectionManager* pTrackCollectionManager);
    ~WTrackMenu() override;

    void setTrack(TrackId track);
    void setTracks(TrackIdList trackList);

  private slots:
    void slotOpenInFileBrowser();

private:
    void createActions();
    void setupActions();
    void trackIdsToTrackPointers();
    TrackPointerList getTrackPointerList();

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

    TrackCollectionManager* m_pTrackCollectionManager;
    TrackPointerList m_pTrackPointerList;
};


#endif // WTRACKMENU_H
