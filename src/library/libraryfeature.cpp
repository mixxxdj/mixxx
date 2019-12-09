// libraryfeature.cpp
// Created 8/17/2009 by RJ Ryan (rryan@mit.edu)

#include <QStandardPaths>

#include "library/libraryfeature.h"

#include "library/library.h"

// KEEP THIS cpp file to tell scons that moc should be called on the class!!!
// The reason for this is that LibraryFeature uses slots/signals and for this
// to work the code has to be precompiles by moc

LibraryFeature::LibraryFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig)
        : QObject(pLibrary),
          m_pLibrary(pLibrary),
          m_pConfig(pConfig) {
}

QStringList LibraryFeature::getPlaylistFiles(QFileDialog::FileMode mode) const {
    QString lastPlaylistDirectory = m_pConfig->getValue(
            ConfigKey("[Library]", "LastImportExportPlaylistDirectory"),
            QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

    QFileDialog dialog(NULL,
                     tr("Import Playlist"),
                     lastPlaylistDirectory,
                     tr("Playlist Files (*.m3u *.m3u8 *.pls *.csv)"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(mode);
    dialog.setModal(true);

    // If the user refuses return
    if (! dialog.exec()) return QStringList();
    return dialog.selectedFiles();
}
