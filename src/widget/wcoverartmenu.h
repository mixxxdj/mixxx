#ifndef WCOVERARTMENU_H
#define WCOVERARTMENU_H

#include <QAction>
#include <QMenu>
#include <QWidget>
#include <QPixmap>

#include "trackinfoobject.h"
#include "library/coverart.h"

// This class implements a context-menu with all CoverArt user actions. Callers
// must call setCoverArt before calling exec or popup. This class does
// not change the database -- it emits a coverArtSelected signal when the user
// performs an action. It is up to the parent to decide how to handle the
// action.
class WCoverArtMenu : public QMenu {
    Q_OBJECT
  public:
    WCoverArtMenu(QWidget *parent = 0);
    virtual ~WCoverArtMenu();

    void setCoverArt(TrackPointer pTrack, const CoverInfo& info);

  signals:
    void coverArtSelected(const CoverArt& art);
    void reloadCoverArt();

  private slots:
    void slotChange();
    void slotUnset();

  private:
    void createActions();

    QAction* m_pChange;
    QAction* m_pReload;
    QAction* m_pUnset;

    TrackPointer m_pTrack;
    CoverInfo m_coverInfo;
};

#endif // WCOVERARTMENU_H
