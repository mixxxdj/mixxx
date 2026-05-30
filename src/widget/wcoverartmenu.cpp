#include "widget/wcoverartmenu.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QWidgetAction>

#include "library/coverartcache.h"
#include "library/coverartutils.h"
#include "moc_wcoverartmenu.cpp"
#include "util/assert.h"

WCoverArtMenu::WCoverArtMenu(QWidget* parent)
        : QMenu(parent),
          m_pCoverPreviewAction(nullptr),
          m_pCoverLabel(nullptr),
          m_isWorkerRunning(false) {
    createActions();
    connect(CoverArtCache::instance(),
            &CoverArtCache::coverFound,
            this,
            &WCoverArtMenu::slotCoverFound);
}

WCoverArtMenu::~WCoverArtMenu() {
    delete m_pCoverPreviewAction;
    delete m_pChange;
    delete m_pReload;
    delete m_pUnset;
}

void WCoverArtMenu::createActions() {
    m_pCoverPreviewAction = new QWidgetAction(this);
    m_pCoverLabel = new QLabel(this);
    m_pCoverLabel->setFixedSize(kCoverPreviewSize, kCoverPreviewSize);
    m_pCoverLabel->setAlignment(Qt::AlignCenter);
    m_pCoverLabel->setScaledContents(false);
    m_pCoverLabel->setStyleSheet(
            QStringLiteral("QLabel { background-color: palette(base); border: 1px solid palette(mid); }"));
    m_pCoverPreviewAction->setDefaultWidget(m_pCoverLabel);
    addAction(m_pCoverPreviewAction);

    addSeparator();

    m_pChange = new QAction(tr("Choose file", "change cover art location"), this);
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
    m_pCoverLabel->clear();
    m_pCoverLabel->setToolTip(QString());
    if (coverInfo.trackLocation.isEmpty()) {
        return;
    }
    CoverArtCache::requestCover(this, coverInfo);
}

void WCoverArtMenu::slotCoverFound(const QObject* requester,
        const CoverInfo& coverInfo,
        const QPixmap& pixmap) {
    if (requester != this || coverInfo != m_coverInfo || pixmap.isNull()) {
        return;
    }
    QPixmap scaled = pixmap.scaled(kCoverPreviewSize,
            kCoverPreviewSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
    m_pCoverLabel->setPixmap(scaled);
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
