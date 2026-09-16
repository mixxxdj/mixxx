#include "widget/wcoverartmenu.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QMessageBox>

#include "library/coverartutils.h"
#include "moc_wcoverartmenu.cpp"
#include "sources/soundsourceproxy.h"
#include "util/assert.h"
#include "util/desktophelper.h"
#include "util/fileaccess.h"
#include "util/fileinfo.h"
#include "util/sandbox.h"

WCoverArtMenu::WCoverArtMenu(QWidget* parent)
        : QMenu(parent),
          m_isWorkerRunning(false) {
}

void WCoverArtMenu::createActions() {
    clear();

    VERIFY_OR_DEBUG_ASSERT(!m_coverInfo.trackLocation.isEmpty()) {
        return;
    }

    m_pChange = make_parented<QAction>(tr("Choose file", "change cover art location"), this);
    connect(m_pChange, &QAction::triggered, this, &WCoverArtMenu::slotChange);
    addAction(m_pChange);

    m_pUnset = make_parented<QAction>(
            tr("Clear cover",
                    "clears the set cover art -- does not touch files on disk"),
            this);
    connect(m_pUnset, &QAction::triggered, this, &WCoverArtMenu::slotUnset);
    addAction(m_pUnset);

    m_pReload = make_parented<QAction>(tr("Reload from file/folder",
                                               "reload cover art from file metadata or folder"),
            this);
    connect(m_pReload, &QAction::triggered, this, &WCoverArtMenu::reloadCoverArt);
    addAction(m_pReload);

    addSeparator();

    if (m_coverInfo.type == CoverInfo::FILE) {
        m_pShowCoverFileInBrowser = make_parented<QAction>(
                tr("Show cover file in browser",
                        "open a file browser with the cover file selected"),
                this);
        connect(m_pShowCoverFileInBrowser,
                &QAction::triggered,
                this,
                &WCoverArtMenu::slotShowCoverInFileBrowser);
        addAction(m_pShowCoverFileInBrowser);
    } else {
        m_pExtractCover = make_parented<QAction>(tr("Extract cover, save to file",
                                                         "extract cover art from track "
                                                         "metadata and save it to a file"),
                this);
        connect(m_pExtractCover, &QAction::triggered, this, &WCoverArtMenu::slotExtractCover);
        addAction(m_pExtractCover);
    }
}

void WCoverArtMenu::setCoverArt(const CoverInfo& coverInfo) {
    m_coverInfo = coverInfo;
    createActions();
}

void WCoverArtMenu::slotChange() {
    VERIFY_OR_DEBUG_ASSERT(!m_coverInfo.trackLocation.isEmpty()) {
        return;
    }

    QFileInfo trackFileInfo(m_coverInfo.trackLocation);

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

    QString coverArtCopyFilePath =
            trackFileInfo.absoluteFilePath().left(
                    trackFileInfo.absoluteFilePath().lastIndexOf('.') + 1) +
            selectedCoverExtension;

    VERIFY_OR_DEBUG_ASSERT(m_isWorkerRunning == false) {
        return;
    };

    m_worker.reset(new CoverArtCopyWorker(selectedCoverPath, coverArtCopyFilePath));

    connect(m_worker.data(),
            &CoverArtCopyWorker::started,
            this,
            &WCoverArtMenu::slotStarted);

    connect(m_worker.data(),
            &CoverArtCopyWorker::askOverwrite,
            this,
            &WCoverArtMenu::slotAskOverwrite);

    connect(m_worker.data(),
            &CoverArtCopyWorker::coverArtCopyFailed,
            this,
            &WCoverArtMenu::slotCoverArtCopyFailed);

    connect(m_worker.data(),
            &CoverArtCopyWorker::coverArtUpdated,
            this,
            &WCoverArtMenu::slotCoverArtUpdated);

    connect(m_worker.data(),
            &CoverArtCopyWorker::finished,
            this,
            &WCoverArtMenu::slotFinished);

    m_worker->start();
}

void WCoverArtMenu::slotUnset() {
    CoverInfo coverInfo;
    coverInfo.source = CoverInfo::USER_SELECTED;
    qDebug() << "WCoverArtMenu::slotUnset emit" << coverInfo;
    emit coverInfoSelected(coverInfo);
}

void WCoverArtMenu::slotShowCoverInFileBrowser() {
    VERIFY_OR_DEBUG_ASSERT(!m_coverInfo.trackLocation.isEmpty()) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(m_coverInfo.type == CoverInfo::FILE) {
        return;
    }

    QFileInfo trackFileInfo(m_coverInfo.trackLocation);
    QFileInfo coverFileInfo(trackFileInfo.dir(), m_coverInfo.coverLocation);
    if (!coverFileInfo.exists()) {
        qWarning() << "WCoverArtMenu::slotShowCoverInFileBrowser: source file not found!";
        return;
    }
    // FIXME Do we need this? We either have permission because we are
    // already allowed to open the track, or we assigned the cover file
    // manually which also required permissions.
    mixxx::FileInfo mixxxCoverFileInfo(coverFileInfo);
    SecurityTokenPointer pToken =
            Sandbox::openSecurityToken(
                    &mixxxCoverFileInfo, true);

    mixxx::DesktopHelper::openInFileBrowser(QStringList(coverFileInfo.absoluteFilePath()));
}

void WCoverArtMenu::slotExtractCover() {
    VERIFY_OR_DEBUG_ASSERT(!m_coverInfo.trackLocation.isEmpty()) {
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(m_coverInfo.type == CoverInfo::METADATA) {
        qWarning() << "WCoverArtMenu::slotExtractCover() unknown CoverInfo type"
                   << static_cast<int>(m_coverInfo.type);
    }

    // Extract embedded cover from the audio file's metadata
    QImage image = CoverArtUtils::extractEmbeddedCover(
            mixxx::FileAccess(mixxx::FileInfo(m_coverInfo.trackLocation)));

    if (image.isNull()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(tr("No cover art found."));
        msgBox.setDetailedText(tr(
                "The track does not contain embedded cover art "
                "and no external cover file was found."));
        msgBox.exec();
        return;
    }

    QFileInfo trackFileInfo(m_coverInfo.trackLocation);
    QString defaultFileName = trackFileInfo.completeBaseName() + QStringLiteral(".jpg");

    QString savePath = QFileDialog::getSaveFileName(
            this,
            tr("Save cover art to file"),
            trackFileInfo.absolutePath() + QStringLiteral("/") + defaultFileName,
            tr("Image Files (*.jpg *.jpeg *.png *.bmp *.gif *.tiff *.tif)"));
    if (savePath.isEmpty()) {
        return;
    }

    // Determine the image format from the file extension
    QString format = QFileInfo(savePath).suffix();
    if (format.compare(QStringLiteral("jpg"), Qt::CaseInsensitive) == 0 ||
            format.compare(QStringLiteral("jpeg"), Qt::CaseInsensitive) == 0) {
        format = QStringLiteral("JPEG");
    } else if (format.compare(QStringLiteral("png"), Qt::CaseInsensitive) == 0) {
        format = QStringLiteral("PNG");
    } else if (format.compare(QStringLiteral("bmp"), Qt::CaseInsensitive) == 0) {
        format = QStringLiteral("BMP");
    } else if (format.compare(QStringLiteral("gif"), Qt::CaseInsensitive) == 0) {
        format = QStringLiteral("GIF");
    } else if (format.compare(QStringLiteral("tiff"), Qt::CaseInsensitive) == 0 ||
            format.compare(QStringLiteral("tif"), Qt::CaseInsensitive) == 0) {
        format = QStringLiteral("TIFF");
    } else {
        // Default to PNG for unknown extensions
        format = QStringLiteral("PNG");
        savePath += QStringLiteral(".png");
    }

    // use src/util/ImageFileData ??
    if (!image.save(savePath, format.toUtf8().constData())) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(tr("Failed to save cover art."));
        msgBox.setDetailedText(tr("Could not write the image to:\n%1").arg(savePath));
        msgBox.exec();
        return;
    }

    // Success message
    // Disabled, too much noise
    //
    // QMessageBox msgBox(this);
    // msgBox.setIcon(QMessageBox::Information);
    // msgBox.setText(tr("Cover art saved successfully."));
    // msgBox.setDetailedText(tr("Cover art extracted and saved to:\n%1").arg(savePath));
    // msgBox.exec();

    // Open file browser
    mixxx::DesktopHelper::openInFileBrowser(QStringList(savePath));
}

void WCoverArtMenu::slotStarted() {
    m_isWorkerRunning = true;
}

void WCoverArtMenu::slotCoverArtUpdated(const CoverInfoRelative& coverInfo) {
    qDebug() << "WCoverArtMenu::slotChange emit" << coverInfo;
    emit coverInfoSelected(coverInfo);
}

void WCoverArtMenu::slotAskOverwrite(const QString& coverArtAbsolutePath,
        std::promise<CoverArtCopyWorker::OverwriteAnswer>* promise) {
    QFileInfo coverArtInfo(coverArtAbsolutePath);
    QString coverArtName = coverArtInfo.completeBaseName();
    QString coverArtFolder = coverArtInfo.absolutePath();
    QMessageBox overwrite_box(
            QMessageBox::Warning,
            tr("Cover Art File Already Exists"),
            tr("File: %1\n"
               "Folder: %2\n"
               "Override existing file?\n"
               "This can not be undone!")
                    .arg(coverArtName, coverArtFolder));
    overwrite_box.addButton(QMessageBox::Yes);
    overwrite_box.addButton(QMessageBox::No);

    switch (overwrite_box.exec()) {
    case QMessageBox::No:
        promise->set_value(CoverArtCopyWorker::OverwriteAnswer::Cancel);
        return;
    case QMessageBox::Yes:
        promise->set_value(CoverArtCopyWorker::OverwriteAnswer::Overwrite);
        return;
    }
}

void WCoverArtMenu::slotCoverArtCopyFailed(const QString& errorMessage) {
    QMessageBox copyFailBox;
    copyFailBox.setText(errorMessage);
    copyFailBox.exec();
}

void WCoverArtMenu::slotFinished() {
    m_isWorkerRunning = false;
}
