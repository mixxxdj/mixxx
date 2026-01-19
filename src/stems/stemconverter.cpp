#include "stems/stemconverter.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include <QRegularExpression>

#include "track/track.h"
#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("StemConverter");
}

StemConverter::StemConverter(QObject* parent)
        : QObject(parent),
          m_state(ConversionState::Idle),
          m_progress(0.0f),
          m_statusMessage(QString()),
          m_trackTitle(QString()) {
}

QString StemConverter::findStemgenPath() {
    // Try to find stemgen in this order:
    // 1. User's pipx installation (~/.local/bin/stemgen)
    // 2. System-wide pipx installation (/usr/local/bin/stemgen)
    // 3. Direct python3 -m stemgen

    // Check pipx user installation
    QString userPipxPath = QDir::homePath() + "/.local/bin/stemgen";
    if (QFileInfo(userPipxPath).exists() && QFileInfo(userPipxPath).isExecutable()) {
        kLogger.info() << "Found stemgen at:" << userPipxPath;
        return userPipxPath;
    }

    // Check system pipx installation
    QString systemPipxPath = "/usr/local/bin/stemgen";
    if (QFileInfo(systemPipxPath).exists() && QFileInfo(systemPipxPath).isExecutable()) {
        kLogger.info() << "Found stemgen at:" << systemPipxPath;
        return systemPipxPath;
    }

    // Check if python3 -m stemgen works
    QProcess checkProcess;
    checkProcess.start("python3", QStringList() << "-m" << "stemgen" << "--version");
    if (checkProcess.waitForFinished(5000) && checkProcess.exitCode() == 0) {
        kLogger.info() << "Found stemgen via python3 -m stemgen";
        return "python3";
    }

    kLogger.warning() << "Stemgen not found in any location";
    return QString();
}

bool StemConverter::convertTrack(const TrackPointer& pTrack) {
    if (!pTrack) {
        qWarning() << "StemConverter: Track pointer is null";
        return false;
    }

    // Find stemgen installation
    QString stemgenPath = findStemgenPath();
    if (stemgenPath.isEmpty()) {
        QString errorMsg = "Stemgen is not installed. Please install it with: pipx install stemgen";
        kLogger.warning() << errorMsg;
        setProgress(0.0f, errorMsg);
        emit conversionFailed(pTrack->getId(), errorMsg);
        return false;
    }

    m_currentTrackId = pTrack->getId();
    m_trackTitle = pTrack->getTitle();
    setState(ConversionState::Processing);
    setProgress(0.0f, "Starting stem conversion...");

    emit conversionStarted(m_currentTrackId, m_trackTitle);

    // Get track location
    QString trackLocation = pTrack->getLocation();
    QFileInfo fileInfo(trackLocation);

    if (!fileInfo.exists()) {
        QString errorMsg = QString("Track file not found: %1").arg(trackLocation);
        kLogger.warning() << errorMsg;
        setProgress(0.0f, errorMsg);
        setState(ConversionState::Failed);
        emit conversionFailed(m_currentTrackId, errorMsg);
        return false;
    }

    // Get output directory (same as track)
    QString outputDir = fileInfo.absolutePath();
    QString baseName = fileInfo.baseName();

    setProgress(0.10f, "Preparing audio file...");

    // Create process
    m_pProcess = std::make_unique<QProcess>();

    // Connect signals
    // Note: In Qt 6, use QProcess::finished and QProcess::errorOccurred instead of error
    connect(m_pProcess.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &StemConverter::onProcessFinished);
    connect(m_pProcess.get(), QOverload<QProcess::ProcessError>::of(&QProcess::errorOccurred),
            this, &StemConverter::onProcessError);
    connect(m_pProcess.get(), &QProcess::readyReadStandardOutput,
            this, &StemConverter::onProcessOutput);
    connect(m_pProcess.get(), &QProcess::readyReadStandardError,
            this, &StemConverter::onProcessOutput);

    setProgress(0.20f, "Starting stem separation...");

    // Build and execute stemgen command
    QStringList arguments;

    if (stemgenPath == "python3") {
        // Using python3 -m stemgen
        arguments << "-m" << "stemgen"
                  << "-i" << trackLocation
                  << "-o" << outputDir
                  << "-f" << "alac"
                  << "-d" << "cpu";
        m_pProcess->start("python3", arguments);
    } else {
        // Using pipx installed stemgen directly
        arguments << "-i" << trackLocation
                  << "-o" << outputDir
                  << "-f" << "alac"
                  << "-d" << "cpu";
        m_pProcess->start(stemgenPath, arguments);
    }

    if (!m_pProcess->waitForStarted()) {
        QString errorMsg = "Failed to start stemgen process";
        kLogger.warning() << errorMsg;
        setProgress(0.0f, errorMsg);
        setState(ConversionState::Failed);
        emit conversionFailed(m_currentTrackId, errorMsg);
        return false;
    }

    setProgress(0.30f, "Processing audio (this may take several minutes)...");

    // Wait for process to finish (with timeout of 1 hour)
    if (!m_pProcess->waitForFinished(3600000)) {
        QString errorMsg = "Stemgen process timeout";
        kLogger.warning() << errorMsg;
        m_pProcess->kill();
        setProgress(0.0f, errorMsg);
        setState(ConversionState::Failed);
        emit conversionFailed(m_currentTrackId, errorMsg);
        return false;
    }

    // Check exit code
    if (m_pProcess->exitCode() != 0) {
        QString errorOutput = QString::fromUtf8(m_pProcess->readAllStandardError());
        QString errorMsg = QString("Stemgen process failed: %1").arg(errorOutput);
        kLogger.warning() << errorMsg;
        setProgress(0.0f, errorMsg);
        setState(ConversionState::Failed);
        emit conversionFailed(m_currentTrackId, errorMsg);
        return false;
    }

    setProgress(0.90f, "Finalizing stem file...");

    // Check if stem file was created
    QString stemFilePath = outputDir + "/" + baseName + ".stem.m4a";
    if (!QFileInfo(stemFilePath).exists()) {
        QString errorMsg = QString("Stem file was not created: %1").arg(stemFilePath);
        kLogger.warning() << errorMsg;
        setProgress(0.0f, errorMsg);
        setState(ConversionState::Failed);
        emit conversionFailed(m_currentTrackId, errorMsg);
        return false;
    }

    kLogger.info() << "Stem conversion completed for:" << trackLocation;
    kLogger.info() << "Stem file created at:" << stemFilePath;

    setProgress(1.0f, "Stem conversion completed successfully");
    setState(ConversionState::Completed);
    emit conversionCompleted(m_currentTrackId);

    return true;
}

void StemConverter::stopConversion() {
    if (m_pProcess && m_pProcess->state() == QProcess::Running) {
        m_pProcess->kill();
        setState(ConversionState::Failed);
        setProgress(0.0f, "Conversion stopped by user");
    }
}

void StemConverter::setState(ConversionState state) {
    m_state = state;
}

void StemConverter::setProgress(float progress, const QString& message) {
    m_progress = qBound(0.0f, progress, 1.0f);
    m_statusMessage = message;
    emit conversionProgress(m_currentTrackId, m_progress, message);
}

void StemConverter::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    // Handled in convertTrack
}

void StemConverter::onProcessError(QProcess::ProcessError error) {
    QString errorMsg;
    switch (error) {
        case QProcess::FailedToStart:
            errorMsg = "Failed to start stemgen process";
            break;
        case QProcess::Crashed:
            errorMsg = "Stemgen process crashed";
            break;
        case QProcess::Timedout:
            errorMsg = "Stemgen process timeout";
            break;
        default:
            errorMsg = "Unknown error in stemgen process";
            break;
    }

    kLogger.warning() << errorMsg;
    setState(ConversionState::Failed);
    emit conversionFailed(m_currentTrackId, errorMsg);
}

void StemConverter::onProcessOutput() {
    if (!m_pProcess) {
        return;
    }

    QString output = QString::fromUtf8(m_pProcess->readAllStandardOutput());
    QString errorOutput = QString::fromUtf8(m_pProcess->readAllStandardError());

    // Combinar stdout i stderr
    QString allOutput = output + errorOutput;

    // Parse output to update progress
    // Buscar patterns de progrés en la sortida de stemgen

    // Pattern 1: "Processing chunk X/Y"
    QRegularExpression chunkPattern(R"(Processing chunk (\d+)/(\d+))");
    QRegularExpressionMatch chunkMatch = chunkPattern.match(allOutput);
    if (chunkMatch.hasMatch()) {
        int current = chunkMatch.captured(1).toInt();
        int total = chunkMatch.captured(2).toInt();
        if (total > 0) {
            float progress = 0.3f + (0.6f * current / total);  // 30% to 90%
            setProgress(progress, QString("Processing chunk %1/%2").arg(current).arg(total));
            return;
        }
    }

    // Pattern 2: "Saving"
    if (allOutput.contains("Saving", Qt::CaseInsensitive)) {
        setProgress(0.85f, "Saving stem file...");
        return;
    }

    // Pattern 3: "Processing"
    if (allOutput.contains("Processing", Qt::CaseInsensitive)) {
        setProgress(0.50f, "Processing audio...");
        return;
    }

    // Pattern 4: "Loading"
    if (allOutput.contains("Loading", Qt::CaseInsensitive)) {
        setProgress(0.25f, "Loading audio file...");
        return;
    }
}

#include "moc_stemconverter.cpp"
