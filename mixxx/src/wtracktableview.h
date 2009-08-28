#ifndef WTRACKTABLEVIEW_H
#define WTRACKTABLEVIEW_H

#include <QAbstractItemModel>
#include <QTableView>

#include "configobject.h"
#include "library/libraryview.h"

class TrackInfoObject;
class TrackModel;

const QString WTRACKTABLEVIEW_HEADERSTATE_KEY = "HeaderState"; /** ConfigValue key for QTable headerview state */
const QString WTRACKTABLEVIEW_VSCROLLBARPOS_KEY = "VScrollBarPos"; /** ConfigValue key for QTable vertical scrollbar position */
const QString LIBRARY_CONFIGVALUE = "[Library]"; /** ConfigValue "value" (wtf) for library stuff */


class WTrackTableView : public QTableView, public LibraryView
{
    Q_OBJECT
 	public:
    WTrackTableView(QWidget * parent, ConfigObject<ConfigValue> * pConfig);
    ~WTrackTableView();
    void setup(QDomNode node);
    void contextMenuEvent(QContextMenuEvent * event);
public slots:
    void saveVScrollBarPos();
    void restoreVScrollBarPos();
    void loadTrackModel(QAbstractItemModel* model);
private slots:
    void slotMouseDoubleClicked(const QModelIndex &);
    void slotLoadPlayer1();
    void slotLoadPlayer2();
    void slotRemove();
signals:
    void loadTrack(TrackInfoObject* pTrack);
    void loadTrackToPlayer(TrackInfoObject* pTrack, int player);
private:
    void createActions();
    void dragMoveEvent(QDragMoveEvent * event);
    void dragEnterEvent(QDragEnterEvent * event);
    void dropEvent(QDropEvent * event);
    
    ConfigObject<ConfigValue> * m_pConfig;
    //QList<QString> m_selectedTrackLocations;
    QModelIndexList m_selectedIndices;   
 	    
    /** The position of the vertical scrollbar slider, eg. before a search is executed */
    int m_iSavedVScrollBarPos;
 	    
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
