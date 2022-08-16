#include "widget/wcoverartmenu.h"

#include <QBuffer>
#include <QFileDialog>
#include <QFileInfo>

#include "library/coverartutils.h"
#include "library/library_prefs.h"
#include "moc_wcoverartmenu.cpp"
#include "util/fileaccess.h"

WCoverArtMenu::WCoverArtMenu(QWidget* parent, UserSettingsPointer pConfig)
        : QMenu(parent),
          m_pConfig(pConfig) {
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

    CoverInfoRelative coverInfo;
    // Create a security token for the file.
    auto selectedCover = mixxx::FileAccess(mixxx::FileInfo(selectedCoverPath));
    QImage image(selectedCoverPath);
    if (image.isNull()) {
        // TODO(rryan): feedback
        return;
    }
    coverInfo.type = CoverInfo::FILE;
    coverInfo.source = CoverInfo::USER_SELECTED;
    coverInfo.coverLocation = selectedCoverPath;
    coverInfo.setImage(image);

    if (m_pConfig->getValue<bool>(mixxx::library::prefs::kCreateCopyOfTheCoverArtConfigKey)) {
        QString savedCoverArtLocation = fileInfo.absoluteDir().path() + "/" +
                fileInfo.completeBaseName() + ".jpg";
        if (QFile::exists(savedCoverArtLocation)) {
            QMessageBox saveBox(
                    QMessageBox::Question,
                    tr("Overwrite Existing Cover Art?"),
                    tr("The track already has a related Cover Art in same the path\n"
                       "Do you want to overwrite it?\n"
                       "Cover art located at: %1")
                            .arg(fileInfo.absoluteDir().path()));

            saveBox.setDefaultButton(QMessageBox::Yes);
            saveBox.addButton(tr("&Overwrite"), QMessageBox::AcceptRole);
            saveBox.addButton(tr("&Update without Overwrite"), QMessageBox::ApplyRole);
            saveBox.addButton(tr("&Discard"), QMessageBox::DestructiveRole);

            switch (saveBox.exec()) {
            case QMessageBox::DestructiveRole:
                qDebug() << "Cover art is not updated";
                return;

            case QMessageBox::ApplyRole:
                qDebug() << "Cover art is updated but didn't overwrite the existing cover art";
                qDebug() << "WCoverArtMenu::slotChange emit" << coverInfo;
                emit coverInfoSelected(coverInfo);
                return;

            case QMessageBox::AcceptRole:
                QFile::remove(savedCoverArtLocation);
                qDebug() << "Updated and overwritten";
                break;
            default:
                break;
            }
        }

        else {
            if (m_pConfig->getValue<bool>(mixxx::library::prefs::
                                kInformCoverArtLocationConfigKey)) {
                QMessageBox::information(nullptr,
                        tr("Cover Art Location"),
                        tr("Cover Art saved at \n%1").arg(fileInfo.absoluteDir().path()));
            }
        }

        QByteArray coverArtByteArray;
        QBuffer bufferCoverArt(&coverArtByteArray);
        bufferCoverArt.open(QIODevice::WriteOnly);
        image.save(&bufferCoverArt, "JPG");
        QFile coverArtFile(savedCoverArtLocation);
        coverArtFile.open(QIODevice::WriteOnly);
        coverArtFile.write(coverArtByteArray);
        bufferCoverArt.close();
        coverArtFile.close();
    }

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
