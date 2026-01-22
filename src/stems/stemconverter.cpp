#include "stems/stemconverter.h"

#include <QProcess>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("StemConverter");
}

StemConverter::StemConverter(QObject* parent)
        : QObject(parent),
          m_state(ConversionState::Idle),
          m_progress(0.0f),
          m_trackTitle("Unknown") {
}

void StemConverter::convertTrack(const TrackPointer& pTrack, Resolution resolution) {
    if (!pTrack) {
        qWarning() << "Cannot convert null track";
        emit conversionFailed(pTrack->getId(), "Track is null");
        return;
    }

    m_pTrack = pTrack;
    m_resolution = resolution;
    m_trackTitle = pTrack->getTitle();
    m_state = ConversionState::Processing;
    m_progress = 0.0f;

    QString trackFilePath = pTrack->getLocation();
    if (trackFilePath.isEmpty()) {
        qWarning() << "Track file path is empty";
        emit conversionFailed(pTrack->getId(), "Track file path is empty");
        m_state = ConversionState::Failed;
        return;
    }

    emit conversionStarted(pTrack->getId(), m_trackTitle);
    emit conversionProgress(pTrack->getId(), 0.1f, "Starting demucs separation...");

    if (!runDemucs(trackFilePath)) {
        QString errorMsg = "Demucs separation failed";
        kLogger.warning() << errorMsg;
        emit conversionFailed(pTrack->getId(), errorMsg);
        m_state = ConversionState::Failed;
        return;
    }

    emit conversionProgress(pTrack->getId(), 0.5f, "Converting stems to M4A...");

    QString stemsDir = getStemsDirectory(trackFilePath);
    if (!convertStemsToM4A(stemsDir)) {
        QString errorMsg = "Stem conversion to M4A failed";
        kLogger.warning() << errorMsg;
        emit conversionFailed(pTrack->getId(), errorMsg);
        m_state = ConversionState::Failed;
        return;
    }

    emit conversionProgress(pTrack->getId(), 0.7f, "Creating STEM container...");

    if (!createStemContainer(trackFilePath, stemsDir)) {
        QString errorMsg = "STEM container creation failed";
        kLogger.warning() << errorMsg;
        emit conversionFailed(pTrack->getId(), errorMsg);
        m_state = ConversionState::Failed;
        return;
    }

    emit conversionProgress(pTrack->getId(), 0.9f, "Adding metadata tags...");

    if (!addMetadataTags(trackFilePath, stemsDir)) {
        kLogger.warning() << "Failed to add metadata tags (non-critical)";
    }

    emit conversionProgress(pTrack->getId(), 1.0f, "Conversion completed successfully!");
    emit conversionCompleted(pTrack->getId());

    m_state = ConversionState::Completed;
    kLogger.info() << "Track conversion completed:" << trackFilePath;
}

float StemConverter::getProgress() const {
    return m_progress;
}

QString StemConverter::getTrackTitle() const {
    return m_trackTitle;
}

StemConverter::ConversionState StemConverter::getState() const {
    return m_state;
}

bool StemConverter::runDemucs(const QString& trackFilePath) {
    kLogger.info() << "Starting demucs for:" << trackFilePath;

    QProcess process;
    QFileInfo fileInfo(trackFilePath);
    QString trackDir = fileInfo.absolutePath();

    QString model = (m_resolution == Resolution::High) ? "htdemucs_ft" : "htdemucs";

    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString venvPath = homeDir + "/.local/mixxx_venv";
    QString demucsPath = venvPath + "/bin/demucs";

    if (!QFile::exists(demucsPath)) {
        kLogger.warning() << "Demucs not found in virtual environment:" << demucsPath;
        kLogger.warning() << "Trying system demucs...";
        demucsPath = "demucs";
    }

    QStringList arguments;
    arguments << "-n" << model;
    arguments << "-o" << trackDir;
    arguments << "--shifts" << "1";
    arguments << "-d" << "cpu";
    arguments << "--mp3";
    arguments << "--mp3-bitrate" << "320";
    arguments << trackFilePath;

    kLogger.info() << "Demucs command:" << demucsPath << arguments.join(" ");

    process.start(demucsPath, arguments);

    if (!process.waitForFinished(-1)) {
        QString errorMsg = process.errorString();
        kLogger.warning() << "Demucs process failed:" << errorMsg;
        return false;
    }

    if (process.exitCode() != 0) {
        QString errorOutput = process.readAllStandardError();
        kLogger.warning() << "Demucs error:" << errorOutput;
        return false;
    }

    kLogger.info() << "Demucs completed successfully";
    return true;
}

QString StemConverter::getStemsDirectory(const QString& trackFilePath) {
    QFileInfo fileInfo(trackFilePath);
    QString fileName = fileInfo.baseName();
    QString trackDir = fileInfo.absolutePath();

    QString model = (m_resolution == Resolution::High) ? "htdemucs_ft" : "htdemucs";
    QString stemsDir = trackDir + "/" + model + "/" + fileName;

    kLogger.info() << "Stems directory:" << stemsDir;

    return stemsDir;
}

bool StemConverter::convertStemsToM4A(const QString& stemsDir) {
    kLogger.info() << "Converting stems to M4A:" << stemsDir;

    QDir dir(stemsDir);
    if (!dir.exists()) {
        kLogger.warning() << "Stems directory does not exist:" << stemsDir;
        return false;
    }

    QStringList stemFiles = {"drums.mp3", "bass.mp3", "other.mp3", "vocals.mp3"};

    for (const QString& stemFile : stemFiles) {
        QString inputPath = stemsDir + "/" + stemFile;
        QString outputFileName = stemFile;
        outputFileName.replace(".mp3", ".m4a");
        QString outputPath = stemsDir + "/" + outputFileName;

        if (!convertTrackToM4A(inputPath, outputPath)) {
            kLogger.warning() << "Failed to convert:" << inputPath;
            return false;
        }
    }

    kLogger.info() << "All stems converted to M4A";
    return true;
}

bool StemConverter::convertTrackToM4A(const QString& inputPath, const QString& outputPath) {
    kLogger.info() << "Converting to M4A:" << inputPath;

    QProcess process;
    QStringList arguments;

    arguments << "-i" << inputPath;
    arguments << "-c:a" << "aac";
    arguments << "-b:a" << "256k";
    arguments << "-y" << outputPath;

    process.start("ffmpeg", arguments);

    if (!process.waitForFinished(-1)) {
        kLogger.warning() << "ffmpeg process failed:" << process.errorString();
        return false;
    }

    if (process.exitCode() != 0) {
        QString errorOutput = process.readAllStandardError();
        kLogger.warning() << "ffmpeg error:" << errorOutput;
        return false;
    }

    kLogger.info() << "Converted to M4A:" << outputPath;
    return true;
}

bool StemConverter::createStemContainer(const QString& trackFilePath, const QString& stemsDir) {
    kLogger.info() << "Creating STEM container";

    QFileInfo fileInfo(trackFilePath);
    QString fileName = fileInfo.baseName();
    QString trackDir = fileInfo.absolutePath();
    QString outputPath = trackDir + "/" + fileName + ".stem.m4a";

    // Create mixdown M4A from original track
    QString mixdownM4A = stemsDir + "/mixdown.m4a";
    if (!convertTrackToM4A(trackFilePath, mixdownM4A)) {
        kLogger.warning() << "Failed to create mixdown M4A";
        return false;
    }

    // Create multi-track M4A with ffmpeg
    // Combine: mixdown.m4a + drums.m4a + bass.m4a + other.m4a + vocals.m4a
    QProcess ffmpegProcess;
    QStringList ffmpegArgs;

    ffmpegArgs << "-y";
    ffmpegArgs << "-i" << mixdownM4A;
    ffmpegArgs << "-i" << (stemsDir + "/drums.m4a");
    ffmpegArgs << "-i" << (stemsDir + "/bass.m4a");
    ffmpegArgs << "-i" << (stemsDir + "/other.m4a");
    ffmpegArgs << "-i" << (stemsDir + "/vocals.m4a");

    // Map all audio tracks
    ffmpegArgs << "-map" << "0:a:0";  // mixdown
    ffmpegArgs << "-map" << "1:a:0";  // drums
    ffmpegArgs << "-map" << "2:a:0";  // bass
    ffmpegArgs << "-map" << "3:a:0";  // other
    ffmpegArgs << "-map" << "4:a:0";  // vocals

    ffmpegArgs << "-c:a" << "aac";
    ffmpegArgs << "-b:a" << "256k";
    ffmpegArgs << "-movflags" << "+faststart";
    ffmpegArgs << outputPath;

    kLogger.info() << "Running ffmpeg to create multi-track M4A...";
    kLogger.info() << "Command: ffmpeg" << ffmpegArgs.join(" ");

    ffmpegProcess.start("ffmpeg", ffmpegArgs);

    if (!ffmpegProcess.waitForFinished(-1)) {
        QString errorMsg = ffmpegProcess.errorString();
        kLogger.warning() << "ffmpeg process failed:" << errorMsg;
        return false;
    }

    if (ffmpegProcess.exitCode() != 0) {
        QString errorOutput = ffmpegProcess.readAllStandardError();
        kLogger.warning() << "ffmpeg error:" << errorOutput;
        return false;
    }

    kLogger.info() << "Multi-track M4A created:" << outputPath;

    // Add STEM metadata atom
    if (!addStemMetadata(outputPath)) {
        kLogger.warning() << "Failed to add STEM metadata (non-critical)";
        // Don't fail - the file is still usable without metadata
    }

    kLogger.info() << "STEM container created:" << outputPath;
    return true;
}

QString StemConverter::createStemManifest() {
    QJsonObject masteringDsp;

    // Compressor settings
    QJsonObject compressor;
    compressor["enabled"] = false;
    compressor["ratio"] = 3;
    compressor["output_gain"] = 0.5;
    compressor["release"] = 0.3;
    compressor["attack"] = 0.003;
    compressor["input_gain"] = 0.5;
    compressor["threshold"] = 0;
    compressor["hp_cutoff"] = 300;
    compressor["dry_wet"] = 50;

    // Limiter settings
    QJsonObject limiter;
    limiter["enabled"] = false;
    limiter["release"] = 0.05;
    limiter["threshold"] = 0;
    limiter["ceiling"] = -0.35;

    masteringDsp["compressor"] = compressor;
    masteringDsp["limiter"] = limiter;

    // Stems metadata
    QJsonObject drums;
    drums["name"] = "Drums";
    drums["color"] = "#009E73";

    QJsonObject bass;
    bass["name"] = "Bass";
    bass["color"] = "#D55E00";

    QJsonObject other;
    other["name"] = "Other";
    other["color"] = "#CC79A7";

    QJsonObject vox;
    vox["name"] = "Vox";
    vox["color"] = "#56B4E9";

    QJsonArray stems;
    stems.append(drums);
    stems.append(bass);
    stems.append(other);
    stems.append(vox);

    QJsonObject root;
    root["mastering_dsp"] = masteringDsp;
    root["version"] = 1;
    root["stems"] = stems;

    QJsonDocument doc(root);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString StemConverter::findMP4Box() {
    // List of possible MP4Box locations
    QStringList possibleLocations = {
        "/usr/local/bin/MP4Box",
        "/usr/bin/MP4Box",
        "/opt/local/bin/MP4Box",
    };

    for (const QString& location : possibleLocations) {
        if (QFile::exists(location)) {
            kLogger.info() << "Found MP4Box at:" << location;
            return location;
        }
    }

    // Try to find it in PATH using 'which'
    QProcess process;
    process.start("which", QStringList() << "MP4Box");

    if (process.waitForFinished(-1) && process.exitCode() == 0) {
        QString mp4boxPath = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
        kLogger.info() << "Found MP4Box in PATH:" << mp4boxPath;
        return mp4boxPath;
    }

    kLogger.warning() << "MP4Box not found in system";
    return "";
}

bool StemConverter::addStemMetadata(const QString& outputPath) {
    kLogger.info() << "Adding STEM metadata to:" << outputPath;

    QString mp4boxPath = findMP4Box();
    if (mp4boxPath.isEmpty()) {
        kLogger.warning() << "MP4Box not found. Skipping STEM metadata.";
        return false;
    }

    // Create temporary JSON file with STEM manifest
    QString stemManifest = createStemManifest();

    QTemporaryFile tempJsonFile;
    tempJsonFile.setAutoRemove(false);  // We'll delete it manually

    if (!tempJsonFile.open()) {
        kLogger.warning() << "Failed to create temporary JSON file";
        return false;
    }

    tempJsonFile.write(stemManifest.toUtf8());
    tempJsonFile.close();

    QString tempJsonPath = tempJsonFile.fileName();
    kLogger.info() << "Created temporary JSON at:" << tempJsonPath;

    // Run MP4Box to add STEM metadata
    QProcess mp4boxProcess;
    QStringList mp4boxArgs;

    mp4boxArgs << "-udta" << QString("0:type=stem:src=%1").arg(tempJsonPath);
    mp4boxArgs << outputPath;

    kLogger.info() << "Running MP4Box:" << mp4boxPath << mp4boxArgs.join(" ");
    mp4boxProcess.start(mp4boxPath, mp4boxArgs);

    if (!mp4boxProcess.waitForFinished(-1)) {
        QString errorMsg = mp4boxProcess.errorString();
        kLogger.warning() << "MP4Box process failed:" << errorMsg;
        QFile::remove(tempJsonPath);
        return false;
    }

    if (mp4boxProcess.exitCode() != 0) {
        QString errorOutput = mp4boxProcess.readAllStandardError();
        kLogger.warning() << "MP4Box error:" << errorOutput;
        QFile::remove(tempJsonPath);
        return false;
    }

    QString standardOutput = mp4boxProcess.readAllStandardOutput();
    kLogger.info() << "MP4Box output:" << standardOutput;

    // Clean up temporary file
    QFile::remove(tempJsonPath);

    kLogger.info() << "STEM metadata added successfully";
    return true;
}

bool StemConverter::addMetadataTags(const QString& /* trackFilePath */, const QString& /* stemsDir */) {
    kLogger.info() << "Adding metadata tags";
    return true;
}

#include "moc_stemconverter.cpp"
