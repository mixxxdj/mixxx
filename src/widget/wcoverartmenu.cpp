#include <QFileDialog>
#include <QStringBuilder>

#include "widget/wcoverartmenu.h"
#include "library/coverartutils.h"

WCoverArtMenu::WCoverArtMenu(QWidget *parent)
        : QMenu(parent) {
    createActions();
    addActions();
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

    m_pUnset = new QAction(tr("Unset cover",
            "unset cover art - load default"), this);
    connect(m_pUnset, SIGNAL(triggered()), this, SLOT(slotUnset()));

    m_pReload = new QAction(tr("Reload from track/folder",
            "reload just cover art, using the search algorithm"), this);
    connect(m_pReload, SIGNAL(triggered()), this, SLOT(slotReload()));
}

void WCoverArtMenu::addActions() {
    addAction(m_pChange);
    addAction(m_pUnset);
    addAction(m_pReload);
}

void WCoverArtMenu::show(QPoint pos, CoverInfo info, TrackPointer pTrack) {
    if (info.trackId < 1) {
        return;
    }
    m_coverInfo = info;
    m_pTrack = pTrack;
    popup(pos);
}

void WCoverArtMenu::slotChange() {
    if (m_coverInfo.trackId < 1 || m_pTrack.isNull()) {
        return;
    }

    // get initial directory (trackdir or coverdir)
    QString initialDir;
    QString trackPath = m_pTrack->getDirectory();
    if (m_coverInfo.coverLocation.isEmpty()) {
        initialDir = trackPath;
    } else {
        initialDir = m_coverInfo.coverLocation;
    }

    QStringList extensions = CoverArtUtils::supportedCoverArtExtensions();
    for (QStringList::iterator it = extensions.begin(); it != extensions.end(); ++it) {
        it->prepend("*.");
    }
    QString supportedText = QString("%1 (%2)").arg(tr("Image Files"))
            .arg(extensions.join(" "));

    // open file dialog
    QString selectedCover = QFileDialog::getOpenFileName(
        this, tr("Change Cover Art"), initialDir, supportedText);

    if (selectedCover.isEmpty()) {
        return;
    }

    // if the cover comes from an external dir,
    // we copy it to the track directory.
    QString newCover;
    QFileInfo fileInfo(selectedCover);
    QString coverPath = fileInfo.absolutePath();
    if (trackPath == coverPath) {
        newCover = selectedCover;
    } else {
        QDir trackDir(trackPath);
        QString ext = fileInfo.suffix();
        QString mixxxCoverFile = trackDir.filePath("mixxx-cover." % ext);
        QStringList filepaths;
        filepaths << trackDir.filePath("cover." % ext)
                  << trackDir.filePath("album." % ext)
                  << mixxxCoverFile;

        foreach (QString filepath, filepaths) {
            if (QFile::copy(selectedCover, filepath)) {
                newCover = filepath;
                break;
            }
        }

        if (newCover.isEmpty()) {
            // overwrites "mixxx-cover"
            QFile::remove(mixxxCoverFile);
            if (QFile::copy(selectedCover, mixxxCoverFile)) {
                newCover = mixxxCoverFile;
            }
        }
    }

    QPixmap px(newCover);
    emit(coverLocationUpdated(newCover, m_coverInfo.coverLocation, px));
}

void WCoverArtMenu::slotReload() {
    if (m_coverInfo.trackId < 1 || m_pTrack.isNull()) {
        return;
    }
    // TODO(rryan): implement, need to signal to parent
}

void WCoverArtMenu::slotUnset() {
    if (m_coverInfo.trackId < 1) {
        return;
    }
    QPixmap px;
    emit(coverLocationUpdated(QString(), m_coverInfo.coverLocation, QPixmap()));
}
