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

    m_pReload = new QAction(tr("Reload from file/folder",
            "reload cover art from file metadata or folder"), this);
    connect(m_pReload, SIGNAL(triggered()), this, SIGNAL(reloadCoverArt()));
    addAction(m_pReload);
}

void WCoverArtMenu::setCoverArt(const QString& trackLocation, const CoverInfo& coverInfo) {
    m_trackLocation = trackLocation;
    m_coverInfo = coverInfo;
}

void WCoverArtMenu::slotChange() {
    QFileInfo fileInfo;
    if (!m_trackLocation.isEmpty()) {
        fileInfo = QFileInfo(m_trackLocation);
    } else if (!m_coverInfo.trackLocation.isEmpty()) {
        fileInfo = QFileInfo(m_coverInfo.trackLocation);
    }

    QString initialDir;
    if (m_coverInfo.type == CoverInfo::FILE) {
        QFileInfo coverFile(fileInfo.dir(), m_coverInfo.coverLocation);
        initialDir = coverFile.absolutePath();
    } else {
        // Default to the track's directory if the cover is not
        // stored in a separate file.
        initialDir = fileInfo.absolutePath();
    }

    QStringList extensions = CoverArtUtils::supportedCoverArtExtensions();
    for (auto&& extension : extensions) {
        extension.prepend("*.");
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
    // TODO() here we may introduce a duplicate hash code
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
