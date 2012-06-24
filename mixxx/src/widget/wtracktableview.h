#ifndef WTRACKTABLEVIEW_H
#define WTRACKTABLEVIEW_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "configobject.h"
#include "trackinfoobject.h"
#include "library/libraryview.h"
#include "library/searchthread.h"
#include "library/trackmodel.h" // Can't forward declare enums
#include "widget/wlibrarytableview.h"

class ControlObjectThreadMain;
class DlgTrackInfo;
class TrackCollection;

const QString WTRACKTABLEVIEW_VSCROLLBARPOS_KEY = "VScrollBarPos"; /** ConfigValue key for QTable vertical scrollbar position */
const QString LIBRARY_CONFIGVALUE = "[Library]"; /** ConfigValue "value" (wtf) for library stuff */


class WTrackTableView : public WLibraryTableView {
    Q_OBJECT
  public:
    WTrackTableView(QWidget* parent, ConfigObject<ConfigValue>* pConfig,
                    TrackCollection* pTrackCollection);
    virtual ~WTrackTableView();
    void contextMenuEvent(QContextMenuEvent * event);
    void onSearchStarting();
    void onSearchCleared();
    void onSearch(const QString& text);
    void onShow();
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void loadSelectedTrack();
    virtual void loadSelectedTrackToGroup(QString group);
    void disableSorting();

  public slots:
    void loadTrackModel(QAbstractItemModel* model);
    void slotMouseDoubleClicked(const QModelIndex &);

  private slots:
    void slotRemove();
    void slotHide();
    void slotUnhide();
    void slotPurge();
    void slotOpenInFileBrowser();
    void slotShowTrackInfo();
    void slotNextTrackInfo();
    void slotPrevTrackInfo();
    void slotSendToAutoDJ();
    void slotSendToAutoDJTop();
    void slotReloadTrackMetadata();
    void slotResetPlayed();
    void addSelectionToPlaylist(int iPlaylistId);
    void addSelectionToCrate(int iCrateId);
    void loadSelectionToGroup(QString group);
    void doSortByColumn(int headerSection);
    void slotLockBpm();
    void slotUnlockBpm();
    void slotClearBeats();

  private:
    void sendToAutoDJ(bool bTop);
    void showTrackInfo(QModelIndex index);
    void createActions();
    void dragMoveEvent(QDragMoveEvent * event);
    void dragEnterEvent(QDragEnterEvent * event);
    void dropEvent(QDropEvent * event);
    void lockBpm(bool lock);

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
    QModelIndex currentTrackInfoIndex;

    SearchThread m_searchThread;

    ControlObjectThreadMain* m_pNumSamplers;
    ControlObjectThreadMain* m_pNumDecks;

    // Context menu machinery
    QMenu *m_pMenu, *m_pPlaylistMenu, *m_pCrateMenu, *m_pSamplerMenu;
    QSignalMapper m_playlistMapper, m_crateMapper, m_deckMapper, m_samplerMapper;

    // Reload Track Metadata Action:
    QAction *m_pReloadMetadataAct;

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

    // BPM Lock feature
    QAction *m_pBpmLockAction;
    QAction *m_pBpmUnlockAction;

    // Clear track beats
    QAction* m_pClearBeatsAction;
};

#endif
