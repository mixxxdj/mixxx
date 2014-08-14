#include <QIcon>

#include "dlgcoverartfullsize.h"
#include "wcoverartmenu.h"

WCoverArtMenu::WCoverArtMenu(QWidget *parent)
        : QMenu(parent),
          m_iTrackId(-1) {
    createActions();
    addActions();
}

WCoverArtMenu::~WCoverArtMenu() {
    delete m_pChange;
    delete m_pFullSize;
    delete m_pReload;
    delete m_pUnset;
}

void WCoverArtMenu::createActions() {
    // change cover art location
    m_pChange = new QAction(QIcon(":/images/library/ic_cover_change.png"),
                            tr("&Change"), this);

    // unset cover art - load default
    m_pUnset = new QAction(QIcon(":/images/library/ic_cover_unset.png"),
                           tr("&Unset"), this);

    // reload just cover art using the search algorithm (in CoverArtCache)
    m_pReload = new QAction(QIcon(":/images/library/ic_cover_reload.png"),
                            tr("&Reload"), this);

    // show full size cover in a new window
    m_pFullSize = new QAction(QIcon(":/images/library/ic_cover_fullsize.png"),
                              tr("&Show Full Size"), this);

    connect(m_pChange, SIGNAL(triggered()), this, SLOT(slotChange()));
    connect(m_pFullSize, SIGNAL(triggered()), this, SLOT(slotShowFullSize()));
    connect(m_pReload, SIGNAL(triggered()), this, SLOT(slotReload()));
    connect(m_pUnset, SIGNAL(triggered()), this, SLOT(slotUnset()));
}

void WCoverArtMenu::addActions() {
    addAction(m_pChange);
    addAction(m_pUnset);
    addAction(m_pReload);
    addAction(m_pFullSize);
}

void WCoverArtMenu::updateData(int trackId, QString coverLocation,
                               QString md5, TrackPointer pTrack) {
    m_iTrackId = trackId;
    m_sCoverLocation = coverLocation;
    m_sMd5 = md5;
    m_pTrack = pTrack;
}

void WCoverArtMenu::slotChange() {
    // TODO
}

void WCoverArtMenu::slotShowFullSize() {
    // TODO
    // DlgCoverArtFullSize::instance()->init(m_currentCover, m_sCoverTitle);
}

void WCoverArtMenu::slotReload() {
    // TODO
}

void WCoverArtMenu::slotUnset() {
    // TODO
}
