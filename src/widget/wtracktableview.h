#ifndef WTRACKTABLEVIEW_H
#define WTRACKTABLEVIEW_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "preferences/usersettings.h"
#include "control/controlproxy.h"
#include "library/coverart.h"
#include "library/dlgtagfetcher.h"
#include "library/libraryview.h"
#include "library/trackcollection.h"
#include "library/trackmodel.h" // Can't forward declare enums
#include "track/track.h"
#include "util/duration.h"
#include "widget/wlibrarytableview.h"

class ControlProxy;
class DlgTrackInfo;
class TrackCollection;
class WCoverArtMenu;

const QString WTRACKTABLEVIEW_VSCROLLBARPOS_KEY = "VScrollBarPos"; /** ConfigValue key for QTable vertical scrollbar position */
const QString LIBRARY_CONFIGVALUE = "[Library]"; /** ConfigValue "value" (wtf) for library stuff */


class WTrackTableView : public WLibraryTableView {
    Q_OBJECT
  public:
    WTrackTableView(QWidget* parent, UserSettingsPointer pConfig,
                    TrackCollection* pTrackCollection, bool sorting = true);
    ~WTrackTableView() override;
    void contextMenuEvent(QContextMenuEvent * event) override;
    void onSearch(const QString& text) override;
    void onShow() override;
    bool hasFocus() const override;
    void keyPressEvent(QKeyEvent* event) override;
    void loadSelectedTrack() override;
    void loadSelectedTrackToGroup(QString group, bool play) override;
    QList<TrackId> getSelectedTrackIds() const;
    void setSelectedTracks(const QList<TrackId>& tracks);
    void saveCurrentVScrollBarPos();
    void restoreCurrentVScrollBarPos();

  public slots:
    void loadTrackModel(QAbstractItemModel* model);
    void slotMouseDoubleClicked(const QModelIndex &);
    void slotUnhide();
    void slotPurge();
    void slotSendToAutoDJBottom() override;
    void slotSendToAutoDJTop() override;
    void slotSendToAutoDJReplace() override;

  private slots:
    void slotRemove();
    void slotHide();
    void slotOpenInFileBrowser();
    void slotShowTrackInfo();
    void slotShowDlgTagFetcher();
    void slotNextTrackInfo();
    void slotNextDlgTagFetcher();
    void slotPrevTrackInfo();
    void slotPrevDlgTagFetcher();
    void slotShowTrackInTagFetcher(TrackPointer track);
    void slotImportTrackMetadataFromFileTags();
    void slotExportTrackMetadataIntoFileTags();
    void slotPopulatePlaylistMenu();
    void addSelectionToPlaylist(int iPlaylistId);
    void updateSelectionCrates(QWidget* qc);
    void slotPopulateCrateMenu();
    void addSelectionToNewCrate();
    void loadSelectionToGroup(QString group, bool play = false);
    void doSortByColumn(int headerSection);
    void applySortingIfVisible();
    void applySorting();
    void slotLockBpm();
    void slotUnlockBpm();
    void slotScaleBpm(int);

    void slotClearBeats();
    void slotClearPlayCount();
    void slotClearMainCue();
    void slotClearHotCues();
    void slotClearLoop();
    void slotClearKey();
    void slotClearReplayGain();
    void slotClearWaveform();
    void slotClearAllMetadata();

    // Signalled 20 times per second (every 50ms) by GuiTick.
    void slotGuiTick50ms(double);
    void slotScrollValueChanged(int);
    void slotCoverInfoSelected(const CoverInfoRelative& coverInfo);
    void slotReloadCoverArt();

    void slotTrackInfoClosed();
    void slotTagFetcherClosed();
    void slotSortingChanged(int headerSection, Qt::SortOrder order);
    void keyNotationChanged();

  private:

    void sendToAutoDJ(PlaylistDAO::AutoDJSendLoc loc);
    void showTrackInfo(QModelIndex index);
    void showDlgTagFetcher(QModelIndex index);
    void createActions();
    void dragMoveEvent(QDragMoveEvent * event) override;
    void dragEnterEvent(QDragEnterEvent * event) override;
    void dropEvent(QDropEvent * event) override;
    void lockBpm(bool lock);

    void enableCachedOnly();
    void selectionChanged(const QItemSelection &selected,
                          const QItemSelection &deselected) override;

    // Mouse move event, implemented to hide the text and show an icon instead
    // when dragging.
    void mouseMoveEvent(QMouseEvent *pEvent) override;

    // Returns the current TrackModel, or returns NULL if none is set.
    TrackModel* getTrackModel() const;
    bool modelHasCapabilities(TrackModel::CapabilitiesFlags capabilities) const;

    UserSettingsPointer m_pConfig;
    TrackCollection* m_pTrackCollection;

    QSignalMapper m_loadTrackMapper;

    QScopedPointer<DlgTrackInfo> m_pTrackInfo;
    QScopedPointer<DlgTagFetcher> m_pTagFetcher;

    QModelIndex currentTrackInfoIndex;


    ControlProxy* m_pNumSamplers;
    ControlProxy* m_pNumDecks;
    ControlProxy* m_pNumPreviewDecks;

    // Context menu machinery
    QMenu *m_pMenu;

    QMenu *m_pLoadToMenu;
    QMenu *m_pDeckMenu;
    QMenu *m_pSamplerMenu;

    QMenu *m_pPlaylistMenu;
    QMenu *m_pCrateMenu;
    QMenu *m_pMetadataMenu;
    QMenu *m_pClearMetadataMenu;
    QMenu *m_pBPMMenu;


    WCoverArtMenu* m_pCoverMenu;
    QSignalMapper m_playlistMapper, m_crateMapper, m_deckMapper, m_samplerMapper;

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
    QSignalMapper m_BpmMapper;
    QAction *m_pBpmDoubleAction;
    QAction *m_pBpmHalveAction;
    QAction *m_pBpmTwoThirdsAction;
    QAction *m_pBpmThreeFourthsAction;
    QAction *m_pBpmFourThirdsAction;
    QAction *m_pBpmThreeHalvesAction;

    // Clear track metadata actions
    QAction* m_pClearBeatsAction;
    QAction* m_pClearPlayCountAction;
    QAction* m_pClearMainCueAction;
    QAction* m_pClearHotCuesAction;
    QAction* m_pClearLoopAction;
    QAction* m_pClearWaveformAction;
    QAction* m_pClearKeyAction;
    QAction* m_pClearReplayGainAction;
    QAction* m_pClearAllMetadataAction;

    bool m_sorting;

    // Column numbers
    int m_iCoverSourceColumn; // cover art source
    int m_iCoverTypeColumn; // cover art type
    int m_iCoverLocationColumn; // cover art location
    int m_iCoverHashColumn; // cover art hash
    int m_iCoverColumn; // visible cover art
    int m_iTrackLocationColumn;

    // Control the delay to load a cover art.
    mixxx::Duration m_lastUserAction;
    bool m_selectionChangedSinceLastGuiTick;
    bool m_loadCachedOnly;
    bool m_bPlaylistMenuLoaded;
    bool m_bCrateMenuLoaded;
    ControlProxy* m_pCOTGuiTick;
    ControlProxy* m_pKeyNotation;
    ControlProxy* m_pSortColumn;
    ControlProxy* m_pSortOrder;
};

#endif
