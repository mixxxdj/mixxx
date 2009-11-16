#ifndef WTRACKTABLEVIEW_H
#define WTRACKTABLEVIEW_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

#include "configobject.h"
#include "library/libraryview.h"
#include "library/searchthread.h"
#include "library/trackmodel.h" // Can't forward declare enums
#include "widget/wlibrarytableview.h"

class TrackInfoObject;

const QString WTRACKTABLEVIEW_HEADERSTATE_KEY = "HeaderState"; /** ConfigValue key for QTable headerview state */
const QString WTRACKTABLEVIEW_VSCROLLBARPOS_KEY = "VScrollBarPos"; /** ConfigValue key for QTable vertical scrollbar position */
const QString LIBRARY_CONFIGVALUE = "[Library]"; /** ConfigValue "value" (wtf) for library stuff */


class WTrackTableView : public WLibraryTableView
{
    Q_OBJECT
 	public:
    WTrackTableView(QWidget* parent, ConfigObject<ConfigValue>* pConfig);
    virtual ~WTrackTableView();
    void contextMenuEvent(QContextMenuEvent * event);
    void onSearchStarting();
    void onSearchCleared();
    void onSearch(const QString& text);
    void onShow();
public slots:
    void loadTrackModel(QAbstractItemModel* model);
private slots:
    void slotMouseDoubleClicked(const QModelIndex &);
    void slotLoadPlayer1();
    void slotLoadPlayer2();
    void slotRemove();
    void slotShowTrackInfo();
signals:
    void loadTrack(TrackInfoObject* pTrack);
    void loadTrackToPlayer(TrackInfoObject* pTrack, int player);

private:
    void createActions();
    void dragMoveEvent(QDragMoveEvent * event);
    void dragEnterEvent(QDragEnterEvent * event);
    void dropEvent(QDropEvent * event);

    // Returns the current TrackModel, or returns NULL if none is set.
    TrackModel* getTrackModel();
    bool modelHasCapabilities(TrackModel::CapabilitiesFlags capability);

    ConfigObject<ConfigValue> * m_pConfig;
    //QList<QString> m_selectedTrackLocations;
    QModelIndexList m_selectedIndices;

    SearchThread m_searchThread;

    //Used for right-click operations
    /**Send to Play Queue Action**/
    QAction *m_pPlayQueueAct;
    /**Send to Player 1 Action**/
    QAction *m_pPlayer1Act;
    /**Send to Player 2 Action**/
    QAction *m_pPlayer2Act;
    /**Remove from Table Action**/
    QAction *m_pRemoveAct;
    /**Shows track editor/BPM tap**/
    QAction *m_pPropertiesAct;
};

#endif
