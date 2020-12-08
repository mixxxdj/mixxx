#include "widget/wcoverartmenu.h"

#include <QFileDialog>
#include <QFileInfo>

#include "library/coverartutils.h"
#include "moc_wcoverartmenu.cpp"
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
    connect(m_pChange, &QAction::triggered, this, &WCoverArtMenu::slotChange);
    addAction(m_pChange);

    m_pUnset = new QAction(tr("Clear cover",
            "clears the set cover art -- does not touch files on disk"), this);
    connect(m_pUnset, &QAction::triggered, this, &WCoverArtMenu::slotUnset);
    addAction(m_pUnset);

    m_pReload = new QAction(tr("Reload from file/folder",
            "reload cover art from file metadata or folder"), this);
    connect(m_pReload, &QAction::triggered, this, &WCoverArtMenu::reloadCoverArt);
    addAction(m_pReload);
}

void WCoverArtMenu::setCoverArt(const CoverInfo& coverInfo) {
    m_coverInfo = coverInfo;
}

void WCoverArtMenu::slotChange() {
    QFileInfo fileInfo;
    if (!m_coverInfo.trackLocation.isEmpty()) {
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
    QString supportedText = QString("%1 (%2)").arg(tr("Image Files"), extensions.join(" "));

    // open file dialog
    QString selectedCoverPath = QFileDialog::getOpenFileName(
        this, tr("Change Cover Art"), initialDir, supportedText);
    if (selectedCoverPath.isEmpty()) {
        return;
    }

    // TODO(rryan): Ask if user wants to copy the file.

    CoverInfoRelative coverInfo;
    // Create a security token for the file.
    QFileInfo selectedCover(selectedCoverPath);
    SecurityTokenPointer pToken = Sandbox::openSecurityToken(
        selectedCover, true);
    QImage image(selectedCoverPath);
    if (image.isNull()) {
        // TODO(rryan): feedback
        return;
    }
    coverInfo.type = CoverInfo::FILE;
    coverInfo.source = CoverInfo::USER_SELECTED;
    coverInfo.coverLocation = selectedCoverPath;
    coverInfo.setImage(image);
    qDebug() << "WCoverArtMenu::slotChange emit" << coverInfo;
    emit coverInfoSelected(coverInfo);
}

void WCoverArtMenu::slotUnset() {
    CoverInfo coverInfo;
    coverInfo.type = CoverInfo::NONE;
    coverInfo.source = CoverInfo::USER_SELECTED;
    coverInfo.setImage();
    qDebug() << "WCoverArtMenu::slotUnset emit" << coverInfo;
    emit coverInfoSelected(coverInfo);
}
