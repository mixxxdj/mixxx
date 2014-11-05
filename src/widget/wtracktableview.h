#ifndef WTRACKTABLEVIEW_H
#define WTRACKTABLEVIEW_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "configobject.h"
#include "controlobjectslave.h"
#include "trackinfoobject.h"
#include "library/libraryview.h"
#include "library/trackmodel.h" // Can't forward declare enums
#include "library/coverart.h"
#include "widget/wlibrarytableview.h"
#include "dlgtagfetcher.h"

class ControlObjectThread;
class DlgTrackInfo;
class TrackCollection;
class WCoverArtMenu;

const QString WTRACKTABLEVIEW_VSCROLLBARPOS_KEY = "VScrollBarPos"; /** ConfigValue key for QTable vertical scrollbar position */
const QString LIBRARY_CONFIGVALUE = "[Library]"; /** ConfigValue "value" (wtf) for library stuff */


class WTrackTableView : public WLibraryTableView {
    Q_OBJECT
  public:
    WTrackTableView(QWidget* parent, ConfigObject<ConfigValue>* pConfig,
                    TrackCollection* pTrackCollection, bool sorting = true);
    virtual ~WTrackTableView();
    void contextMenuEvent(QContextMenuEvent * event);
    void onSearchStarting();
    void onSearchCleared();
    void onSearch(const QString& text);
    void onShow();
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void loadSelectedTrack();
    virtual void loadSelectedTrackToGroup(QString group, bool play);

    enum BPMScale {
        DOUBLE,
        HALVE,
        TWOTHIRDS,
        THREEFOURTHS,
    };

  public slots:
    void loadTrackModel(QAbstractItemModel* model);
    void slotMouseDoubleClicked(const QModelIndex &);
    void slotUnhide();
    void slotPurge();

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
    void slotSendToAutoDJ();
    void slotSendToAutoDJTop();
    void slotReloadTrackMetadata();
    void slotResetPlayed();
    void addSelectionToPlaylist(int iPlaylistId);
    void addSelectionToCrate(int iCrateId);
    void loadSelectionToGroup(QString group, bool play = false);
    void doSortByColumn(int headerSection);
    void slotLockBpm();
    void slotUnlockBpm();
    void slotScaleBpm(int);
    void slotClearBeats();
    // Signalled 20 times per second (every 50ms) by GuiTick.
    void slotGuiTick50ms(double);
    void slotScrollValueChanged(int);
    void slotCoverArtSelected(const CoverArt& art);
    void slotReloadCoverArt();

  private:
    void sendToAutoDJ(bool bTop);
    void showTrackInfo(QModelIndex index);
    void showDlgTagFetcher(QModelIndex index);
    void createActions();
    void dragMoveEvent(QDragMoveEvent * event);
    void dragEnterEvent(QDragEnterEvent * event);
    void dropEvent(QDropEvent * event);
    void lockBpm(bool lock);

    void enableCachedOnly();
    void selectionChanged(const QItemSelection &selected,
                          const QItemSelection &deselected);

    // Mouse move event, implemented to hide the text and show an icon instead
    // when dragging.
    void mouseMoveEvent(QMouseEvent *pEvent);

    // Returns the current TrackModel, or returns NULL if none is set.
    TrackModel* getTrackModel();
    bool modelHasCapabilities(TrackModel::CapabilitiesFlags capability);

    ConfigObject<ConfigValue> * m_pConfig;
    TrackCollection* m_pTrackCollection;

    QSignalMapper m_loadTrackMapper;

    DlgTrackInfo* m_pTrackInfo;
    DlgTagFetcher m_DlgTagFetcher;
    QModelIndex currentTrackInfoIndex;


    ControlObjectThread* m_pNumSamplers;
    ControlObjectThread* m_pNumDecks;
    ControlObjectThread* m_pNumPreviewDecks;

    // Context menu machinery
    QMenu *m_pMenu, *m_pPlaylistMenu, *m_pCrateMenu, *m_pSamplerMenu, *m_pBPMMenu;
    WCoverArtMenu* m_pCoverMenu;
    QSignalMapper m_playlistMapper, m_crateMapper, m_deckMapper, m_samplerMapper;

    // Reload Track Metadata Action:
    QAction *m_pReloadMetadataAct;
    QAction *m_pReloadMetadataFromMusicBrainzAct;

    // Load Track to PreviewDeck
    QAction* m_pAddToPreviewDeck;

    // Send to Auto-DJ Action
    QAction *m_pAutoDJAct;
    QAction *m_pAutoDJTopAct;

    // Remove from table
    QAction *m_pRemoveAct;
    QAction *m_pHideAct;
    QAction *m_pUnhideAct;
    QAction *m_pPurgeAct;

    // Reset the played count of selected track or tracks
    QAction* m_pResetPlayedAct;

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

    // Clear track beats
    QAction* m_pClearBeatsAction;

    bool m_sorting;

    // Column numbers
    int m_iCoverSourceColumn; // cover art source
    int m_iCoverTypeColumn; // cover art type
    int m_iCoverLocationColumn; // cover art location
    int m_iCoverHashColumn; // cover art hash
    int m_iCoverColumn; // visible cover art
    int m_iTrackLocationColumn;

    // Control the delay to load a cover art.
    qint64 m_lastUserActionNanos;
    bool m_loadCachedOnly;
    ControlObjectSlave* m_pCOTGuiTick;
};

#endif
