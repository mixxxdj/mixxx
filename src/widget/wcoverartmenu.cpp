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
    m_pChange = new QAction(QIcon(":/images/library/ic_cover_change.png"),
                            tr("Choose new cover",
                               "change cover art location"),
                            this);
    connect(m_pChange, SIGNAL(triggered()), this, SLOT(slotChange()));

    m_pFullSize = new QAction(QIcon(":/images/library/ic_cover_fullsize.png"),
                              tr("Show Full Size",
                                 "show full size cover in a new window"),
                              this);
    connect(m_pFullSize, SIGNAL(triggered()), this, SLOT(slotShowFullSize()));

    m_pUnset = new QAction(QIcon(":/images/library/ic_cover_unset.png"),
                           tr("Unset cover",
                              "unset cover art - load default"),
                           this);
    connect(m_pUnset, SIGNAL(triggered()), this, SLOT(slotUnset()));

    m_pReload = new QAction(QIcon(":/images/library/ic_cover_reload.png"),
                            tr("Reload from track/folder",
                               "reload just cover art, using the search algorithm"),
                            this);
    connect(m_pReload, SIGNAL(triggered()), this, SLOT(slotReload()));
}

void WCoverArtMenu::addActions() {
    addAction(m_pChange);
    addAction(m_pUnset);
    addAction(m_pReload);
    addAction(m_pFullSize);
}

void WCoverArtMenu::show(QPoint pos, QPair<QString, QString> cover,
                         int trackId, TrackPointer pTrack) {
    m_iTrackId = trackId;
    m_sCoverLocation = cover.first;
    m_sMd5 = cover.second;
    m_pTrack = pTrack;

    if (trackId < 1) {
        return;
    }

    QString defaultLoc = CoverArtCache::instance()->getDefaultCoverLocation();
    if (cover.first == defaultLoc || cover.second.isEmpty()) {
        m_pFullSize->setEnabled(false);
    } else {
        m_pFullSize->setEnabled(true);
    }

    popup(pos);
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
    if (!res) {
        QMessageBox::warning(this, tr("Change Cover Art"),
                             tr("Could not change the cover art!"));
        return;
    }

    emit(coverLocationUpdated(newCover, m_sCoverLocation));
    m_sCoverLocation = newCover;
}

void WCoverArtMenu::slotShowFullSize() {
    DlgCoverArtFullSize::instance()->init();
}

void WCoverArtMenu::slotReload() {
    if (m_iTrackId < 1) {
        return;
    }
    CoverArtCache::instance()->changeCoverArt(m_iTrackId);
    CoverArtCache::instance()->requestPixmap(m_iTrackId);
}

void WCoverArtMenu::slotUnset() {
    if (m_iTrackId < 1) {
        return;
    }
    QString newLoc = CoverArtCache::instance()->getDefaultCoverLocation();
    if (!CoverArtCache::instance()->changeCoverArt(m_iTrackId, newLoc)) {
        QMessageBox::warning(this, tr("Unset Cover Art"),
                             tr("Could not unset the cover art!"));
        return;
    }
    emit(coverLocationUpdated(newLoc, m_sCoverLocation));
    m_sCoverLocation = newLoc;
}
