#include "stems/stemgenerator.h"

#include <QAbstractItemModel>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcessEnvironment>
#include <QUrl>

#include "control/controlobject.h"
#include "coreservices.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playermanager.h"
#include "track/track.h"
#include "track/trackref.h"
#include "util/cmdlineargs.h"
#include "util/logger.h"

#if defined(MIXXX_USE_QML)
#include <QJSEngine>
#include <QQmlEngine>
#endif

namespace {

const mixxx::Logger kLogger("StemGenerator");

const QString kStemFileSuffix = QStringLiteral(".stem.mp4");
const QString kSettingsGroup = QStringLiteral("[Stems]");

// Values measured on the development machine with the default model and
// precision, over a 243 s track: 22.4 s on an RTX 3060 through CUDA and 102.4 s
// on an i5-12400F. Only used to estimate how long a job will take.
constexpr double kFactorGpu = 10.9;
constexpr double kFactorCpu = 2.4;

// How often the queue is retried while a deck is playing.
constexpr int kRetryDelayMs = 5000;

QString findPythonIn(const QString& directory) {
    const QDir dir(directory);
    if (!dir.exists()) {
        return QString();
    }
    const QStringList candidates = {
            QStringLiteral("bin/python3"),
            QStringLiteral("bin/python"),
            QStringLiteral("Scripts/python.exe"),
    };
    for (const QString& candidate : candidates) {
        const QString path = dir.absoluteFilePath(candidate);
        if (QFileInfo::exists(path)) {
            return path;
        }
    }
    return QString();
}

} // namespace

StemGenerator::StemGenerator(UserSettingsPointer pConfig, QObject* pParent)
        : QObject(pParent),
          m_pConfig(std::move(pConfig)) {
    m_startTimer.setSingleShot(true);
    connect(&m_startTimer, &QTimer::timeout, this, &StemGenerator::slotStartNextJob);

    connect(&m_worker, &QProcess::readyReadStandardOutput, this, &StemGenerator::slotWorkerOutput);
    connect(&m_worker,
            &QProcess::readyReadStandardError,
            this,
            &StemGenerator::slotWorkerOutput);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(&m_worker,
            &QProcess::finished,
            this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                slotWorkerFinished(exitCode, exitStatus);
            });
#else
    connect(&m_worker,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &StemGenerator::slotWorkerFinished);
#endif
    connect(&m_queue, &StemQueueModel::countsChanged, this, [this] {
        emit queueChanged();
    });
}

StemGenerator::~StemGenerator() {
    if (m_worker.state() != QProcess::NotRunning) {
        m_worker.kill();
        m_worker.waitForFinished(3000);
    }
}

void StemGenerator::registerDependencies(mixxx::CoreServices* pCoreServices) {
    m_pCoreServices = pCoreServices;
}

void StemGenerator::registerInstance(StemGenerator* pInstance) {
    s_pInstance = pInstance;
}

#if defined(MIXXX_USE_QML)
StemGenerator* StemGenerator::create(QQmlEngine* pQmlEngine, QJSEngine* pJsEngine) {
    Q_UNUSED(pQmlEngine);
    Q_UNUSED(pJsEngine);
    if (!s_pInstance) {
        return nullptr;
    }
    // The instance is owned by CoreServices, QML must not delete it.
    QQmlEngine::setObjectOwnership(s_pInstance, QQmlEngine::CppOwnership);
    return s_pInstance;
}
#endif

// ---------------------------------------------------------------- settings --

bool StemGenerator::enabled() const {
    return m_pConfig->getValue(ConfigKey(kSettingsGroup, QStringLiteral("Enabled")), true);
}

void StemGenerator::setEnabled(bool enabled) {
    if (this->enabled() == enabled) {
        return;
    }
    m_pConfig->setValue(ConfigKey(kSettingsGroup, QStringLiteral("Enabled")), enabled);
    if (enabled) {
        startNextJob();
    }
    emit enabledChanged();
}

QString StemGenerator::pythonPath() const {
    // Trimmed because hand edited or rewritten settings can carry a trailing
    // space, which would make the existence check fail.
    return m_pConfig->getValue(ConfigKey(kSettingsGroup, QStringLiteral("PythonPath")),
            QString())
            .trimmed();
}

void StemGenerator::setPythonPath(const QString& path) {
    if (pythonPath() == path) {
        return;
    }
    m_pConfig->setValue(ConfigKey(kSettingsGroup, QStringLiteral("PythonPath")), path);
    emit pythonPathChanged();
}

QString StemGenerator::ffmpegPath() const {
    return m_pConfig->getValue(ConfigKey(kSettingsGroup, QStringLiteral("FfmpegPath")),
            QStringLiteral("ffmpeg"))
            .trimmed();
}

void StemGenerator::setFfmpegPath(const QString& path) {
    if (ffmpegPath() == path) {
        return;
    }
    m_pConfig->setValue(ConfigKey(kSettingsGroup, QStringLiteral("FfmpegPath")), path);
    emit ffmpegPathChanged();
}

QString StemGenerator::cacheDirectory() const {
    const QString configured = m_pConfig->getValue(
            ConfigKey(kSettingsGroup, QStringLiteral("CacheDirectory")), QString())
                                       .trimmed();
    if (!configured.isEmpty()) {
        return configured;
    }
    return QDir(m_pConfig->getSettingsPath()).absoluteFilePath(QStringLiteral("stems"));
}

void StemGenerator::setCacheDirectory(const QString& path) {
    if (cacheDirectory() == path) {
        return;
    }
    m_pConfig->setValue(ConfigKey(kSettingsGroup, QStringLiteral("CacheDirectory")), path);
    emit cacheDirectoryChanged();
}

bool StemGenerator::writeNextToTrack() const {
    return m_pConfig->getValue(
            ConfigKey(kSettingsGroup, QStringLiteral("WriteNextToTrack")), true);
}

void StemGenerator::setWriteNextToTrack(bool writeNextToTrack) {
    if (this->writeNextToTrack() == writeNextToTrack) {
        return;
    }
    m_pConfig->setValue(ConfigKey(kSettingsGroup, QStringLiteral("WriteNextToTrack")),
            writeNextToTrack);
    emit writeNextToTrackChanged();
}

QString StemGenerator::codec() const {
    return m_pConfig->getValue(ConfigKey(kSettingsGroup, QStringLiteral("Codec")),
            QStringLiteral("aac"))
            .trimmed();
}

void StemGenerator::setCodec(const QString& codec) {
    if (this->codec() == codec) {
        return;
    }
    m_pConfig->setValue(ConfigKey(kSettingsGroup, QStringLiteral("Codec")), codec);
    emit codecChanged();
}

QString StemGenerator::model() const {
    return m_pConfig->getValue(ConfigKey(kSettingsGroup, QStringLiteral("Model")),
            QStringLiteral("htdemucs"))
            .trimmed();
}

void StemGenerator::setModel(const QString& model) {
    if (this->model() == model) {
        return;
    }
    m_pConfig->setValue(ConfigKey(kSettingsGroup, QStringLiteral("Model")), model);
    emit modelChanged();
}

QString StemGenerator::provider() const {
    return m_pConfig->getValue(ConfigKey(kSettingsGroup, QStringLiteral("Provider")),
            QStringLiteral("auto"))
            .trimmed();
}

void StemGenerator::setProvider(const QString& provider) {
    if (this->provider() == provider) {
        return;
    }
    m_pConfig->setValue(ConfigKey(kSettingsGroup, QStringLiteral("Provider")), provider);
    emit providerChanged();
}

bool StemGenerator::pauseDuringPlayback() const {
    return m_pConfig->getValue(
            ConfigKey(kSettingsGroup, QStringLiteral("PauseDuringPlayback")), true);
}

void StemGenerator::setPauseDuringPlayback(bool pause) {
    if (pauseDuringPlayback() == pause) {
        return;
    }
    m_pConfig->setValue(ConfigKey(kSettingsGroup, QStringLiteral("PauseDuringPlayback")), pause);
    emit pauseDuringPlaybackChanged();
}

// ------------------------------------------------------------ capabilities --

double StemGenerator::estimatedFactor() const {
    return m_gpuAvailable ? kFactorGpu : kFactorCpu;
}

bool StemGenerator::busy() const {
    return m_worker.state() != QProcess::NotRunning;
}

int StemGenerator::pendingCount() const {
    return m_queue.pendingCount();
}

QString StemGenerator::queueStatus() const {
    if (!m_queueStatus.isEmpty()) {
        return m_queueStatus;
    }
    const int pending = m_queue.pendingCount();
    if (pending == 0) {
        return QString();
    }
    return QStringLiteral("%1 en cola").arg(pending);
}

QString StemGenerator::resolvedPython() const {
    const QString configured = pythonPath();
    if (!configured.isEmpty() && QFileInfo::exists(configured)) {
        return configured;
    }
    // A virtual environment managed by the user next to the stem cache is the
    // most likely place to find the model runtime.
    const QString managed = QDir(cacheDirectory()).absoluteFilePath(QStringLiteral("venv"));
    const QString found = findPythonIn(managed);
    if (!found.isEmpty()) {
        return found;
    }
    if (!configured.isEmpty()) {
        return configured;
    }
    return QStringLiteral("python");
}

QString StemGenerator::resolvedWorkerScript() const {
    const QDir resourceDir(CmdlineArgs::Instance().getResourcePath());
    const QStringList candidates = {
            resourceDir.absoluteFilePath(QStringLiteral("stems/stems_worker.py")),
            resourceDir.absoluteFilePath(QStringLiteral("../res/stems/stems_worker.py")),
    };
    for (const QString& candidate : candidates) {
        if (QFileInfo::exists(candidate)) {
            return QFileInfo(candidate).absoluteFilePath();
        }
    }
    return candidates.constFirst();
}

/// Directories of the CUDA runtime shipped as Python wheels. Mixxx spawns the
/// worker with a plain environment, so the loader needs them on PATH; they live
/// under ``site-packages/nvidia`` and not next to the interpreter.
QStringList StemGenerator::workerEnvironmentValue() const {
    const QFileInfo pythonInfo(resolvedPython());
    const QDir venvRoot(pythonInfo.absolutePath() + QStringLiteral("/.."));
    QStringList sitePackages;
    for (const QString& relative : {QStringLiteral("Lib/site-packages"),
                 QStringLiteral("lib/site-packages")}) {
        const QString path = venvRoot.absoluteFilePath(relative);
        if (QDir(path).exists()) {
            sitePackages << path;
        }
    }
    // Linux layouts add a python version level: lib/python3.11/site-packages
    const QDir libDir(venvRoot.absoluteFilePath(QStringLiteral("lib")));
    if (libDir.exists()) {
        for (const QString& entry : libDir.entryList(
                     {QStringLiteral("python*")}, QDir::Dirs | QDir::NoDotAndDotDot)) {
            const QString path = libDir.absoluteFilePath(
                    entry + QStringLiteral("/site-packages"));
            if (QDir(path).exists()) {
                sitePackages << path;
            }
        }
    }

    QStringList extraPaths;
    for (const QString& sitePackage : sitePackages) {
        const QDir nvidiaDir(sitePackage + QStringLiteral("/nvidia"));
        if (!nvidiaDir.exists()) {
            continue;
        }
        const QStringList packages = nvidiaDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString& package : packages) {
            for (const QString& bin : {QStringLiteral("bin"), QStringLiteral("bin/x86_64")}) {
                const QString path = nvidiaDir.absoluteFilePath(package + QLatin1Char('/') + bin);
                if (QDir(path).exists()) {
                    extraPaths << path;
                }
            }
        }
    }

    QStringList values = extraPaths;
    const QString currentPath = QProcessEnvironment::systemEnvironment()
                                        .value(QStringLiteral("PATH"));
    if (!currentPath.isEmpty()) {
        values << currentPath;
    }
    return values;
}

// ------------------------------------------------------------------ helpers --

QString StemGenerator::displayTitleForTrack(const QString& trackPath) {
    const QFileInfo info(trackPath);
    QString title = info.completeBaseName();
    // Strip the ".stem" of files that are already stem containers.
    if (title.endsWith(QStringLiteral(".stem"))) {
        title.chop(5);
    }
    return title;
}

QString StemGenerator::cachePathForSource(const QString& sourcePath) const {
    const QFileInfo info(sourcePath);
    const QByteArray digest = QCryptographicHash::hash(
            info.absoluteFilePath().toUtf8(), QCryptographicHash::Sha1);
    const QString key = QString::fromLatin1(digest.toHex().left(8));
    QString baseName = info.completeBaseName();
    if (baseName.endsWith(QStringLiteral(".stem"))) {
        baseName.chop(5);
    }
    return QDir(cacheDirectory()).absoluteFilePath(key + QLatin1Char('/') + baseName + kStemFileSuffix);
}

QString StemGenerator::outputPathForSource(const QString& sourcePath) const {
    if (!writeNextToTrack()) {
        return cachePathForSource(sourcePath);
    }
    const QFileInfo info(sourcePath);
    return info.absolutePath() + QLatin1Char('/') + info.completeBaseName() + kStemFileSuffix;
}

QString StemGenerator::stemFileFor(const QString& trackPath) const {
    const QFileInfo info(trackPath);
    if (info.fileName().endsWith(kStemFileSuffix)) {
        return info.absoluteFilePath();
    }
    const QString sibling = info.absolutePath() + QLatin1Char('/') +
            info.completeBaseName() + kStemFileSuffix;
    if (QFileInfo::exists(sibling)) {
        return sibling;
    }
    const QString cached = cachePathForSource(trackPath);
    if (QFileInfo::exists(cached)) {
        return cached;
    }
    return QString();
}

bool StemGenerator::hasStems(const QString& trackPath) const {
    return !stemFileFor(trackPath).isEmpty();
}

TrackPointer StemGenerator::stemTrackFor(const TrackPointer& pTrack) const {
    if (!pTrack || !m_pCoreServices) {
        return pTrack;
    }
    const QString location = pTrack->getLocation();
    if (location.isEmpty() || location.endsWith(kStemFileSuffix)) {
        return pTrack;
    }
    const QString stemFile = stemFileFor(location);
    if (stemFile.isEmpty()) {
        return pTrack;
    }
    const std::shared_ptr<TrackCollectionManager> pTrackCollectionManager =
            m_pCoreServices->getTrackCollectionManager();
    if (!pTrackCollectionManager) {
        return pTrack;
    }
    TrackPointer pStemTrack =
            pTrackCollectionManager->getOrAddTrack(TrackRef::fromFilePath(stemFile));
    if (!pStemTrack) {
        return pTrack;
    }
    kLogger.info() << "Playing the stems of" << location << "from" << stemFile;
    return pStemTrack;
}

bool StemGenerator::isAnyDeckPlaying() const {
    if (!m_pCoreServices) {
        return false;
    }
    const std::shared_ptr<PlayerManager> pPlayerManager = m_pCoreServices->getPlayerManager();
    if (!pPlayerManager) {
        return false;
    }
    const QStringList groups = pPlayerManager->getVisualPlayerGroups();
    for (const QString& group : groups) {
        if (ControlObject::get(ConfigKey(group, QStringLiteral("play"))) > 0.0) {
            return true;
        }
    }
    return false;
}

// -------------------------------------------------------------------- queue --

void StemGenerator::enqueueTracks(const QList<TrackId>& trackIds, bool regenerate) {
    if (!m_pCoreServices) {
        return;
    }
    const std::shared_ptr<TrackCollectionManager> pTrackCollectionManager =
            m_pCoreServices->getTrackCollectionManager();
    for (const TrackId& trackId : trackIds) {
        const TrackPointer pTrack = pTrackCollectionManager->getTrackById(trackId);
        if (!pTrack) {
            continue;
        }
        const QString location = pTrack->getLocation();
        if (location.isEmpty()) {
            continue;
        }
        if (location.endsWith(kStemFileSuffix)) {
            continue;
        }
        if (!regenerate && hasStems(location)) {
            continue;
        }
        if (m_queue.hasSource(location)) {
            continue;
        }
        StemQueueModel::Job job;
        job.sourcePath = location;
        job.outputPath = outputPathForSource(location);
        const QString title = pTrack->getTitle();
        const QString artist = pTrack->getArtist();
        if (!title.isEmpty() && !artist.isEmpty()) {
            job.title = QStringLiteral("%1 - %2").arg(artist, title);
        } else if (!title.isEmpty()) {
            job.title = title;
        } else {
            job.title = displayTitleForTrack(location);
        }
        m_queue.add(job);
    }
    startNextJob();
}

void StemGenerator::slotStartNextJob() {
    startNextJob();
}

void StemGenerator::startNextJob() {
    if (!m_pConfig) {
        return;
    }
    if (m_worker.state() != QProcess::NotRunning) {
        return;
    }
    if (!enabled()) {
        return;
    }
    const int row = m_queue.nextQueuedRow();
    if (row < 0) {
        if (m_waitingForPlayback) {
            m_waitingForPlayback = false;
            emit waitingForPlaybackChanged();
        }
        m_queueStatus.clear();
        emit queueChanged();
        return;
    }
    if (pauseDuringPlayback() && isAnyDeckPlaying()) {
        if (!m_waitingForPlayback) {
            m_waitingForPlayback = true;
            emit waitingForPlaybackChanged();
        }
        m_queueStatus = QStringLiteral("En espera: hay música sonando");
        emit queueChanged();
        m_startTimer.start(kRetryDelayMs);
        return;
    }
    if (m_waitingForPlayback) {
        m_waitingForPlayback = false;
        emit waitingForPlaybackChanged();
    }

    const QString script = resolvedWorkerScript();
    if (!QFileInfo::exists(script)) {
        failCurrentJob(QStringLiteral("No se encuentra el worker de stems (%1)").arg(script));
        // Try the next job later instead of recursing into a long queue.
        m_startTimer.start(0);
        return;
    }
    const QString python = resolvedPython();
    if (python != QStringLiteral("python") && !QFileInfo::exists(python)) {
        failCurrentJob(QStringLiteral("No se encuentra el intérprete de Python (%1)").arg(python));
        m_startTimer.start(0);
        return;
    }
    const StemQueueModel::Job job = m_queue.jobAt(row);
    m_currentRow = row;
    m_outputBuffer.clear();
    m_probing = false;
    m_queue.markRunning(row);

    m_worker.setProgram(python);
    m_worker.setArguments({script,
            QStringLiteral("separate"),
            QStringLiteral("--input"),
            job.sourcePath,
            QStringLiteral("--out"),
            job.outputPath,
            QStringLiteral("--provider"),
            provider(),
            QStringLiteral("--codec"),
            codec(),
            QStringLiteral("--model"),
            model(),
            QStringLiteral("--ffmpeg"),
            ffmpegPath()});
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    const QStringList paths = workerEnvironmentValue();
    if (!paths.isEmpty()) {
        environment.insert(QStringLiteral("PATH"), paths.join(QLatin1Char(';')));
    }
    // Keep the progress lines readable even on a non English Windows.
    environment.insert(QStringLiteral("PYTHONIOENCODING"), QStringLiteral("utf-8"));
    environment.insert(QStringLiteral("PYTHONUNBUFFERED"), QStringLiteral("1"));
    m_worker.setProcessEnvironment(environment);
    m_worker.setWorkingDirectory(QFileInfo(script).absolutePath());
    m_queueStatus = QStringLiteral("Generando \"%1\"").arg(job.title);
    kLogger.info() << "Generating stems for" << job.sourcePath << "->" << job.outputPath
                   << "with" << python << "provider" << provider();
    emit queueChanged();
    emit busyChanged();
    m_worker.start();
}

void StemGenerator::slotWorkerOutput() {
    const QByteArray chunk = m_worker.readAllStandardOutput() +
            m_worker.readAllStandardError();
    m_outputBuffer.append(QString::fromUtf8(chunk));
    int newline = m_outputBuffer.indexOf(QLatin1Char('\n'));
    while (newline >= 0) {
        const QString line = m_outputBuffer.left(newline).trimmed();
        m_outputBuffer.remove(0, newline + 1);
        if (!line.isEmpty()) {
            const int space = line.indexOf(QLatin1Char(' '));
            const QString kind = space < 0 ? line : line.left(space);
            const QString payload = space < 0 ? QString() : line.mid(space + 1);
            if (kind == QStringLiteral("PROGRESS")) {
                m_queue.setProgress(m_currentRow, payload.toInt());
            } else if (kind == QStringLiteral("STATUS")) {
                m_queueStatus = payload;
                emit queueChanged();
            } else if (kind == QStringLiteral("RESULT")) {
                const QJsonDocument document = QJsonDocument::fromJson(payload.toUtf8());
                if (document.isObject()) {
                    m_detectedProvider = document.object()
                                                 .value(QStringLiteral("provider"))
                                                 .toString(m_detectedProvider);
                }
            } else if (kind == QStringLiteral("DONE")) {
                // Success is confirmed when the process exits.
            } else if (kind == QStringLiteral("ERROR")) {
                m_queueStatus = payload;
                m_queue.markFailed(m_currentRow, payload);
                emit queueChanged();
            }
        }
        newline = m_outputBuffer.indexOf(QLatin1Char('\n'));
    }
}

void StemGenerator::slotWorkerFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    const QString rest = m_outputBuffer.trimmed();
    m_outputBuffer.clear();

    if (m_probing) {
        m_probing = false;
        m_ready = false;
        m_gpuAvailable = false;
        m_gpuName.clear();
        m_probeErrors.clear();
        m_detectedProvider.clear();
        const QJsonDocument document = QJsonDocument::fromJson(rest.toUtf8());
        if (!document.isObject()) {
            m_probeErrors << QStringLiteral("El intérprete no respondió con un informe válido: %1")
                                     .arg(rest.right(200).trimmed());
        } else {
            const QJsonObject report = document.object();
            m_gpuAvailable = report.value(QStringLiteral("has_cuda")).toBool();
            m_gpuName = report.value(QStringLiteral("gpu")).toString();
            m_detectedProvider = report.value(QStringLiteral("provider")).toString();
            m_ready = report.value(QStringLiteral("ok")).toBool();
            for (const QJsonValue& value : report.value(QStringLiteral("errors")).toArray()) {
                m_probeErrors << value.toString();
            }
        }
        if (exitCode != 0 && m_probeErrors.isEmpty()) {
            m_probeErrors << QStringLiteral("La comprobación terminó con código %1").arg(exitCode);
        }
        if (m_probeErrors.isEmpty() && m_ready) {
            kLogger.info() << "Stem environment ready, provider" << m_detectedProvider
                           << "GPU" << m_gpuName;
        } else {
            kLogger.warning() << "Stem environment is not ready:" << m_probeErrors
                              << "interpreter" << resolvedPython();
        }
        emit probeChanged();
        return;
    }

    const int row = m_currentRow;
    m_currentRow = -1;
    const StemQueueModel::Job job = m_queue.jobAt(row);
    bool succeeded = exitStatus == QProcess::NormalExit && exitCode == 0 &&
            !job.outputPath.isEmpty() && QFileInfo::exists(job.outputPath);
    if (succeeded) {
        m_queue.markFinished(row, m_detectedProvider);
        m_queueStatus = QStringLiteral("Listo: %1").arg(job.title);
        kLogger.info() << "Stem file written to" << job.outputPath
                       << "provider" << m_detectedProvider;
        if (m_pCoreServices && job.outputPath.endsWith(kStemFileSuffix) &&
                QFileInfo(job.outputPath).absolutePath() ==
                        QFileInfo(job.sourcePath).absolutePath()) {
            // The stem file sits next to the track, so the library has to see it.
            rescanLibrary();
        }
    } else {
        QString error = QStringLiteral("La generación falló (código %1)").arg(exitCode);
        const StemQueueModel::Job failed = m_queue.jobAt(row);
        if (!failed.error.isEmpty()) {
            error = failed.error;
        } else if (!rest.isEmpty()) {
            error = rest;
        }
        m_queue.markFailed(row, error);
        m_queueStatus = error;
        kLogger.warning() << "Stem generation failed for" << job.sourcePath << "exit code"
                          << exitCode << "exit status" << exitStatus << ":" << error;
    }
    emit queueChanged();
    emit busyChanged();
    startNextJob();
}

void StemGenerator::failCurrentJob(const QString& error) {
    if (m_currentRow < 0) {
        return;
    }
    m_queue.markFailed(m_currentRow, error);
    m_currentRow = -1;
    m_queueStatus = error;
    emit queueChanged();
}

// ------------------------------------------------------------------- actions --

void StemGenerator::probe() {
    kLogger.info() << "Stem settings: enabled" << enabled() << "interpreter" << resolvedPython()
                   << "ffmpeg" << ffmpegPath() << "cache" << cacheDirectory();
    if (m_worker.state() != QProcess::NotRunning) {
        return;
    }
    const QString script = resolvedWorkerScript();
    if (!QFileInfo::exists(script)) {
        m_ready = false;
        m_probeErrors = {QStringLiteral("No se encuentra %1").arg(script)};
        emit probeChanged();
        return;
    }
    m_probing = true;
    m_outputBuffer.clear();
    emit probeChanged();

    m_worker.setProgram(resolvedPython());
    m_worker.setArguments({script,
            QStringLiteral("probe"),
            QStringLiteral("--ffmpeg"),
            ffmpegPath()});
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    const QStringList paths = workerEnvironmentValue();
    if (!paths.isEmpty()) {
        environment.insert(QStringLiteral("PATH"), paths.join(QLatin1Char(';')));
    }
    environment.insert(QStringLiteral("PYTHONIOENCODING"), QStringLiteral("utf-8"));
    environment.insert(QStringLiteral("PYTHONUNBUFFERED"), QStringLiteral("1"));
    m_worker.setProcessEnvironment(environment);
    m_worker.setWorkingDirectory(QFileInfo(script).absolutePath());
    m_worker.start();
}

void StemGenerator::cancel() {
    if (m_worker.state() != QProcess::NotRunning) {
        m_worker.kill();
        m_worker.waitForFinished(3000);
    }
    m_currentRow = -1;
    m_queue.clearAll();
    m_queueStatus.clear();
    m_startTimer.stop();
    if (m_waitingForPlayback) {
        m_waitingForPlayback = false;
        emit waitingForPlaybackChanged();
    }
    emit queueChanged();
    emit busyChanged();
}

void StemGenerator::clearFinished() {
    m_queue.clearFinished();
    emit queueChanged();
}

void StemGenerator::openCacheFolder() const {
    const QString path = cacheDirectory();
    QDir().mkpath(path);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void StemGenerator::openTrackFolder(const QString& trackPath) const {
    const QFileInfo info(trackPath);
    if (!info.absolutePath().isEmpty()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(info.absolutePath()));
    }
}

void StemGenerator::rescanLibrary() {
    if (!m_pCoreServices) {
        return;
    }
    const std::shared_ptr<TrackCollectionManager> pTrackCollectionManager =
            m_pCoreServices->getTrackCollectionManager();
    if (pTrackCollectionManager) {
        pTrackCollectionManager->startLibraryScan();
    }
}

#include "moc_stemgenerator.cpp"
