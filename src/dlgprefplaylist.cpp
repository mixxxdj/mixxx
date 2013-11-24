/***************************************************************************
                          dlgprefplaylist.cpp  -  description
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
#include <QFileDialog>
#include <QUrl>

#include "dlgprefplaylist.h"
#include "soundsourceproxy.h"

#define MIXXX_ADDONS_URL "http://www.mixxx.org/wiki/doku.php/add-ons"


DlgPrefPlaylist::DlgPrefPlaylist(QWidget * parent, ConfigObject<ConfigValue> * config)
        : DlgPreferencePage(parent),
          m_pconfig(config) {
    setupUi(this);
    slotUpdate();
    checkbox_ID3_sync->setVisible(false);

    connect(PushButtonBrowsePlaylist, SIGNAL(clicked()),
            this, SLOT(slotBrowseDir()));
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

DlgPrefPlaylist::~DlgPrefPlaylist() {
}

void DlgPrefPlaylist::slotExtraPlugins() {
    QDesktopServices::openUrl(QUrl(MIXXX_ADDONS_URL));
}

void DlgPrefPlaylist::slotUpdate() {
    // Library Path
    LineEditSongfiles->setText(m_pconfig->getValueString(
            ConfigKey("[Playlist]","Directory")));
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

void DlgPrefPlaylist::slotBrowseDir() {
    QString fd = QFileDialog::getExistingDirectory(this,
                 tr("Choose music library directory"),
                 m_pconfig->getValueString(ConfigKey("[Playlist]","Directory")));
    if (fd != "") {
        LineEditSongfiles->setText(fd);
    }
}

void DlgPrefPlaylist::slotApply() {
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

    if (LineEditSongfiles->text() !=
            m_pconfig->getValueString(ConfigKey("[Playlist]","Directory"))) {
        m_pconfig->set(ConfigKey("[Playlist]","Directory"), LineEditSongfiles->text());
        emit(apply());
    }
    m_pconfig->Save();
}
