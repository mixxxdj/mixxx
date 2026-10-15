
#include "library/trackset/basetracksetfeature.h"

#include <QStandardPaths>

#include "library/parserm3u.h"
#include "library/parserpls.h"
#include "moc_basetracksetfeature.cpp"
#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("BaseTrackSetFeature");
} // anonymous namespace

BaseTrackSetFeature::BaseTrackSetFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig,
        const QString& rootViewName,
        const QString& iconName)
        : LibraryFeature(pLibrary, pConfig, iconName),
          m_rootViewName(rootViewName),
          m_pSidebarModel(make_parented<TreeItemModel>(this)) {
}

void BaseTrackSetFeature::pasteChild(const QModelIndex&) {
    emit pasteFromSidebar();
}

void BaseTrackSetFeature::activate() {
    emit switchToView(m_rootViewName);
    emit disableSearch();
    emit enableCoverArtDisplay(true);
}

QStringList BaseTrackSetFeature::getPlaylistFiles(QFileDialog::FileMode mode) const {
    QString lastPlaylistDirectory = m_pConfig->getValue(
            ConfigKey("[Library]", "LastImportExportPlaylistDirectory"),
            QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

    QFileDialog dialog(nullptr,
            tr("Import Playlist"),
            lastPlaylistDirectory,
            tr("Playlist Files (*.m3u *.m3u8 *.pls *.csv)"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(mode);
    dialog.setModal(true);

    // If the user refuses return
    if (!dialog.exec()) {
        return QStringList();
    }
    return dialog.selectedFiles();
}

QStringList BaseTrackSetFeature::getPlaylistFiles() const {
    return getPlaylistFiles(QFileDialog::ExistingFiles);
}

QString BaseTrackSetFeature::getPlaylistFile() const {
    const QStringList playListFiles = getPlaylistFiles();
    if (playListFiles.isEmpty()) {
        return QString(); // no file chosen
    } else {
        return playListFiles.first();
    }
}

bool BaseTrackSetFeature::exportPlaylistItemsIntoFile(
        QString playlistFilePath,
        const QList<QString>& playlistItemLocations,
        bool useRelativePath) {
    if (playlistFilePath.endsWith(
                QStringLiteral(".pls"),
                Qt::CaseInsensitive)) {
        return ParserPls::writePLSFile(
                playlistFilePath,
                playlistItemLocations,
                useRelativePath);
    } else if (playlistFilePath.endsWith(
                       QStringLiteral(".m3u8"),
                       Qt::CaseInsensitive)) {
        return ParserM3u::writeM3U8File(
                playlistFilePath,
                playlistItemLocations,
                useRelativePath);
    } else {
        // default export to M3U if file extension is missing
        if (!playlistFilePath.endsWith(
                    QStringLiteral(".m3u"),
                    Qt::CaseInsensitive)) {
            kLogger.debug()
                    << "No valid file extension for playlist export specified."
                    << "Appending .m3u and exporting to M3U.";
            playlistFilePath.append(QStringLiteral(".m3u"));
            if (QFileInfo::exists(playlistFilePath)) {
                auto overwrite = QMessageBox::question(
                        nullptr,
                        tr("Overwrite File?"),
                        tr("A playlist file with the name \"%1\" already exists.\n"
                           "The default \"m3u\" extension was added because none was specified.\n\n"
                           "Do you really want to overwrite it?")
                                .arg(playlistFilePath));
                if (overwrite != QMessageBox::StandardButton::Yes) {
                    return false;
                }
            }
        }
        return ParserM3u::writeM3UFile(
                playlistFilePath,
                playlistItemLocations,
                useRelativePath);
    }
}
