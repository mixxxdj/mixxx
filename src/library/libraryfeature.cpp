#include "library/libraryfeature.h"

#include <QStandardPaths>

#include "library/library.h"
#include "library/parserm3u.h"
#include "library/parserpls.h"
// #include "library/trackset/searchcrate/dlgsearchcrate.h"
#include "moc_libraryfeature.cpp"
#include "util/logger.h"

// KEEP THIS cpp file to tell scons that moc should be called on the class!!!
// The reason for this is that LibraryFeature uses slots/signals and for this
// to work the code has to be precompiles by moc

namespace {

const mixxx::Logger kLogger("LibraryFeature");
const QString kIconPath = QStringLiteral(":/images/library/ic_library_%1.svg");

} // anonymous namespace

LibraryFeature::LibraryFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig,
        const QString& iconName)
        : QObject(pLibrary),
          m_pLibrary(pLibrary),
          m_pConfig(pConfig),
          m_iconName(iconName) {
    if (!m_iconName.isEmpty()) {
        m_icon = QIcon(kIconPath.arg(m_iconName));
    }
}

QStringList LibraryFeature::getPlaylistFiles(QFileDialog::FileMode mode) const {
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

bool LibraryFeature::exportPlaylistItemsIntoFile(
        QString playlistFilePath,
        const QList<QString>& playlistItemLocations,
        bool useRelativePath)    {
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
        //default export to M3U if file extension is missing
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
