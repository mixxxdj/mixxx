#include "preferences/upgrade.h"

#include <QPixmap>
#include <QMessageBox>
#include <QPushButton>
#include <QTranslator>
#include <QScopedPointer>

#include "preferences/usersettings.h"
#include "preferences/beatdetectionsettings.h"
#include "database/mixxxdb.h"
#include "controllers/defs_controllers.h"
#include "defs_version.h"
#include "library/library_preferences.h"
#include "library/trackcollection.h"
#include "util/cmdlineargs.h"
#include "util/math.h"
#include "util/db/dbconnectionpooler.h"
#include "util/db/dbconnectionpooled.h"

Upgrade::Upgrade()
        : m_bFirstRun(false),
          m_bRescanLibrary(false) {
}

Upgrade::~Upgrade() {
}

// We return the UserSettings here because we have to make changes to the
// configuration and the location of the file may change between releases.
UserSettingsPointer Upgrade::versionUpgrade(const QString& settingsPath) {

/*  Pre-1.7.0:
*
*   Since we didn't store version numbers in the config file prior to 1.7.0,
*   we check to see if the user is upgrading if his config files are in the old location,
*   since we moved them in 1.7.0. This code takes care of moving them.
*/

    QDir oldLocation = QDir(QDir::homePath());
#ifdef __WINDOWS__
    QFileInfo* pre170Config = new QFileInfo(oldLocation.filePath("mixxx.cfg"));
#else
    QFileInfo* pre170Config = new QFileInfo(oldLocation.filePath(".mixxx.cfg"));
#endif

    if (pre170Config->exists()) {

        // Move the files to their new location
        QDir newLocation = QDir(settingsPath);

        if (!newLocation.exists()) {
            qDebug() << "Creating new settings directory" << newLocation.absolutePath();
            newLocation.mkpath(".");
        }

        QString errorText = "Error moving your %1 file %2 to the new location %3: \n";

#ifdef __WINDOWS__
        QString oldFilePath = oldLocation.filePath("mixxxtrack.xml");
#else
        QString oldFilePath = oldLocation.filePath(".mixxxtrack.xml");
#endif

        QString newFilePath = newLocation.filePath("mixxxtrack.xml");
        QFile* oldFile = new QFile(oldFilePath);
        if (oldFile->exists()) {
            if (oldFile->copy(newFilePath)) {
                oldFile->remove();
            }
            else {
                if (oldFile->error() == 14) {
                    qDebug() << errorText.arg(
                                        "library", oldFilePath, newFilePath)
                             << "The destination file already exists.";
                } else {
                    qDebug() << errorText.arg(
                                        "library", oldFilePath, newFilePath)
                             << "Error #" << oldFile->error();
                }
            }
        }
        delete oldFile;

#ifdef __WINDOWS__
        oldFilePath = oldLocation.filePath("mixxxbpmschemes.xml");
#else
        oldFilePath = oldLocation.filePath(".mixxxbpmscheme.xml");
#endif
        newFilePath = newLocation.filePath("mixxxbpmscheme.xml");
        oldFile = new QFile(oldFilePath);
        if (oldFile->exists()) {
            if (oldFile->copy(newFilePath)) {
                oldFile->remove();
            } else {
                if (oldFile->error() == 14) {
                    qDebug() << errorText.arg(
                                        "settings", oldFilePath, newFilePath)
                             << "The destination file already exists.";
                } else {
                    qDebug() << errorText.arg(
                                        "settings", oldFilePath, newFilePath)
                             << "Error #" << oldFile->error();
                }
            }
        }
        delete oldFile;
#ifdef __WINDOWS__
        oldFilePath = oldLocation.filePath("MixxxMIDIBindings.xml");
#else
        oldFilePath = oldLocation.filePath(".MixxxMIDIBindings.xml");
#endif
        newFilePath = newLocation.filePath("MixxxMIDIBindings.xml");
        oldFile = new QFile(oldFilePath);
        if (oldFile->exists()) {
            qWarning() << "The MIDI mapping file format has changed in this version of Mixxx. You will need to reconfigure your MIDI controller. See the Wiki for full details on the new format.";
            if (oldFile->copy(newFilePath)) {
                oldFile->remove();
            } else {
                if (oldFile->error() == 14) {
                    qDebug()
                            << errorText.arg(
                                       "MIDI mapping", oldFilePath, newFilePath)
                            << "The destination file already exists.";
                } else {
                    qDebug()
                            << errorText.arg(
                                       "MIDI mapping", oldFilePath, newFilePath)
                            << "Error #" << oldFile->error();
                }
            }
        }
        // Tidy up
        delete oldFile;
#ifdef __WINDOWS__
        QFile::remove(oldLocation.filePath("MixxxMIDIDevice.xml")); // Obsolete file, so just delete it
#else
        QFile::remove(oldLocation.filePath(".MixxxMIDIDevice.xml")); // Obsolete file, so just delete it
#endif

#ifdef __WINDOWS__
        oldFilePath = oldLocation.filePath("mixxx.cfg");
#else
        oldFilePath = oldLocation.filePath(".mixxx.cfg");
#endif
        newFilePath = newLocation.filePath(SETTINGS_FILE);
        oldFile = new QFile(oldFilePath);
        if (oldFile->copy(newFilePath)) {
            oldFile->remove();
        } else {
            if (oldFile->error() == 14) {
                qDebug() << errorText.arg(
                                    "configuration", oldFilePath, newFilePath)
                         << "The destination file already exists.";
            } else {
                qDebug() << errorText.arg(
                                    "configuration", oldFilePath, newFilePath)
                         << "Error #" << oldFile->error();
            }
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
*   in case a user upgrades from a few versions prior.
****************************************************************************/

    // Read the config file from home directory
    UserSettingsPointer config(new ConfigObject<ConfigValue>(
        QDir(settingsPath).filePath(SETTINGS_FILE)));

    QString configVersion = config->getValueString(ConfigKey("[Config]","Version"));

    if (configVersion.isEmpty()) {

#ifdef __APPLE__
        qDebug() << "Config version is empty, trying to read pre-1.9.0 config";
        // Try to read the config from the pre-1.9.0 final directory on OS X (we moved it in 1.9.0 final)
        QScopedPointer<QFile> oldConfigFile(new QFile(QDir::homePath().append("/").append(".mixxx/mixxx.cfg")));
        if (oldConfigFile->exists() && ! CmdlineArgs::Instance().getSettingsPathSet()) {
            qDebug() << "Found pre-1.9.0 config for OS X";
            // Note: We changed SETTINGS_PATH in 1.9.0 final on OS X so it must be hardcoded to ".mixxx" here for legacy.
            config = UserSettingsPointer(new ConfigObject<ConfigValue>(
                QDir::homePath().append("/.mixxx/mixxx.cfg")));
            // Just to be sure all files like logs and soundconfig go with mixxx.cfg
            // TODO(XXX) Trailing slash not needed anymore as we switches from String::append
            // to QDir::filePath elsewhere in the code. This is candidate for removal.
            CmdlineArgs::Instance().setSettingsPath(QDir::homePath().append("/.mixxx/"));
            configVersion = config->getValueString(ConfigKey("[Config]","Version"));
        }
        else {
#elif __WINDOWS__
        qDebug() << "Config version is empty, trying to read pre-1.12.0 config";
        // Try to read the config from the pre-1.12.0 final directory on Windows (we moved it in 1.12.0 final)
        QScopedPointer<QFile> oldConfigFile(new QFile(QDir::homePath().append("/Local Settings/Application Data/Mixxx/mixxx.cfg")));
        if (oldConfigFile->exists() && ! CmdlineArgs::Instance().getSettingsPathSet()) {
            qDebug() << "Found pre-1.12.0 config for Windows";
            // Note: We changed SETTINGS_PATH in 1.12.0 final on Windows so it must be hardcoded to "Local Settings/Application Data/Mixxx/" here for legacy.
            config = UserSettingsPointer(new ConfigObject<ConfigValue>(
                QDir::homePath().append("/Local Settings/Application Data/Mixxx/mixxx.cfg")));
            // Just to be sure all files like logs and soundconfig go with mixxx.cfg
            // TODO(XXX) Trailing slash not needed anymore as we switches from String::append
            // to QDir::filePath elsewhere in the code. This is candidate for removal.
            CmdlineArgs::Instance().setSettingsPath(QDir::homePath().append("/Local Settings/Application Data/Mixxx/"));
            configVersion = config->getValueString(ConfigKey("[Config]","Version"));
        }
        else {
#endif
            // This must have been the first run... right? :)
            qDebug() << "No version number in configuration file. Setting to" << MIXXX_VERSION;
            config->set(ConfigKey("[Config]","Version"), ConfigValue(MIXXX_VERSION));
            m_bFirstRun = true;
            return config;
#ifdef __APPLE__
        }
#elif __WINDOWS__
        }
#endif
    }

    // If it's already current, stop here
    if (configVersion == MIXXX_VERSION) {
        qDebug() << "Configuration file is at the current version" << MIXXX_VERSION;
        return config;
    }

    // Allows for incremental upgrades in case someone upgrades from a few versions prior
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

    // We use the following blocks to detect if this is the first time
    // you've run the latest version of Mixxx. This lets us show
    // the promo tracks stats agreement stuff for all users that are
    // upgrading Mixxx.

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
            newSubDir.mkpath(curPair.second); // Create the new destination directory

            QStringList contents = oldSubDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
            QStringListIterator it(contents);
            QString cur;
            // Iterate over all the files in the source directory and copy them to the dest dir.
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

            // Rename the old directory.
            newOSXDir.rename(OSXLocation180, OSXLocation180 + "-1.8");
        }
        // Reload the configuration file from the new location.
        // (We want to make sure we save to the new location...)
        config = UserSettingsPointer(new ConfigObject<ConfigValue>(
            QDir(settingsPath).filePath(SETTINGS_FILE)));
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
        // Iterate over all the files in the source directory and copy them to the dest dir.
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
        BeatDetectionSettings bpmSettings(config);
        bpmSettings.setReanalyzeWhenSettingsChange(reanalyze_choice);

        if (successful) {
            qDebug() << "Upgrade Successful";
            configVersion = "1.11.0";
            config->set(ConfigKey("[Config]","Version"),
                        ConfigValue(configVersion));
        } else {
            qDebug() << "Upgrade Failed";
        }
    }

    if (configVersion.startsWith("1.11")) {
        qDebug() << "Upgrading from v1.11.x...";
        bool successful = false;
        {
            MixxxDb mixxxDb(config);
            const mixxx::DbConnectionPooler dbConnectionPooler(
                    mixxxDb.connectionPool());
            if (dbConnectionPooler.isPooling()) {
                QSqlDatabase dbConnection = mixxx::DbConnectionPooled(mixxxDb.connectionPool());
                DEBUG_ASSERT(dbConnection.isOpen());
                if (MixxxDb::initDatabaseSchema(dbConnection)) {
                    TrackCollection tc(config);
                    tc.connectDatabase(dbConnection);

                    // upgrade to the multi library folder settings
                    QString currentFolder = config->getValueString(PREF_LEGACY_LIBRARY_DIR);
                    // to migrate the DB just add the current directory to the new
                    // directories table
                    // NOTE(rryan): We don't have to ask for sandbox permission to this
                    // directory because the normal startup integrity check in Library will
                    // notice if we don't have permission and ask for access. Also, the
                    // Sandbox isn't setup yet at this point in startup because it relies on
                    // the config settings path and this function is what loads the config
                    // so it's not ready yet.
                    successful = tc.addDirectory(currentFolder);

                    tc.disconnectDatabase();
                }
            }
        }

        // ask for library rescan to activate cover art. We can later ask for
        // this variable when the library scanner is constructed.
        m_bRescanLibrary = askReScanLibrary();

        // Versions of mixxx until 1.11 had a hack that multiplied gain by 1/2,
        // which was compensation for another hack that set replaygain to a
        // default of 6.  We've now removed all of the hacks, so subtracting
        // 6 from everyone's replay gain should keep things consistent for
        // all users.
        int oldReplayGain = config->getValue(
                ConfigKey("[ReplayGain]", "InitialReplayGainBoost"), 6);
        int newReplayGain = math_max(-6, oldReplayGain - 6);
        config->set(ConfigKey("[ReplayGain]", "InitialReplayGainBoost"),
                    ConfigValue(newReplayGain));

        // if everything until here worked fine we can mark the configuration as
        // updated
        if (successful) {
            configVersion = MIXXX_VERSION;
            config->set(ConfigKey("[Config]","Version"), ConfigValue(MIXXX_VERSION));
        }
        else {
            qDebug() << "Upgrade failed!\n";
        }
    }

    if (configVersion.startsWith("1.12") ||
        configVersion.startsWith("2.0") ||
        configVersion.startsWith("2.1.0")) {
        // No special upgrade required, just update the value.
        configVersion = MIXXX_VERSION;
        config->set(ConfigKey("[Config]","Version"), ConfigValue(MIXXX_VERSION));
    }

    if (configVersion == MIXXX_VERSION) {
        qDebug() << "Configuration file is now at the current version" << MIXXX_VERSION;
    } else {
        qWarning() << "Configuration file is at version" << configVersion
                   << "instead of the current" << MIXXX_VERSION;
    }

    return config;
}

bool Upgrade::askReScanLibrary() {
    QMessageBox msgBox;
    msgBox.setIconPixmap(QPixmap(":/images/mixxx_icon.svg"));
    msgBox.setWindowTitle(QMessageBox::tr("Upgrading Mixxx"));
    msgBox.setText(QMessageBox::tr("Mixxx now supports displaying cover art.\n"
                      "Do you want to scan your library for cover files now?"));
    QPushButton* rescanButton = msgBox.addButton(
        QMessageBox::tr("Scan"), QMessageBox::AcceptRole);
    msgBox.addButton(QMessageBox::tr("Later"), QMessageBox::RejectRole);
    msgBox.setDefaultButton(rescanButton);
    msgBox.exec();

    return msgBox.clickedButton() == rescanButton;
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
    msgBox.setIconPixmap(QPixmap(":/images/mixxx_icon.svg"));
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
