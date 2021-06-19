#pragma once

#include <QAction>
#include <QMenu>
#include <QWidget>
#include <QPixmap>

#include "library/coverart.h"

// This class implements a context-menu with all CoverArt user actions. Callers
// must call setCoverArt before calling exec or popup. This class does
// not change the database -- it emits a coverArtSelected signal when the user
// performs an action. It is up to the parent to decide how to handle the
// action.
class WCoverArtMenu : public QMenu {
    Q_OBJECT
  public:
    explicit WCoverArtMenu(QWidget *parent = nullptr);
    ~WCoverArtMenu() override;

    void setCoverArt(const CoverInfo& coverInfo);

  signals:
    void coverInfoSelected(const CoverInfoRelative& coverInfo);
    void reloadCoverArt();

  private slots:
    void slotChange();
    void slotUnset();

  private:
    void createActions();

    QAction* m_pChange;
    QAction* m_pReload;
    QAction* m_pUnset;

    CoverInfo m_coverInfo;
};
