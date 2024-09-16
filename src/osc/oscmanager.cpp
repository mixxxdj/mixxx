#include "osc/oscmanager.h"

#include <QDateTime>
#include <QDir>
#include <QMessageBox>
#include <QMutex>
#include <QStorageInfo>

#include "control/controlpushbutton.h"
#include "engine/enginemixer.h"
#include "engine/sidechain/engineosc.h"
#include "engine/sidechain/enginesidechain.h"
#include "errordialoghandler.h"
#include "moc_oscmanager.cpp"
#include "osc/defs_osc.h"

#define MIN_DISK_FREE 1024 * 1024 * 1024ll // one gibibyte

OscManager::OscManager(UserSettingsPointer pConfig, EngineMixer* pEngine)
        : m_pConfig(pConfig),
          m_oscDir(""),
          m_osc_base_file(""),
          m_oscFile(""),
          m_oscLocation(""),
          m_bOsc(false),
          m_iNumberOfBytesOsc(0),
          m_iNumberOfBytesOscSplit(0),
          m_split_size(0),
          m_split_time(0),
          m_iNumberSplits(0),
          m_secondsOsc(0),
          m_secondsOscSplit(0) {
    m_pToggleOsc = std::make_unique<ControlPushButton>(
            ConfigKey(OSC_PREF_KEY, "toggle_osc"));
    connect(m_pToggleOsc.get(),
            &ControlPushButton::valueChanged,
            this,
            &OscManager::slotToggleOsc);
    m_pCoOscStatus = std::make_unique<ControlObject>(ConfigKey(OSC_PREF_KEY, "status"));

    m_split_size = getFileSplitSize();
    m_split_time = getFileSplitSeconds();

    // Register EngineOscv with the engine sidechain.
    EngineSideChain* pSidechain = pEngine->getSideChain();
    if (pSidechain) {
        EngineOsc* pEngineOsc = new EngineOsc(m_pConfig);
        connect(pEngineOsc,
                &EngineOsc::isOsc,
                this,
                &OscManager::slotIsOsc);
        connect(pEngineOsc,
                &EngineOsc::bytesOsc,
                this,
                &OscManager::slotBytesOsc);
        connect(pEngineOsc,
                &EngineOsc::durationOsc,
                this,
                &OscManager::slotDurationOsc);
        pSidechain->addSideChainWorker(pEngineOsc);
    }
}

QString OscManager::formatDateTimeForFilename(const QDateTime& dateTime) const {
    // Use a format based on ISO 8601. Windows does not support colons in
    // filenames so we can't use them anywhere.
    QString formatted = dateTime.toString("yyyy-MM-dd_hh'h'mm'm'ss's'");
    return formatted;
}

void OscManager::slotSetOsc(bool osc) {
    if (osc && !isOscActive()) {
        startOsc();
    } else if (!osc && isOscActive()) {
        stopOsc();
    }
}

void OscManager::slotToggleOsc(double value) {
    bool toggle = static_cast<bool>(value);
    if (toggle) {
        if (isOscActive()) {
            stopOsc();
        } else {
            startOsc();
        }
    }
}

qint64 OscManager::getFreeSpace() {
    // returns the free space on the osc location in bytes
    // return -1 if the free space could not be determined
    qint64 rv = -1;
    QStorageInfo storage(m_oscDir);
    if (storage.isValid()) {
        rv = storage.bytesAvailable();
    }
    return rv;
}

void OscManager::startOsc() {
    QString encodingType = m_pConfig->getValueString(
            ConfigKey(OSC_PREF_KEY, "Encoding"));
    QString fileExtension = EncoderFactory::getFactory()
                                    .getFormatFor(encodingType)
                                    .fileExtension;

    m_iNumberOfBytesOscSplit = 0;
    m_secondsOscSplit=0;
    m_iNumberOfBytesOsc = 0;
    m_secondsOsc=0;
    m_dfSilence = false;
    m_dfCounter=0;
    m_split_size = getFileSplitSize();
    m_split_time = getFileSplitSeconds();
    if (m_split_time < INT_MAX) {
        qDebug() << "Split time is:" << m_split_time;
    }
    else {
        qDebug() << "Split size is:" << m_split_size;
    }

    m_iNumberSplits = 1;
    // Append file extension.
    QString date_time_str = formatDateTimeForFilename(QDateTime::currentDateTime());
    m_oscFile = QString("%1.%2")
                              .arg(date_time_str, fileExtension);

    // Storing the absolutePath of the osc file without file extension.
    m_osc_base_file = getOscDir();
    m_osc_base_file.append("/").append(date_time_str);
    // Appending file extension to get the filelocation.
    m_oscLocation = m_osc_base_file + QChar('.') + fileExtension;
    m_pConfig->set(ConfigKey(OSC_PREF_KEY, "Path"), m_oscLocation);
    m_pConfig->set(ConfigKey(OSC_PREF_KEY, "CuePath"), ConfigValue(m_osc_base_file + QStringLiteral(".cue")));

    m_pCoOscStatus->set(OSC_READY);
}

void OscManager::splitContinueOsc()
{
    ++m_iNumberSplits;
    m_secondsOsc+=m_secondsOscSplit;

    m_iNumberOfBytesOscSplit = 0;
    m_secondsOscSplit=0;

    QString encodingType = m_pConfig->getValueString(ConfigKey(OSC_PREF_KEY, "Encoding"));
    QString fileExtension = EncoderFactory::getFactory()
                                    .getFormatFor(encodingType)
                                    .fileExtension;

    QString new_base_filename = m_osc_base_file + QStringLiteral("part") + QString::number(m_iNumberSplits);
    m_oscLocation = new_base_filename + QChar('.') + fileExtension;

    m_pConfig->set(ConfigKey(OSC_PREF_KEY, "Path"), m_oscLocation);
    m_pConfig->set(ConfigKey(OSC_PREF_KEY, "CuePath"), ConfigValue(new_base_filename + QStringLiteral(".cue")));
    m_oscFile = QFileInfo(m_oscLocation).fileName();

    m_pCoOscStatus->set(OSC_SPLIT_CONTINUE);
}

void OscManager::stopOsc() {
    qDebug() << "Osc stopped";
    m_pCoOscStatus->set(OSC_OFF);
    m_oscFile = "";
    m_oscLocation = "";
    m_iNumberOfBytesOsc = 0;
    m_secondsOsc = 0;
}

void OscManager::setOscDir() {
    QDir oscDir(m_pConfig->getValueString(
        ConfigKey(OSC_PREF_KEY, "Directory")));
    // Note: the default ConfigKey for oscDir is set in DlgOsc::DlgPrefOsc.

    if (!oscDir.exists()) {
        if (oscDir.mkpath(oscDir.absolutePath())) {
            qDebug() << "Created folder" << oscDir.absolutePath() << "for osc";
        } else {
            // Using qt_error_string() since QDir has not yet a wrapper for error strings.
            // https://bugreports.qt.io/browse/QTBUG-1483
            qDebug() << "Failed to create folder" << oscDir.absolutePath()
                     << "for osc:" << qt_error_string();
        }
    }
    m_oscDir = oscDir.absolutePath();
    qDebug() << "Oscs folder set to" << m_oscDir;
}

QString& OscManager::getOscDir() {
    // Update current osc dir from preferences.
    setOscDir();
    return m_oscDir;
}

// Only called when osc is active.
void OscManager::slotDurationOsc(quint64 duration) {
    if (m_secondsOscSplit != duration) {
        m_secondsOscSplit = duration;
        if (duration >= m_split_time) {
            qDebug() << "Splitting after " << duration << " seconds";
            // This will reuse the previous filename but append a suffix.
            splitContinueOsc();
        }
        emit durationOsc(getOscDurationStr(m_secondsOsc+m_secondsOscSplit));
    }
}

// Copy from the implementation in engineosc.cpp
QString OscManager::getOscDurationStr(unsigned int duration) {
    return QString("%1:%2")
                 .arg(duration / 60, 2, 'f', 0, '0')   // minutes
                 .arg(duration % 60, 2, 'f', 0, '0');  // seconds
}

// Only called when osc is active.
void OscManager::slotBytesOsc(int bytes) {
    // auto conversion to quint64
    m_iNumberOfBytesOsc += bytes;
    m_iNumberOfBytesOscSplit += bytes;

    //Split before reaching the max size. m_split_size has some headroom, as
    //seen in the constant definitions in defs_osc.h. Also, note that
    //bytes are increased in the order of 10s of KBs each call.
    if (m_iNumberOfBytesOscSplit >= m_split_size) {
        qDebug() << "Splitting after " << m_iNumberOfBytesOsc << " bytes written";
        // This will reuse the previous filename but append a suffix.
        splitContinueOsc();
    }
    emit bytesOsc(m_iNumberOfBytesOsc);

    // check for free space

    // we only check every 1 MB of data to minimize syscalls
    m_dfCounter -= bytes;

    if (m_dfCounter > 0) {
        return;
    }

    qint64 dfree = getFreeSpace();
    // reset counter
    m_dfCounter = 1024 * 1024;
    if (dfree == -1) {
        qDebug() << "can't determine free space";
        return;
    }
    if (dfree > MIN_DISK_FREE) {
        m_dfSilence = false;
    } else if (m_dfSilence != true) {
        // suppress further warnings until the situation has cleared
        m_dfSilence = true;
        // we run out of diskspace and should warn the user.
        // FIXME(poelzi) temporary display a error message. Replace this with Message Infrastructure when ready
        warnFreespace();
    }
}

void OscManager::warnFreespace() {
    qWarning() << "OscManager: less than 1 GiB free space";
    ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
    props->setType(DLG_WARNING);
    props->setTitle(tr("Low Disk Space Warning"));
    props->setText(tr("There is less than 1 GiB of usable space in the osc folder"));
    props->setKey("OscManager::warnFreespace");   // To prevent multiple windows for the same error

    props->addButton(QMessageBox::Ok);
    props->setDefaultButton(QMessageBox::Ok);
    props->setEscapeButton(QMessageBox::Ok);
    props->setModal(false);

    ErrorDialogHandler::instance()->requestErrorDialog(props);
}

void OscManager::slotIsOsc(bool isOscActive, bool error) {
    //qDebug() << "SlotIsOsc " << isOsc << error;

    // Notify the GUI controls, see dlgosc.cpp.
    m_bOsc = isOscActive;
    emit isOsc(isOscActive);

    if (error) {
        ErrorDialogProperties* props = ErrorDialogHandler::instance()->newDialogProperties();
        props->setType(DLG_WARNING);
        props->setTitle(tr("Osc"));
        props->setText("<html>"+tr("Could not create audio file for osc!")
                       +"<p>"+tr("Ensure there is enough free disk space and you have write permission for the Oscs folder.")
                       +"<p>"+tr("You can change the location of the Oscs folder in Preferences -> Osc.")
                       +"</p></html>");
        ErrorDialogHandler::instance()->requestErrorDialog(props);
    }
}

bool OscManager::isOscActive() const {
    return m_bOsc;
}

const QString& OscManager::getOscFile() const {
    return m_oscFile;
}

const QString& OscManager::getOscLocation() const {
    return m_oscLocation;
}

quint64 OscManager::getFileSplitSize() {
    QString fileSizeStr = m_pConfig->getValueString(ConfigKey(OSC_PREF_KEY, "FileSize"));
    if (fileSizeStr == SPLIT_650MB) {
        return SIZE_650MB;
    } else if (fileSizeStr == SPLIT_700MB) {
        return SIZE_700MB;
    } else if (fileSizeStr == SPLIT_1024MB) {
        return SIZE_1GB;
    } else if (fileSizeStr == SPLIT_2048MB) {
        return SIZE_2GB;
    } else if (fileSizeStr == SPLIT_4096MB) {
        return SIZE_4GB;
    } else if (fileSizeStr == SPLIT_60MIN) {
        return SIZE_4GB; //Ignore size limit. use time limit
    } else if (fileSizeStr == SPLIT_74MIN) {
        return SIZE_4GB; //Ignore size limit. use time limit
    } else if (fileSizeStr == SPLIT_80MIN) {
        return SIZE_4GB; //Ignore size limit. use time limit
    } else if (fileSizeStr == SPLIT_120MIN) {
        return SIZE_4GB; //Ignore size limit. use time limit
    } else {
        return SIZE_4GB; // default
    }
}

unsigned int OscManager::getFileSplitSeconds() {
    QString fileSizeStr = m_pConfig->getValueString(ConfigKey(OSC_PREF_KEY, "FileSize"));
    if (fileSizeStr == SPLIT_60MIN) {
        return 60*60;
    } else if (fileSizeStr == SPLIT_74MIN) {
        return 74*60;
    } else if (fileSizeStr == SPLIT_80MIN) {
        return 80*60;
    } else if (fileSizeStr == SPLIT_120MIN) {
        return 120*60;
    } else { // Do not limit by time for the rest.
        return INT_MAX;
    }
}
