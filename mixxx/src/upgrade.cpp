/***************************************************************************
                          upgrade.cpp  -  description
                             -------------------
    begin                : Fri Mar 13 2009
    copyright            : (C) 2009 by Sean M. Pappalardo
    email                : pegasus@c64.org
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QPixmap>
#include <QtCore>
#include <QMessageBox>
#include <QTranslator>
#include "defs_version.h"
#include "controllers/defs_controllers.h"
#include "track/beat_preferences.h"
#include "configobject.h"
#include "upgrade.h"


Upgrade::Upgrade()
{
    m_bFirstRun = false;
    m_bUpgraded = false;
}

Upgrade::~Upgrade()
{

}

// We return the ConfigObject here because we have to make changes to the
// configuration and the location of the file may change between releases.
ConfigObject<ConfigValue>* Upgrade::versionUpgrade(const QString& settingsPath) {

/*  Pre-1.7.0:
*
*   Since we didn't store version numbers in the config file prior to 1.7.0,
*   we check to see if the user is upgrading if his config files are in the old location,
*   since we moved them in 1.7.0. This code takes care of moving them.
*/

    QString oldLocation = QDir::homePath().append("/%1");
#ifdef __WINDOWS__
    QFileInfo* pre170Config = new QFileInfo(oldLocation.arg("mixxx.cfg"));
#else
    QFileInfo* pre170Config = new QFileInfo(oldLocation.arg(".mixxx.cfg"));
#endif

    if (pre170Config->exists()) {

        // Move the files to their new location
        QString newLocation = settingsPath;

        if (!QDir(newLocation).exists()) {
            qDebug() << "Creating new settings directory" << newLocation;
            QDir().mkpath(newLocation);
        }

        newLocation.append("%1");
        QString errorText = "Error moving your %1 file %2 to the new location %3: \n";

#ifdef __WINDOWS__
        QString oldFilePath = oldLocation.arg("mixxxtrack.xml");
#else
        QString oldFilePath = oldLocation.arg(".mixxxtrack.xml");
#endif

        QString newFilePath = newLocation.arg("mixxxtrack.xml");
        QFile* oldFile = new QFile(oldFilePath);
        if (oldFile->exists()) {
            if (oldFile->copy(newFilePath)) {
                oldFile->remove();
                m_bUpgraded = true;
            }
            else {
                if (oldFile->error()==14) qDebug() << errorText.arg("library", oldFilePath, newFilePath) << "The destination file already exists.";
                else qDebug() << errorText.arg("library", oldFilePath, newFilePath) << "Error #" << oldFile->error();
            }
        }
        delete oldFile;

#ifdef __WINDOWS__
        oldFilePath = oldLocation.arg("mixxxbpmschemes.xml");
#else
        oldFilePath = oldLocation.arg(".mixxxbpmscheme.xml");
#endif
        newFilePath = newLocation.arg("mixxxbpmscheme.xml");
        oldFile = new QFile(oldFilePath);
        if (oldFile->exists()) {
            if (oldFile->copy(newFilePath))
                oldFile->remove();
            else {
                if (oldFile->error()==14) qDebug() << errorText.arg("settings", oldFilePath, newFilePath) << "The destination file already exists.";
                else qDebug() << errorText.arg("settings", oldFilePath, newFilePath) << "Error #" << oldFile->error();
            }
        }
        delete oldFile;
#ifdef __WINDOWS__
        oldFilePath = oldLocation.arg("MixxxMIDIBindings.xml");
#else
        oldFilePath = oldLocation.arg(".MixxxMIDIBindings.xml");
#endif
        newFilePath = newLocation.arg("MixxxMIDIBindings.xml");
        oldFile = new QFile(oldFilePath);
        if (oldFile->exists()) {
            qWarning() << "The MIDI mapping file format has changed in this version of Mixxx. You will need to reconfigure your MIDI controller. See the Wiki for full details on the new format.";
            if (oldFile->copy(newFilePath))
                oldFile->remove();
            else {
                if (oldFile->error()==14) qDebug() << errorText.arg("MIDI mapping", oldFilePath, newFilePath) << "The destination file already exists.";
                else qDebug() << errorText.arg("MIDI mapping", oldFilePath, newFilePath) << "Error #" << oldFile->error();
            }
        }
        // Tidy up
        delete oldFile;

        QFile::remove(oldLocation.arg(".MixxxMIDIDevice.xml")); // Obsolete file, so just delete it

#ifdef __WINDOWS__
        oldFilePath = oldLocation.arg("mixxx.cfg");
#else
        oldFilePath = oldLocation.arg(".mixxx.cfg");
#endif
        newFilePath = newLocation.arg(SETTINGS_FILE);
        oldFile = new QFile(oldFilePath);
        if (oldFile->copy(newFilePath))
            oldFile->remove();
        else {
                if (oldFile->error()==14) qDebug() << errorText.arg("configuration", oldFilePath, newFilePath) << "The destination file already exists.";
                else qDebug() << errorText.arg("configuration", oldFilePath, newFilePath) << "Error #" << oldFile->error();
            }
        delete oldFile;

    }
    // Tidy up
    delete pre170Config;
    // End pre-1.7.0 code


/***************************************************************************
*                           Post-1.7.0 upgrade code
*
*   Add entries to the IF ladder below if anything needs to change from the
*   previous to the current version. This allows for incremental upgrades
*   incase a user upgrades from a few versions prior.
****************************************************************************/

    // Read the config file from home directory
    ConfigObject<ConfigValue> *config = new ConfigObject<ConfigValue>(settingsPath + SETTINGS_FILE);

    QString configVersion = config->getValueString(ConfigKey("[Config]","Version"));

    if (configVersion.isEmpty()) {

#ifdef __APPLE__
        qDebug() << "Config version is empty, trying to read pre-1.9.0 config";
        //Try to read the config from the pre-1.9.0 final directory on OS X (we moved it in 1.9.0 final)
        QFile* oldFile = new QFile(QDir::homePath().append("/").append(".mixxx/mixxx.cfg"));
        if (oldFile->exists()) {
            qDebug() << "Found pre-1.9.0 config for OS X";
            config = new ConfigObject<ConfigValue>(QDir::homePath().append("/").append(".mixxx/mixxx.cfg"));
            //Note: We changed SETTINGS_PATH in 1.9.0 final on OS X so it must be hardcoded to ".mixxx" here for legacy.
            configVersion = config->getValueString(ConfigKey("[Config]","Version"));
            delete oldFile;
        }
        else {
#endif
            //This must have been the first run... right? :)
            qDebug() << "No version number in configuration file. Setting to" << VERSION;
            config->set(ConfigKey("[Config]","Version"), ConfigValue(VERSION));
            m_bFirstRun = true;
            return config;
#ifdef __APPLE__
        }
#endif
    }

    // If it's already current, stop here
    if (configVersion == VERSION) {
        qDebug() << "Configuration file is at the current version" << VERSION;
        return config;
    }

    // Allows for incremental upgrades incase someone upgrades from a few versions prior
    // (I wish we could do a switch on a QString.)
    /*
    // Examples, since we didn't store the version number prior to v1.7.0
    if (configVersion.startsWith("1.6.0")) {
        qDebug() << "Upgrading from v1.6.0 to 1.6.1...";
        // Upgrade tasks go here
        configVersion = "1.6.1";
        config->set(ConfigKey("[Config]","Version"), ConfigValue("1.6.1"));
    }
    if (configVersion.startsWith("1.6.1")) {
        qDebug() << "Upgrading from v1.6.1 to 1.7.0...";
        // Upgrade tasks go here
        configVersion = "1.7.0";
        config->set(ConfigKey("[Config]","Version"), ConfigValue("1.7.0"));
    }
    */

    //We use the following blocks to detect if this is the first time
    //you've run the latest version of Mixxx. This lets us show
    //the promo tracks stats agreement stuff for all users that are
    //upgrading Mixxx.

    if (configVersion.startsWith("1.7")) {
        qDebug() << "Upgrading from v1.7.x...";
        // Upgrade tasks go here
        // Nothing to change, really
        configVersion = "1.8.0";
        config->set(ConfigKey("[Config]","Version"), ConfigValue("1.8.0"));
    }

    if (configVersion.startsWith("1.8.0~beta1") ||
        configVersion.startsWith("1.8.0~beta2")) {
        qDebug() << "Upgrading from v1.8.0~beta...";
        // Upgrade tasks go here
        configVersion = "1.8.0";
        config->set(ConfigKey("[Config]","Version"), ConfigValue("1.8.0"));
    }
    if (configVersion.startsWith("1.8") || configVersion.startsWith("1.9.0beta1")) {
        qDebug() << "Upgrading from" << configVersion << "...";
        // Upgrade tasks go here
#ifdef __APPLE__
        QString OSXLocation180 = QDir::homePath().append("/").append(".mixxx");
        QString OSXLocation190 = settingsPath;
        QDir newOSXDir(OSXLocation190);
        newOSXDir.mkpath(OSXLocation190);

        QList<QPair<QString, QString> > dirsToMove;
        dirsToMove.push_back(QPair<QString, QString>(OSXLocation180, OSXLocation190));
        dirsToMove.push_back(QPair<QString, QString>(OSXLocation180 + "/midi", OSXLocation190 + "midi"));
        dirsToMove.push_back(QPair<QString, QString>(OSXLocation180 + "/presets", OSXLocation190 + "presets"));

        QListIterator<QPair<QString, QString> > dirIt(dirsToMove);
        QPair<QString, QString> curPair;
        while (dirIt.hasNext())
        {
            curPair = dirIt.next();
            qDebug() << "Moving" << curPair.first << "to" << curPair.second;
            QDir oldSubDir(curPair.first);
            QDir newSubDir(curPair.second);
            newSubDir.mkpath(curPair.second); //Create the new destination directory

            QStringList contents = oldSubDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
            QStringListIterator it(contents);
            QString cur;
            //Iterate over all the files in the source directory and copy them to the dest dir.
            while (it.hasNext())
            {
                cur = it.next();
                QString src = curPair.first + "/" + cur;
                QString dest = curPair.second + "/" + cur;
                qDebug() << "Copying" << src << "to" << dest;
                if (!QFile::copy(src, dest))
                {
                    qDebug() << "Failed to move file during upgrade.";
                }
            }

            //Rename the old directory.
            newOSXDir.rename(OSXLocation180, OSXLocation180 + "-1.8");
        }
        //Reload the configuration file from the new location.
        //(We want to make sure we save to the new location...)
        config = new ConfigObject<ConfigValue>(settingsPath + SETTINGS_FILE);
#endif
        configVersion = "1.9.0";
        config->set(ConfigKey("[Config]","Version"), ConfigValue("1.9.0"));
    }
    if (configVersion.startsWith("1.9") || configVersion.startsWith("1.10")) {
        qDebug() << "Upgrading from v1.9.x/1.10.x...";

        bool successful = true;

        qDebug() << "Copying midi/ to controllers/";
        QString midiPath = legacyUserPresetsPath(config);
        QString controllerPath = userPresetsPath(config);
        QDir oldDir(midiPath);
        QDir newDir(controllerPath);
        newDir.mkpath(controllerPath);  // create the new directory

        QStringList contents = oldDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        QStringListIterator it(contents);
        QString cur;
        //Iterate over all the files in the source directory and copy them to the dest dir.
        while (it.hasNext()) {
            cur = it.next();
            if (newDir.exists(cur)) {
                qDebug() << cur << "already exists in"
                         << controllerPath << "Skipping.";
                continue;
            }
            QString src = oldDir.absoluteFilePath(cur);
            QString dest = newDir.absoluteFilePath(cur);
            qDebug() << "Copying" << src << "to" << dest;
            if (!QFile::copy(src, dest)) {
                qDebug() << "Failed to copy file during upgrade.";
                successful = false;
            }
        }

        bool reanalyze_choice = askReanalyzeBeats();
        config->set(ConfigKey(BPM_CONFIG_KEY,
                              BPM_REANALYZE_WHEN_SETTINGS_CHANGE),
                    ConfigValue(reanalyze_choice));

        if (successful) {
            qDebug() << "Upgrade Successful";
            configVersion = "1.11.0";
            m_bUpgraded = true;
            config->set(ConfigKey("[Config]","Version"),
                        ConfigValue(configVersion));
        } else {
            qDebug() << "Upgrade Failed";
        }
    }
    // Next applicable release goes here
    /*
    if (configVersion.startsWith("1.11")) {
        qDebug() << "Upgrading from v1.11.x...";

        // Upgrade tasks go here

        if (successful) {
            configVersion = VERSION;
            m_bUpgraded = true;
            config->set(ConfigKey("[Config]","Version"), ConfigValue(VERSION));
        }
        else {
            qDebug() << "Upgrade failed!\n";
        }
    }
    */

    if (configVersion == VERSION) qDebug() << "Configuration file is now at the current version" << VERSION;
    else {
        /* Way too verbose, this confuses the hell out of Linux users when they see this:
        qWarning() << "Configuration file is at version" << configVersion
                   << "and I don't know how to upgrade it to the current" << VERSION
                   << "\n   (That means a function to do this needs to be added to upgrade.cpp.)"
                   << "\n-> Leaving the configuration file version as-is.";
        */
        qWarning() << "Configuration file is at version" << configVersion
                   << "instead of the current" << VERSION;
    }

    return config;
}

bool Upgrade::askReanalyzeBeats() {
    QString windowTitle =
            QMessageBox::tr("Upgrading Mixxx from v1.9.x/1.10.x.");
    QString mainHeading =
            QMessageBox::tr("Mixxx has a new and improved beat detector.");
    QString paragraph1 = QMessageBox::tr(
        "When you load tracks, Mixxx can re-analyze them "
        "and generate new, more accurate beatgrids. This will make "
        "automatic beatsync and looping more reliable.");
    QString paragraph2 = QMessageBox::tr(
        "This does not affect saved cues, hotcues, playlists, or crates.");
    QString paragraph3 = QMessageBox::tr(
        "If you do not want Mixxx to re-analyze your tracks, choose "
        "\"Keep Current Beatgrids\". You can change this setting at any time "
        "from the \"Beat Detection\" section of the Preferences.");
    QString keepCurrent = QMessageBox::tr("Keep Current Beatgrids");
    QString generateNew = QMessageBox::tr("Generate New Beatgrids");

    QMessageBox msgBox;
    msgBox.setIconPixmap(QPixmap(":/images/mixxx-icon.png"));
    msgBox.setWindowTitle(windowTitle);
    msgBox.setText(QString("<html><h2>%1</h2><p>%2</p><p>%3</p><p>%4</p></html>")
                   .arg(mainHeading, paragraph1, paragraph2, paragraph3));
    msgBox.addButton(keepCurrent, QMessageBox::NoRole);
    QPushButton* OverwriteButton = msgBox.addButton(
        generateNew, QMessageBox::YesRole);
    msgBox.setDefaultButton(OverwriteButton);
    msgBox.exec();

    if (msgBox.clickedButton() == (QAbstractButton*)OverwriteButton) {
        return true;
    }
    return false;
}
