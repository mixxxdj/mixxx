#include <QFileDialog>
#include <QFileInfo>

#include "widget/wcoverartmenu.h"
#include "library/coverartutils.h"
#include "util/sandbox.h"

WCoverArtMenu::WCoverArtMenu(QWidget *parent)
        : QMenu(parent) {
    createActions();
}

WCoverArtMenu::~WCoverArtMenu() {
    delete m_pChange;
    delete m_pReload;
    delete m_pUnset;
}

void WCoverArtMenu::createActions() {
    m_pChange = new QAction(tr("Choose new cover",
            "change cover art location"), this);
    connect(m_pChange, SIGNAL(triggered()), this, SLOT(slotChange()));
    addAction(m_pChange);

    m_pUnset = new QAction(tr("Unset cover",
            "clears the set cover art -- does not touch files on disk"), this);
    connect(m_pUnset, SIGNAL(triggered()), this, SLOT(slotUnset()));
    addAction(m_pUnset);

    m_pReload = new QAction(tr("Reload from track/folder",
            "reload cover art from track metadata or folder"), this);
    connect(m_pReload, SIGNAL(triggered()), this, SIGNAL(reloadCoverArt()));
    addAction(m_pReload);
}

void WCoverArtMenu::setCoverArt(TrackPointer pTrack, const CoverInfo& info) {
    m_pTrack = pTrack;
    m_coverInfo = info;
}

void WCoverArtMenu::slotChange() {
    // get initial directory (trackdir or coverdir)
    QString initialDir;

    QFileInfo track;
    if (m_pTrack) {
        track = m_pTrack->getFileInfo();
    } else if (!m_coverInfo.trackLocation.isEmpty()) {
        track = QFileInfo(m_coverInfo.trackLocation);
    }

    // If the cover is from file metadata then use the directory the track is
    // in.
    if (m_coverInfo.type == CoverInfo::METADATA) {
        initialDir = track.absolutePath();
    } else if (m_coverInfo.type == CoverInfo::FILE) {
        QFileInfo file(track.dir(), m_coverInfo.coverLocation);
        initialDir = file.absolutePath();
    } else {
        // Otherwise, default to the track directory.
        initialDir = track.absolutePath();
    }

    QStringList extensions = CoverArtUtils::supportedCoverArtExtensions();
    for (QStringList::iterator it = extensions.begin(); it != extensions.end(); ++it) {
        it->prepend("*.");
    }
    QString supportedText = QString("%1 (%2)").arg(tr("Image Files"))
            .arg(extensions.join(" "));

    // open file dialog
    QString selectedCoverPath = QFileDialog::getOpenFileName(
        this, tr("Change Cover Art"), initialDir, supportedText);
    if (selectedCoverPath.isEmpty()) {
        return;
    }

    // TODO(rryan): Ask if user wants to copy the file.

    CoverArt art;
    // Create a security token for the file.
    QFileInfo selectedCover(selectedCoverPath);
    SecurityTokenPointer pToken = Sandbox::openSecurityToken(
        selectedCover, true);
    art.image = QImage(selectedCoverPath);
    if (art.image.isNull()) {
        // TODO(rryan): feedback
        return;
    }
    art.info.type = CoverInfo::FILE;
    art.info.source = CoverInfo::USER_SELECTED;
    art.info.coverLocation = selectedCoverPath;
    art.info.hash = CoverArtUtils::calculateHash(art.image);
    qDebug() << "WCoverArtMenu::slotChange emit" << art;
    emit(coverArtSelected(art));
}

void WCoverArtMenu::slotUnset() {
    CoverArt art;
    art.info.type = CoverInfo::NONE;
    art.info.source = CoverInfo::USER_SELECTED;
    qDebug() << "WCoverArtMenu::slotUnset emit" << art;
    emit(coverArtSelected(art));
}
