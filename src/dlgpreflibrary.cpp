/***************************************************************************
                          dlgpreflibrary.cpp  -  description
                             -------------------
    begin                : Thu Apr 17 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QStringList>
#include <QUrl>

#include "dlgpreflibrary.h"
#include "soundsourceproxy.h"

#define MIXXX_ADDONS_URL "http://www.mixxx.org/wiki/doku.php/add-ons"


DlgPrefLibrary::DlgPrefLibrary(QWidget * parent,
                               ConfigObject<ConfigValue> * config, Library *pLibrary)
        : DlgPreferencePage(parent),
          m_dirListModel(),
          m_pconfig(config),
          m_pLibrary(pLibrary) {
    setupUi(this);
    slotUpdate();
    checkbox_ID3_sync->setVisible(false);

    connect(this, SIGNAL(requestAddDir(QString)),
            m_pLibrary, SLOT(slotRequestAddDir(QString)));
    connect(this, SIGNAL(requestRemoveDir(QString, Library::RemovalType)),
            m_pLibrary, SLOT(slotRequestRemoveDir(QString, Library::RemovalType)));
    connect(this, SIGNAL(requestRelocateDir(QString,QString)),
            m_pLibrary, SLOT(slotRequestRelocateDir(QString,QString)));
    connect(PushButtonAddDir, SIGNAL(clicked()),
            this, SLOT(slotAddDir()));
    connect(PushButtonRemoveDir, SIGNAL(clicked()),
            this, SLOT(slotRemoveDir()));
    connect(PushButtonRelocateDir, SIGNAL(clicked()),
            this, SLOT(slotRelocateDir()));
    //connect(pushButtonM4A, SIGNAL(clicked()), this, SLOT(slotM4ACheck()));
    connect(pushButtonExtraPlugins, SIGNAL(clicked()),
            this, SLOT(slotExtraPlugins()));

    // plugins are loaded in src/main.cpp way early in boot so this is safe
    // here, doesn't need done at every slotUpdate
    QStringList plugins(SoundSourceProxy::supportedFileExtensionsByPlugins());
    if (plugins.length() > 0) {
        pluginsLabel->setText(plugins.join(", "));
    }
}

DlgPrefLibrary::~DlgPrefLibrary() {
}

void DlgPrefLibrary::initialiseDirList(){
    // save which index was selected
    const QString selected = dirList->currentIndex().data().toString();
    // clear and fill model
    m_dirListModel.clear();
    QStringList dirs = m_pLibrary->getDirs();
    foreach (QString dir, dirs) {
        m_dirListModel.appendRow(new QStandardItem(dir));
    }
    dirList->setModel(&m_dirListModel);
    dirList->setCurrentIndex(m_dirListModel.index(0, 0));
    // reselect index if it still exists
    for (int i=0 ; i<m_dirListModel.rowCount() ; ++i) {
        const QModelIndex index = m_dirListModel.index(i, 0);
        if (index.data().toString() == selected) {
            dirList->setCurrentIndex(index);
            break;
        }
    }
}

void DlgPrefLibrary::slotExtraPlugins() {
    QDesktopServices::openUrl(QUrl(MIXXX_ADDONS_URL));
}

void DlgPrefLibrary::slotUpdate() {
    initialiseDirList();
    //Bundled songs stat tracking
    checkBox_library_scan->setChecked((bool)m_pconfig->getValueString(
            ConfigKey("[Library]","RescanOnStartup")).toInt());
    checkbox_ID3_sync->setChecked((bool)m_pconfig->getValueString(
            ConfigKey("[Library]","WriteAudioTags")).toInt());
    checkBox_use_relative_path->setChecked((bool)m_pconfig->getValueString(
            ConfigKey("[Library]","UseRelativePathOnExport")).toInt());
    checkBox_show_rhythmbox->setChecked((bool)m_pconfig->getValueString(
            ConfigKey("[Library]","ShowRhythmboxLibrary"),"1").toInt());
    checkBox_show_banshee->setChecked((bool)m_pconfig->getValueString(
            ConfigKey("[Library]","ShowBansheeLibrary"),"1").toInt());
    checkBox_show_itunes->setChecked((bool)m_pconfig->getValueString(
            ConfigKey("[Library]","ShowITunesLibrary"),"1").toInt());
    checkBox_show_traktor->setChecked((bool)m_pconfig->getValueString(
            ConfigKey("[Library]","ShowTraktorLibrary"),"1").toInt());
}

void DlgPrefLibrary::slotAddDir() {
    QString fd = QFileDialog::getExistingDirectory(
        this, tr("Choose a music directory"),
        QDesktopServices::storageLocation(QDesktopServices::MusicLocation));
    if (!fd.isEmpty()) {
        emit(requestAddDir(fd));
        slotUpdate();
    }
}

void DlgPrefLibrary::slotRemoveDir() {
    QModelIndex index = dirList->currentIndex();
    QString fd = index.data().toString();
    QMessageBox removeMsgBox;

    removeMsgBox.setIcon(QMessageBox::Warning);
    removeMsgBox.setWindowTitle(tr("Confirm Directory Removal"));

    removeMsgBox.setText(tr(
        "Mixxx will no longer watch this directory for new tracks. "
        "What would you like to do with the tracks from this directory and "
        "subdirectories?"
        "<ul>"
        "<li>Hide all tracks from this directory and subdirectories.</li>"
        "<li>Delete all metadata for these tracks from Mixxx permanently.</li>"
        "<li>Leave the tracks unchanged in your library.</li>"
        "</ul>"
        "Hiding tracks saves their metadata in case you re-add them in the "
        "future."));
    removeMsgBox.setInformativeText(tr(
        "Metadata means all track details (artist, title, playcount, etc.) as "
        "well as beatgrids, hotcues, and loops. This choice only affects the "
        "Mixxx library. No files on disk will be changed or deleted."));

    QPushButton* cancelButton =
            removeMsgBox.addButton(QMessageBox::Cancel);
    QPushButton* hideAllButton = removeMsgBox.addButton(
        tr("Hide Tracks"), QMessageBox::AcceptRole);
    QPushButton* deleteAllButton = removeMsgBox.addButton(
        tr("Delete Track Metadata"), QMessageBox::AcceptRole);
    QPushButton* leaveUnchangedButton = removeMsgBox.addButton(
        tr("Leave Tracks Unchanged"), QMessageBox::AcceptRole);
    removeMsgBox.setDefaultButton(cancelButton);
    removeMsgBox.exec();

    if (removeMsgBox.clickedButton() == cancelButton) {
        return;
    }

    bool deleteAll = removeMsgBox.clickedButton() == deleteAllButton;
    bool hideAll = removeMsgBox.clickedButton() == hideAllButton;
    bool leaveUnchanged = removeMsgBox.clickedButton() == leaveUnchangedButton;

    Library::RemovalType removalType = Library::LeaveTracksUnchanged;
    if (leaveUnchanged) {
        removalType = Library::LeaveTracksUnchanged;
    } else if (deleteAll) {
        removalType = Library::PurgeTracks;
    } else if (hideAll) {
        removalType = Library::HideTracks;
    }

    emit(requestRemoveDir(fd, removalType));
    slotUpdate();
}

void DlgPrefLibrary::slotRelocateDir() {
    QModelIndex index = dirList->currentIndex();
    QString currentFd = index.data().toString();

    // If the selected directory exists, use it. If not, go up one directory (if
    // that directory exists). If neither exist, use the default music
    // directory.
    QString startDir = currentFd;
    QDir dir(startDir);
    if (!dir.exists() && dir.cdUp()) {
        startDir = dir.absolutePath();
    } else if (!dir.exists()) {
        startDir = QDesktopServices::storageLocation(
            QDesktopServices::MusicLocation);
    }

    QString fd = QFileDialog::getExistingDirectory(
        this, tr("Relink music directory to new location"), startDir);

    if (!fd.isEmpty()) {
        emit(requestRelocateDir(currentFd, fd));
        slotUpdate();
    }
}

void DlgPrefLibrary::slotApply() {
    m_pconfig->set(ConfigKey("[Library]","RescanOnStartup"),
                ConfigValue((int)checkBox_library_scan->isChecked()));
    m_pconfig->set(ConfigKey("[Library]","WriteAudioTags"),
                ConfigValue((int)checkbox_ID3_sync->isChecked()));
    m_pconfig->set(ConfigKey("[Library]","UseRelativePathOnExport"),
                ConfigValue((int)checkBox_use_relative_path->isChecked()));
    m_pconfig->set(ConfigKey("[Library]","ShowRhythmboxLibrary"),
                ConfigValue((int)checkBox_show_rhythmbox->isChecked()));
    m_pconfig->set(ConfigKey("[Library]","ShowBansheeLibrary"),
                ConfigValue((int)checkBox_show_banshee->isChecked()));
    m_pconfig->set(ConfigKey("[Library]","ShowITunesLibrary"),
                ConfigValue((int)checkBox_show_itunes->isChecked()));
    m_pconfig->set(ConfigKey("[Library]","ShowTraktorLibrary"),
                ConfigValue((int)checkBox_show_traktor->isChecked()));

    m_pconfig->Save();
}
