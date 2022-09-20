#include "widget/wcoverartmenu.h"

#include <QFileDialog>
#include <QFileInfo>

#include "library/coverartutils.h"
#include "moc_wcoverartmenu.cpp"
#include "util/assert.h"
#include "util/fileaccess.h"
#include "util/imagefiledata.h"

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
    QFileInfo trackFileInfo;

    VERIFY_OR_DEBUG_ASSERT(!m_coverInfo.trackLocation.isEmpty()) {
        return;
    }

    trackFileInfo = QFileInfo(m_coverInfo.trackLocation);

    QString initialDir;
    if (m_coverInfo.type == CoverInfo::FILE) {
        QFileInfo coverFile(trackFileInfo.dir(), m_coverInfo.coverLocation);
        initialDir = coverFile.absolutePath();
    } else {
        // Default to the track's directory if the cover is not
        // stored in a separate file.
        initialDir = trackFileInfo.absolutePath();
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

    QString selectedCoverExtension = QFileInfo(selectedCoverPath).suffix();

    CoverInfoRelative coverInfo;
    // Create a security token for the file.
    auto selectedCover = mixxx::FileAccess(mixxx::FileInfo(selectedCoverPath));
    // TODO: this is file access in main thread. Move this to another thread.
    ImageFileData image = ImageFileData::fromFilePath(selectedCoverPath);
    if (image.isNull()) {
        // TODO(rryan): feedback
        return;
    }
    coverInfo.type = CoverInfo::FILE;
    coverInfo.source = CoverInfo::USER_SELECTED;
    coverInfo.coverLocation = selectedCoverPath;
    coverInfo.setImage(image);

    QString coverArtCopyFilePath =
            trackFileInfo.absoluteFilePath().left(
                    trackFileInfo.absoluteFilePath().lastIndexOf('.') + 1) +
            selectedCoverExtension;

    if (QFileInfo(m_coverInfo.trackLocation).canonicalPath() ==
            QFileInfo(selectedCoverPath).canonicalPath()) {
        qDebug() << "Track and selected cover art are in the same path:"
                 << QFileInfo(selectedCoverPath).canonicalPath()
                 << "Cover art updated without copying";
        emit coverInfoSelected(coverInfo);
        qDebug() << "WCoverArtMenu::slotChange emit" << coverInfo;
        return;
    }

    m_worker.reset(new CoverArtCopyWorker(image, coverArtCopyFilePath));
    m_worker->run();
    if (m_worker->isCoverUpdated()) {
        qDebug() << "WCoverArtMenu::slotChange emit" << coverInfo;
        emit coverInfoSelected(coverInfo);
    }
}

void WCoverArtMenu::slotUnset() {
    CoverInfo coverInfo;
    coverInfo.type = CoverInfo::NONE;
    coverInfo.source = CoverInfo::USER_SELECTED;
    coverInfo.setImage();
    qDebug() << "WCoverArtMenu::slotUnset emit" << coverInfo;
    emit coverInfoSelected(coverInfo);
}
