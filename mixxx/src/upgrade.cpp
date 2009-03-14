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

/*  This file is to be #included in mixxx.cpp. It's separated for clarity.   */

// We return the ConfigObject here because we have to make changes to the
// configuration and the location of the file may change between releases.
static ConfigObject<ConfigValue>* versionUpgrade() {

/*  Pre-1.6.2:
*
*   Since we didn't store version numbers in the config file prior to 1.6.2,
*   we check to see if the user is upgrading if his config files are in the old location,
*   since we moved them in 1.6.2. This code takes care of moving them.
*/

    QString oldLocation = QDir::homePath().append("/%1");
    QFileInfo* pre162Config = new QFileInfo(oldLocation.arg(".%1").arg(SETTINGS_FILE));
    if (pre162Config->exists()) {
    
        // Move the files to their new location
        QString newLocation = QDir::homePath().append("/").append(SETTINGS_PATH);
        
        if (!QDir(newLocation).exists()) {
            qDebug() << "Creating new settings directory" << newLocation;
            QDir().mkpath(newLocation);
        }
        
        newLocation.append("%1");
        
        QString oldFilePath = oldLocation.arg(".mixxxtrack.xml");
        QString newFilePath = newLocation.arg("mixxxtrack.xml");
        QFile* oldFile = new QFile(oldFilePath);
        if (oldFile->exists()) {
            if (oldFile->copy(newFilePath))
                oldFile->remove();
            else qWarning() << "Error" << oldFile->error() << "moving your library file" << oldFilePath << "to the new location" << newFilePath;
        }
        delete oldFile;
        
        oldFilePath = oldLocation.arg(".mixxxbpmscheme.xml");
        newFilePath = newLocation.arg("mixxxbpmscheme.xml");
        oldFile = new QFile(oldFilePath);
        if (oldFile->exists()) {
            if (oldFile->copy(newFilePath))
                oldFile->remove();
            else qWarning() << "Error" << oldFile->error() << "moving your settings file" << oldFilePath << "to the new location" << newFilePath;
        }
        delete oldFile;
        
        oldFilePath = oldLocation.arg(".MixxxMIDIBindings.xml");
        newFilePath = newLocation.arg("MixxxMIDIBindings.xml");
        oldFile = new QFile(oldFilePath);
        if (oldFile->exists()) {
            qWarning() << "The MIDI mapping file format has changed in this version of Mixxx. You will need to reconfigure your MIDI controller. See the Wiki for full details on the new format.";
            if (oldFile->copy(newFilePath))
                oldFile->remove();
            else qWarning() << "Error" << oldFile->error() << "moving your MIDI mapping file" << oldFilePath << "to the new location" << newFilePath;
        }
        // Tidy up
        delete oldFile;

        QFile::remove(oldLocation.arg(".MixxxMIDIDevice.xml")); // Obsolete file, so just delete it
        
        oldFilePath = oldLocation.arg(".mixxx.cfg");
        newFilePath = newLocation.arg("mixxx.cfg");
        oldFile = new QFile(oldFilePath);
        if (oldFile->copy(newFilePath))
            oldFile->remove();
        else qWarning() << "Error" << oldFile->error() << "moving your configuration file" << oldFilePath << "to the new location" << newFilePath;
        delete oldFile;
        
    }
    // Tidy up
    delete pre162Config;
    // End pre-1.6.2 code
    
    
/***************************************************************************
*                           Post-1.6.2 upgrade code
*
*   Add entries to the IF ladder below if anything needs to change from the
*   previous to the current version. This allows for incremental upgrades
*   incase a user upgrades from a few versions prior.
****************************************************************************/

    // Read the config file from home directory
    ConfigObject<ConfigValue> *config = new ConfigObject<ConfigValue>(QDir::homePath().append("/").append(SETTINGS_PATH).append(SETTINGS_FILE));

    QString configVersion = config->getValueString(ConfigKey("[Config]","Version"));

    if (configVersion.isEmpty()) {
        qDebug() << "No version number in configuration file. Setting to" << VERSION;
        config->set(ConfigKey("[Config]","Version"), ConfigValue(VERSION));
        return config;
    }
    
    // Allows for incremental upgrades incase someone upgrades from a few versions prior
    // (I wish we could do a switch on a QString.)
    /*
    // Examples, since we didn't store the version number prior to v1.6.2
    if (configVersion == "1.6.0") {
        qDebug() << "Upgrading from v1.6.0 to 1.6.1...";
        // Upgrade tasks here
        configVersion = "1.6.1";
        config->set(ConfigKey("[Config]","Version"), ConfigValue("1.6.1"));
    }
    if (configVersion == "1.6.1") {
        qDebug() << "Upgrading from v1.6.1 to 1.6.2...";
        // Upgrade tasks here
        configVersion = "1.6.2";
        config->set(ConfigKey("[Config]","Version"), ConfigValue("1.6.2"));
    }
    */
    
    // For the next release, if needed:
//     if (configVersion == "1.6.2") {
//         qDebug() << "Upgrading from v1.6.2 to 1.7.0...";
//         // Upgrade tasks here
//         configVersion = "1.7.0";
//         config->set(ConfigKey("[Config]","Version"), ConfigValue("1.7.0"));
//     }

    if (configVersion == VERSION) qDebug() << "At current version" << VERSION;

    return config;
}
