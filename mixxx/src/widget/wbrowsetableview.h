
#ifndef WBROWSETABLEVIEW_H
#define WBROWSETABLEVIEW_H

#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>

#include "configobject.h"
#include "widget/wlibrarytableview.h"

class WBrowseTableView : public virtual WLibraryTableView {
    Q_OBJECT
  public:
    WBrowseTableView(QWidget* parent, ConfigObject<ConfigValue>* pConfig);
    virtual ~WBrowseTableView();

    void contextMenuEvent(QContextMenuEvent * event);
    void onSearchStarting();
    void onSearchCleared();
    void onSearch(const QString& text);
    void onShow();
    QWidget* getWidgetForMIDIControl();
    virtual void keyPressEvent(QKeyEvent* event);
  signals:
    void loadToPlayer(const QModelIndex&, int);
    void loadToSampler(const QModelIndex&, int);
    void search(const QString&);
    void searchStarting();
    void searchCleared();
  private slots:
    void slotLoadPlayer1();
    void slotLoadPlayer2();
    void slotLoadSampler1();
  private:
    //Used for right-click operations
    /**Send to Player 1 Action**/
    QAction m_player1Act;
    /**Send to Player 2 Action**/
    QAction m_player2Act;
    /**Send to Sampler Actions**/
    QAction m_sampler1Act;
    QMenu m_contextMenu;
};

#endif /* WBROWSETABLEVIEW_H */
