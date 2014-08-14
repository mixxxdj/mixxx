#include <QFileDialog>
#include <QIcon>
#include <QStringBuilder>

#include "dlgcoverartfullsize.h"
#include "wcoverartmenu.h"
#include "library/coverartcache.h"

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
    if (m_iTrackId < 1) {
        return;
    }

    if (!m_pTrack) {
        m_pTrack = CoverArtCache::instance()->getTrack(m_iTrackId);
        if (!m_pTrack) {
            return;
        }
    }

    // get initial directory (trackdir or coverdir)
    QString initialDir;
    QString trackPath = m_pTrack->getDirectory();
    if (m_sCoverLocation.isEmpty() ||
        m_sCoverLocation == CoverArtCache::instance()
                                ->getDefaultCoverLocation()) {
        initialDir = trackPath;
    } else {
        initialDir = m_sCoverLocation;
    }

    // open file dialog
    QString selectedCover = QFileDialog::getOpenFileName(
                this, tr("Change Cover Art"), initialDir,
                tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));

    if (selectedCover.isEmpty()) {
        return;
    }

    // if the cover comes from an external dir,
    // we copy it to the track directory.
    QString newCover;
    QFileInfo coverInfo(selectedCover);
    QString coverPath = coverInfo.absolutePath();
    if (trackPath != coverPath) {
        QString ext = coverInfo.suffix();
        QStringList filepaths;
        filepaths << trackPath % "/cover." % ext
                  << trackPath % "/album." % ext
                  << trackPath % "/mixxx-cover." % ext;

        foreach (QString filepath, filepaths) {
            if (QFile::copy(selectedCover, filepath)) {
                newCover = filepath;
                break;
            }
        }

        if (newCover.isEmpty()) {
            // overwrites "mixxx-cover"
            QFile::remove(filepaths.last());
            if (QFile::copy(selectedCover, filepaths.last())) {
                newCover = filepaths.last();
            }
        }
    } else {
        newCover = selectedCover;
    }

    bool res = CoverArtCache::instance()->changeCoverArt(m_pTrack->getId(),
                                                         newCover);
    if (res) {
        m_sCoverLocation = newCover;
    } else {
        QMessageBox::warning(this, tr("Change Cover Art"),
                             tr("Could not change the cover art!"));
    }
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
