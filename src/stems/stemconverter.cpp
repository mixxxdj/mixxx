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

    QString mixdownM4A = stemsDir + "/mixdown.m4a";
    if (!convertTrackToM4A(trackFilePath, mixdownM4A)) {
        kLogger.warning() << "Failed to create mixdown M4A";
        return false;
    }

    QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString venvPath = homeDir + "/.local/mixxx_venv";
    QString pythonPath = venvPath + "/bin/python3";
    QString scriptPath = venvPath + "/bin/stems_create_container.py";

    if (!QFile::exists(pythonPath)) {
        kLogger.warning() << "Python not found in virtual environment:" << pythonPath;
        return false;
    }

    if (!QFile::exists(scriptPath)) {
        kLogger.warning() << "Python script not found:" << scriptPath;
        return false;
    }

    kLogger.info() << "Using Python from venv:" << pythonPath;
    kLogger.info() << "Using script:" << scriptPath;

    QProcess process;
    QStringList arguments;
    arguments << scriptPath << mixdownM4A << stemsDir << outputPath;

    kLogger.info() << "Running:" << pythonPath << arguments.join(" ");
    process.start(pythonPath, arguments);

    if (!process.waitForFinished(-1)) {
        QString errorMsg = process.errorString();
        kLogger.warning() << "Python process failed:" << errorMsg;
        return false;
    }

    if (process.exitCode() != 0) {
        QString errorOutput = process.readAllStandardError();
        QString standardOutput = process.readAllStandardOutput();
        kLogger.warning() << "Python exit code:" << process.exitCode();
        kLogger.warning() << "Python stderr:" << errorOutput;
        kLogger.warning() << "Python stdout:" << standardOutput;
        return false;
    }

    QString standardOutput = process.readAllStandardOutput();
    kLogger.info() << "Python output:" << standardOutput;

    kLogger.info() << "STEM container created:" << outputPath;
    return true;
}

QString StemConverter::createMetadataJson() {
    QJsonObject stemsArray;

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
    root["stems"] = stems;

    QJsonDocument doc(root);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString StemConverter::createPythonScript() {
    return "";
}

bool StemConverter::addMetadataTags(const QString& /* trackFilePath */, const QString& /* stemsDir */) {
    kLogger.info() << "Adding metadata tags";
    return true;
}

#include "moc_stemconverter.cpp"
