#include "mixer/nowplaying.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <algorithm>

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "mixer/playerinfo.h"
#include "moc_nowplaying.cpp"
#include "util/versionstore.h"

namespace {
const ConfigKey kConfigKeyNowPlayingEnabled = ConfigKey("[NowPlaying]", "Enabled");
const ConfigKey kConfigKeyNowPlayingPollInterval = ConfigKey("[NowPlaying]", "PollInterval");
const ConfigKey kConfigKeyNowPlayingAppendMode = ConfigKey("[NowPlaying]", "AppendMode");
const ConfigKey kConfigKeyNowPlayingAddTimestamp = ConfigKey("[NowPlaying]", "AddTimestamp");
const ConfigKey kConfigKeyNowPlayingArchive = ConfigKey("[NowPlaying]", "Archive");

constexpr bool kDefaultNowPlayingEnabled = true;
constexpr int kDefaultPollInterval = 1000;
constexpr bool kDefaultAppendMode = false;
constexpr bool kDefaultAddTimestamp = true;
constexpr bool kDefaultArchive = true;

constexpr int MAX_DECKS = 4;
constexpr double VOLUME_THRESHOLD = 0.001;
constexpr bool debugNowPlaing = true;
} // namespace

// NowPlaying Main
NowPlaying::NowPlaying(UserSettingsPointer pConfig, QObject* parent)
        : QObject(parent),
          m_pConfig(std::move(pConfig)),
          m_pWorker(nullptr) {
}

NowPlaying::~NowPlaying() {
    shutdown();
}

void NowPlaying::initialize() {
    m_pWorker = new NowPlayingWorker(m_pConfig, nullptr);
    m_pWorker->moveToThread(&m_workerThread);

    connect(&m_workerThread, &QThread::finished, m_pWorker, &QObject::deleteLater);
    connect(m_pWorker,
            &NowPlayingWorker::updateWindowTitle,
            this,
            &NowPlaying::slotUpdateWindowTitle,
            Qt::QueuedConnection);

    m_workerThread.start();
    QMetaObject::invokeMethod(m_pWorker, &NowPlayingWorker::initialize, Qt::QueuedConnection);
}

void NowPlaying::shutdown() {
    if (m_pWorker) {
        QMetaObject::invokeMethod(m_pWorker, &NowPlayingWorker::shutdown, Qt::QueuedConnection);
    }
    m_workerThread.quit();
    m_workerThread.wait();
}

void NowPlaying::slotUpdateWindowTitle(const QString& title, const QString& filePath) {
    QWidget* pParentWidget = qobject_cast<QWidget*>(parent());
    if (!pParentWidget) {
        return;
    }

    pParentWidget->setWindowTitle(title);
    pParentWidget->setWindowFilePath(filePath);
}

// NowPlay Worker
NowPlayingWorker::NowPlayingWorker(UserSettingsPointer pConfig, QObject* parent)
        : QObject(parent),
          m_pConfig(std::move(pConfig)),
          m_pMasterCrossfaderProxy(nullptr),
          m_pollTimer(nullptr),
          m_sessionArchived(false),
          m_nowPlayingEnabled(true),
          m_pollIntervalMs(1000),
          m_appendMode(false),
          m_addTimestamp(true),
          m_archive(true) {
    VERIFY_OR_DEBUG_ASSERT(m_pConfig) {
        qDebug() << "[NowPlaying]: UserSettingsPointer is null!";
        return;
    }

    const QString settingsPath = m_pConfig->getSettingsPath();
    m_nowPlayingFilePath = settingsPath + "/NowPlaying.txt";
    m_archiveFolderPath = settingsPath + "/NowPlaying";

    for (int i = 0; i < MAX_DECKS; ++i) {
        DeckState state;
        state.group = QString("[Channel%1]").arg(i + 1);
        m_deckStates.append(state);
    }
}

NowPlayingWorker::~NowPlayingWorker() {
    shutdown();
}

void NowPlayingWorker::initialize() {
    // Read initial settings
    m_nowPlayingEnabled = m_pConfig->getValue(
            kConfigKeyNowPlayingEnabled, kDefaultNowPlayingEnabled);
    m_pollIntervalMs = m_pConfig->getValue(kConfigKeyNowPlayingPollInterval, kDefaultPollInterval);
    m_appendMode = m_pConfig->getValue(kConfigKeyNowPlayingAppendMode, kDefaultAppendMode);
    m_addTimestamp = m_pConfig->getValue(kConfigKeyNowPlayingAddTimestamp, kDefaultAddTimestamp);
    m_archive = m_pConfig->getValue(kConfigKeyNowPlayingArchive, kDefaultArchive);

    if (debugNowPlaing) {
        qDebug() << "[NowPlayingWorker] Initialized with enabled:" << m_nowPlayingEnabled
                 << "appendMode:" << m_appendMode << "pollInterval:" << m_pollIntervalMs;
    }

    // Check if NowPlaying.txt exists from previous session (crash or normal)
    QFile existingFile(m_nowPlayingFilePath);
    if (existingFile.exists() && existingFile.size() > 0) {
        if (debugNowPlaing) {
            qDebug() << "[NowPlayingWorker] Found existing NowPlaying.txt with content";
        }

        const QFileInfo fileInfo(existingFile);
        const QString timestamp = fileInfo.lastModified().toString("yyyyMMdd_HHmmss");
        const QString expectedArchiveName = QString("NowPlaying_%1.txt").arg(timestamp);
        const QString archivePath = m_archiveFolderPath + "/" + expectedArchiveName;

        if (QFile::exists(archivePath)) {
            qDebug() << "[NowPlayingWorker] Archive already exists:" << expectedArchiveName;
            if (existingFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                existingFile.resize(0);
                existingFile.close();
                if (debugNowPlaing) {
                    qDebug() << "[NowPlayingWorker] Cleared existing "
                                "NowPlaying.txt (already archived)";
                }
            }
        } else {
            if (debugNowPlaing) {
                qDebug() << "[NowPlayingWorker] Archive not found, archiving now";
            }
            ensureArchiveFolderExists();
            if (existingFile.copy(archivePath)) {
                qDebug() << "[NowPlayingWorker] Archived previous session to:" << archivePath;
                if (existingFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    existingFile.resize(0);
                    existingFile.close();
                    qDebug() << "[NowPlayingWorker] Cleared NowPlaying.txt after archive";
                }
            } else {
                if (debugNowPlaing) {
                    qDebug() << "[NowPlayingWorker] Failed to archive previous session";
                }
            }
        }
    }

    if (m_nowPlayingEnabled) {
        writeSessionHeader();
        m_sessionArchived = false;
        if (debugNowPlaing) {
            qDebug() << "[NowPlayingWorker] Started new session";
        }
    }

    connect(&PlayerInfo::instance(),
            &PlayerInfo::trackChanged,
            this,
            &NowPlayingWorker::slotTrackChanged,
            Qt::DirectConnection);

    for (int i = 0; i < MAX_DECKS; ++i) {
        const QString& group = m_deckStates[i].group;

        if (ControlObject::exists(ConfigKey(group, "play"))) {
            ControlProxy* proxy = new ControlProxy(group, "play");
            m_deckPlayProxies.append(proxy);
        }
        if (ControlObject::exists(ConfigKey(group, "pregain"))) {
            ControlProxy* proxy = new ControlProxy(group, "pregain");
            m_deckPregainProxies.append(proxy);
        }
        if (ControlObject::exists(ConfigKey(group, "volume"))) {
            ControlProxy* proxy = new ControlProxy(group, "volume");
            m_deckVolumeProxies.append(proxy);
        }
        if (ControlObject::exists(ConfigKey(group, "orientation"))) {
            ControlProxy* proxy = new ControlProxy(group, "orientation");
            m_deckCrossfaderOrientationProxies.append(proxy);
        }
    }

    if (ControlObject::exists(ConfigKey("[Master]", "crossfader"))) {
        m_pMasterCrossfaderProxy = new ControlProxy("[Master]", "crossfader");
    }

    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(m_pollIntervalMs);
    connect(m_pollTimer, &QTimer::timeout, this, &NowPlayingWorker::slotPollNowPlaying);
    m_pollTimer->start();

    slotPollNowPlaying();
}

void NowPlayingWorker::shutdown() {
    if (m_pollTimer) {
        m_pollTimer->stop();
        delete m_pollTimer;
        m_pollTimer = nullptr;
    }

    if (m_nowPlayingEnabled && m_appendMode && m_archive && !m_sessionArchived) {
        QFile currentFile(m_nowPlayingFilePath);
        if (currentFile.exists() && currentFile.size() > 0) {
            const QFileInfo fileInfo(currentFile);
            const QString timestamp = fileInfo.lastModified().toString("yyyyMMdd_HHmmss");
            const QString archivePath = m_archiveFolderPath + "/NowPlaying_" + timestamp + ".txt";

            if (!QFile::exists(archivePath)) {
                ensureArchiveFolderExists();
                if (currentFile.copy(archivePath)) {
                    if (debugNowPlaing) {
                        qDebug() << "[NowPlayingWorker] Archived session to:" << archivePath;
                    }
                    m_sessionArchived = true;
                }
            }
        }
    } else if (m_nowPlayingEnabled && m_appendMode && !m_archive) {
        QFile file(m_nowPlayingFilePath);
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << "# Session ended: "
                   << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
            stream << "# " << QString(50, '-') << "\n\n";
            file.close();
        }
    }

    qDeleteAll(m_deckPlayProxies);
    qDeleteAll(m_deckPregainProxies);
    qDeleteAll(m_deckVolumeProxies);
    qDeleteAll(m_deckCrossfaderOrientationProxies);
    delete m_pMasterCrossfaderProxy;

    m_deckPlayProxies.clear();
    m_deckPregainProxies.clear();
    m_deckVolumeProxies.clear();
    m_deckCrossfaderOrientationProxies.clear();
    m_pMasterCrossfaderProxy = nullptr;
}

void NowPlayingWorker::DeckState::calculateEffectiveVolume(double crossfaderValue) {
    double baseVolume = volume * pregain;
    const double EPSILON = 0.001;

    if (!isPlaying || !currentTrack || baseVolume < EPSILON) {
        effectiveVolume = 0.0;
        return;
    }

    const double FULL_LEFT = -1.0;
    const double FULL_RIGHT = 1.0;

    if (crossfaderValue < -0.95) {
        if (crossfaderOrientation == 0) {
            effectiveVolume = baseVolume;
        } else if (crossfaderOrientation == 1) {
            effectiveVolume = baseVolume * 0.7;
        } else {
            effectiveVolume = 0.0;
        }
    } else if (crossfaderValue > 0.95) {
        if (crossfaderOrientation == 2) {
            effectiveVolume = baseVolume;
        } else if (crossfaderOrientation == 1) {
            effectiveVolume = baseVolume * 0.7;
        } else {
            effectiveVolume = 0.0;
        }
    } else {
        double blendFactor = (crossfaderValue - FULL_LEFT) / (FULL_RIGHT - FULL_LEFT);
        blendFactor = std::max(0.0, std::min(1.0, blendFactor));

        if (crossfaderOrientation == 0) {
            effectiveVolume = baseVolume * (1.0 - blendFactor);
        } else if (crossfaderOrientation == 2) {
            effectiveVolume = baseVolume * blendFactor;
        } else {
            effectiveVolume = baseVolume;
        }
    }
}

void NowPlayingWorker::ensureArchiveFolderExists() {
    QDir dir(m_archiveFolderPath);
    if (!dir.exists() && !dir.mkpath(".")) {
        if (debugNowPlaing) {
            qDebug() << "[NowPlaying] Failed to create archive folder:" << m_archiveFolderPath;
        }
    } else {
        if (debugNowPlaing) {
            qDebug() << "[NowPlaying] Failed to create archive folder:" << m_archiveFolderPath;
        }
    }
}

void NowPlayingWorker::archiveCurrentSession() {
    if (!m_nowPlayingEnabled || !m_appendMode || !m_archive || m_sessionArchived) {
        return;
    }

    QFile currentFile(m_nowPlayingFilePath);
    if (!currentFile.exists()) {
        if (debugNowPlaing) {
            qDebug() << "[NowPlayingWorker] No file to archive";
        }
        return;
    }

    if (currentFile.size() == 0) {
        if (debugNowPlaing) {
            qDebug() << "[NowPlayingWorker] File is empty, nothing to archive";
        }
        return;
    }

    ensureArchiveFolderExists();

    const QFileInfo fileInfo(currentFile);
    const QString timestamp = fileInfo.lastModified().toString("yyyyMMdd_HHmmss");
    const QString archivePath = m_archiveFolderPath + "/NowPlaying_" + timestamp + ".txt";

    // Check if this session was already archived
    // = checking if archived file with in names filetimpestamp of original
    if (QFile::exists(archivePath)) {
        if (debugNowPlaing) {
            qDebug() << "[NowPlayingWorker] Session already archived:" << archivePath;
        }
        m_sessionArchived = true;
        return;
    }

    // Check for any file with same timestamp (in case of rename)
    QDir archiveDir(m_archiveFolderPath);
    QStringList existingFiles = archiveDir.entryList(QDir::Files);
    for (const QString& existingFile : std::as_const(existingFiles)) {
        if (existingFile.contains(timestamp)) {
            if (debugNowPlaing) {
                qDebug() << "[NowPlayingWorker] Session already archived as:" << existingFile;
            }
            m_sessionArchived = true;
            return;
        }
    }

    // No archive found -> create new one
    if (currentFile.copy(archivePath)) {
        QFile archivedFile(archivePath);
        archivedFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner |
                QFile::ReadGroup | QFile::ReadOther);
        if (debugNowPlaing) {
            qDebug() << "[NowPlayingWorker] Archived session to:" << archivePath;
        }
        m_sessionArchived = true;
    } else {
        if (debugNowPlaing) {
            qDebug() << "[NowPlayingWorker] Failed to archive session";
        }
    }
}

int NowPlayingWorker::getDeckIndexFromGroup(const QString& group) const {
    for (int i = 0; i < MAX_DECKS; ++i) {
        if (m_deckStates[i].group == group) {
            return i;
        }
    }
    return -1;
}

void NowPlayingWorker::slotTrackChanged(
        const QString& group, TrackPointer pTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pOldTrack);
    const int deckIndex = getDeckIndexFromGroup(group);
    if (deckIndex >= 0) {
        m_deckStates[deckIndex].currentTrack = std::move(pTrack);
    }
}

void NowPlayingWorker::slotPollNowPlaying() {
    if (debugNowPlaing) {
        qDebug() << "[NowPlayingWorker] Polling...";
    }

    const bool newEnabled = m_pConfig->getValue(
            kConfigKeyNowPlayingEnabled, kDefaultNowPlayingEnabled);
    const int newPollInterval = m_pConfig->getValue(
            kConfigKeyNowPlayingPollInterval, kDefaultPollInterval);
    const bool newAppendMode = m_pConfig->getValue(
            kConfigKeyNowPlayingAppendMode, kDefaultAppendMode);
    const bool newAddTimestamp = m_pConfig->getValue(
            kConfigKeyNowPlayingAddTimestamp, kDefaultAddTimestamp);
    const bool newArchive = m_pConfig->getValue(kConfigKeyNowPlayingArchive, kDefaultArchive);

    if (newEnabled != m_nowPlayingEnabled) {
        m_nowPlayingEnabled = newEnabled;
        if (m_nowPlayingEnabled) {
            if (debugNowPlaing) {
                qDebug() << "[NowPlaying] File writing enabled - writing session header";
            }
            writeSessionHeader();
        } else {
            if (debugNowPlaing) {
                qDebug() << "[NowPlaying] File writing disabled - archiving and clearing";
            }
            if (m_appendMode && m_archive) {
                archiveCurrentSession();
            }
            QFile file(m_nowPlayingFilePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                file.resize(0);
                file.close();
                file.setPermissions(QFile::ReadOwner | QFile::WriteOwner |
                        QFile::ReadGroup | QFile::ReadOther);
            }
        }
    }

    if (newAppendMode != m_appendMode) {
        m_appendMode = newAppendMode;
        if (debugNowPlaing) {
            qDebug() << "[NowPlaying] AppendMode changed to:" << m_appendMode;
        }
        if (m_nowPlayingEnabled) {
            writeSessionHeader();
        }
    } else {
        m_appendMode = newAppendMode;
    }

    m_archive = newArchive;

    m_addTimestamp = newAddTimestamp;

    if (newPollInterval != m_pollIntervalMs && newPollInterval > 0) {
        m_pollIntervalMs = newPollInterval;
        if (m_pollTimer) {
            m_pollTimer->setInterval(m_pollIntervalMs);
        }
        qDebug() << "[NowPlaying] Poll interval changed to:" << m_pollIntervalMs;
    }

    const double crossfaderValue = m_pMasterCrossfaderProxy ? m_pMasterCrossfaderProxy->get() : 0.0;

    QList<DeckState> audibleDecks;
    for (int i = 0; i < MAX_DECKS; ++i) {
        if (i < m_deckPlayProxies.size() && m_deckPlayProxies[i]) {
            m_deckStates[i].isPlaying = m_deckPlayProxies[i]->get() > 0.0;
        }
        if (i < m_deckPregainProxies.size() && m_deckPregainProxies[i]) {
            m_deckStates[i].pregain = m_deckPregainProxies[i]->get();
        }
        if (i < m_deckVolumeProxies.size() && m_deckVolumeProxies[i]) {
            m_deckStates[i].volume = m_deckVolumeProxies[i]->get();
        }
        if (i < m_deckCrossfaderOrientationProxies.size() &&
                m_deckCrossfaderOrientationProxies[i]) {
            m_deckStates[i].crossfaderOrientation = static_cast<int>(
                    m_deckCrossfaderOrientationProxies[i]->get());
        }

        m_deckStates[i].calculateEffectiveVolume(crossfaderValue);

        if (m_deckStates[i].isPlaying &&
                m_deckStates[i].effectiveVolume > VOLUME_THRESHOLD &&
                m_deckStates[i].currentTrack) {
            audibleDecks.append(m_deckStates[i]);
        }
    }

    // Sort by volume (loudest first)
    std::sort(audibleDecks.begin(), audibleDecks.end(), [](const DeckState& a, const DeckState& b) {
        return a.effectiveVolume > b.effectiveVolume;
    });

    if (audibleDecks.isEmpty()) {
        emit updateWindowTitle(VersionStore::applicationName(), QString());
        if (m_nowPlayingEnabled) {
            writeNowPlayingFile("");
        }
        return;
    }

    QStringList trackStrings;
    trackStrings.reserve(audibleDecks.size());
    for (const auto& deck : std::as_const(audibleDecks)) {
        const QString trackInfo = deck.currentTrack->getInfo();
        if (!trackInfo.isEmpty()) {
            trackStrings.append(trackInfo);
        }
    }

    const QString combinedTitle = trackStrings.join(" | ");
    const QString appTitle = VersionStore::applicationName();
    const QString windowTitle = QString("%1 | %2").arg(appTitle, combinedTitle);
    const QString filePath = audibleDecks[0].currentTrack->getLocation();

    emit updateWindowTitle(windowTitle, filePath);

    if (m_nowPlayingEnabled) {
        writeNowPlayingFile(trackStrings.join(" | "));
    }
}

void NowPlayingWorker::writeNowPlayingFile(const QString& content) {
    static QString lastContent;

    if (!m_nowPlayingEnabled) {
        return;
    }

    if (content == lastContent) {
        return;
    }

    QFile file(m_nowPlayingFilePath);
    const QIODevice::OpenMode mode =
            (m_appendMode ? QIODevice::Append : QIODevice::WriteOnly) |
            QIODevice::Text;

    if (file.open(mode)) {
        QTextStream stream(&file);

        if (m_appendMode && m_addTimestamp) {
            const QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            if (content.isEmpty()) {
                stream << timestamp << " | [NO TRACKS PLAYING]\n";
            } else {
                stream << timestamp << " | " << content << "\n";
            }
        } else if (m_appendMode && !m_addTimestamp) {
            if (content.isEmpty()) {
                stream << "[NO TRACKS PLAYING]\n";
            } else {
                stream << content << "\n";
            }
        } else if (!content.isEmpty()) {
            stream << content << "\n";
        }

        file.close();
        lastContent = content;
        if (debugNowPlaing) {
            qDebug() << "[NowPlaying] File written:" << content;
        }
    }
}

void NowPlayingWorker::writeSessionHeader() {
    QFile file(m_nowPlayingFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "# Mixxx Session started: "
               << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
        stream << "# " << QString(50, '-') << "\n";
        file.close();
        file.setPermissions(QFile::ReadOwner | QFile::WriteOwner |
                QFile::ReadGroup | QFile::ReadOther);
        if (debugNowPlaing) {
            qDebug() << "[NowPlaying] Session header written";
        }
    }
}
