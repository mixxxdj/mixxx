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
    char* title;
    char* context; // to help translators
    QString iconPath;

    context = (char*) "change cover art location";
    title =  (char*) "&Choose new cover";
    iconPath = ":/images/library/ic_cover_change.png";
    m_pChange = new QAction(QIcon(iconPath), tr(title, context), this);
    connect(m_pChange, SIGNAL(triggered()), this, SLOT(slotChange()));

    context = (char*) "show full size cover in a new window";
    title = (char*) "&Show Full Size";
    iconPath = (":/images/library/ic_cover_fullsize.png");
    m_pFullSize = new QAction(QIcon(iconPath), tr(title, context), this);
    connect(m_pFullSize, SIGNAL(triggered()), this, SLOT(slotShowFullSize()));

    context = (char*) "unset cover art - load default";
    title = (char*) "Unset cover";
    iconPath = (":/images/library/ic_cover_unset.png");
    m_pUnset = new QAction(QIcon(iconPath), tr(title, context), this);
    connect(m_pUnset, SIGNAL(triggered()), this, SLOT(slotUnset()));

    context = (char*) "reload just cover art, using the search algorithm";
    title = (char*) "&Reload from track/folder";
    iconPath = (":/images/library/ic_cover_reload.png");
    m_pReload = new QAction(QIcon(iconPath), tr(title, context), this);
    connect(m_pReload, SIGNAL(triggered()), this, SLOT(slotReload()));
}

void WCoverArtMenu::addActions() {
    addAction(m_pChange);
    addAction(m_pUnset);
    addAction(m_pReload);
    addAction(m_pFullSize);
}

void WCoverArtMenu::updateData(QString coverLocation, QString md5,
                               int trackId, TrackPointer pTrack) {
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
