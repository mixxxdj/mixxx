// libraryfeature.cpp
// Created 8/17/2009 by RJ Ryan (rryan@mit.edu)

#include "library/libraryfeature.h"

// KEEP THIS cpp file to tell scons that moc should be called on the class!!!
// The reason for this is that LibraryFeature uses slots/signals and for this
// to work the code has to be precompiles by moc
LibraryFeature::LibraryFeature(QObject *parent)
        : QObject(parent) {

}

LibraryFeature::LibraryFeature(UserSettingsPointer pConfig, QObject* parent)
        : QObject(parent),
          m_pConfig(pConfig) {
}

LibraryFeature::~LibraryFeature() {

}

QStringList LibraryFeature::getPlaylistFiles(QFileDialog::FileMode mode) {
    QString lastPlaylistDirectory = m_pConfig->getValueString(
            ConfigKey("[Library]", "LastImportExportPlaylistDirectory"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

    QFileDialog dialogg(NULL,
                     tr("Import Playlist"),
                     lastPlaylistDirectory,
                     tr("Playlist Files (*.m3u *.m3u8 *.pls *.csv)"));
    dialogg.setAcceptMode(QFileDialog::AcceptOpen);
    dialogg.setFileMode(mode);
    dialogg.setModal(true);

    // If the user refuses return
    if (! dialogg.exec()) return QStringList();
    return dialogg.selectedFiles();
}
