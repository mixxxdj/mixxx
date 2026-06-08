#include "stems/stemconverter.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QStandardPaths>

#ifdef __STEM_CONVERSION__
#include <onnxruntime_cxx_api.h>
#include <sndfile.h>
#endif

#include "sources/soundsourceproxy.h"
#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("StemConverter");
}

StemConverter::StemConverter(QObject* parent)
        : QObject(parent),
          m_state(ConversionState::Idle),
          m_progress(0.0f),
          m_trackTitle("Unknown") {
#ifdef __STEM__
    // Suppress ONNX Runtime verbose output
    setenv("ORT_DISABLE_TELEMETRY", "1", 1);
    setenv("ORT_DISABLE_LOGGING", "1", 1);

    // Redirect stderr to suppress ONNX schema warnings
    int devNull = open("/dev/null", O_WRONLY);
    if (devNull != -1) {
        dup2(devNull, STDERR_FILENO);
        close(devNull);
    }

    try {
        m_pOrtEnv = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "StemConverter");
        kLogger.info() << "ONNX Runtime environment initialized";
    } catch (const std::exception& e) {
        kLogger.warning() << "Failed to initialize ONNX Runtime:" << e.what();
    }
#endif
}

void StemConverter::convertTrack(const TrackPointer& pTrack, Resolution resolution) {
    if (!pTrack) {
        qWarning() << "Cannot convert null track";
        emit conversionFailed(pTrack->getId(), "Track is null");
        return;
    }

    m_pTrack = pTrack;
    m_resolution = resolution;

    QString trackFilePath = pTrack->getLocation();
    QFileInfo fileInfo(trackFilePath);
    m_trackTitle = fileInfo.fileName();

    m_state = ConversionState::Processing;
    m_progress = 0.0f;

    if (trackFilePath.isEmpty()) {
        qWarning() << "Track file path is empty";
        emit conversionFailed(pTrack->getId(), "Track file path is empty");
        m_state = ConversionState::Failed;
        return;
    }

    emit conversionStarted(pTrack->getId(), m_trackTitle);
    emit conversionProgress(pTrack->getId(), 0.1f, "Starting ONNX separation...");

#ifdef __STEM__
    QString stemsDir = getStemsDirectory(trackFilePath);
    if (!runOnnxSeparation(trackFilePath, stemsDir)) {
        QString errorMsg = "ONNX separation failed";
        kLogger.warning() << errorMsg;
        emit conversionFailed(pTrack->getId(), errorMsg);
        m_state = ConversionState::Failed;
        return;
    }
#else
    QString errorMsg = "STEM conversion not available (ONNX Runtime not compiled)";
    kLogger.warning() << errorMsg;
    emit conversionFailed(pTrack->getId(), errorMsg);
    m_state = ConversionState::Failed;
    return;
#endif

    emit conversionProgress(pTrack->getId(), 0.5f, "Converting stems to M4A...");

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

#ifdef __STEM__
bool StemConverter::loadOnnxModel() {
    if (m_pOrtSession) {
        return true; // Already loaded
    }

    if (!m_pOrtEnv) {
        kLogger.warning() << "ONNX Runtime environment not initialized";
        return false;
    }

    try {
        // Get model path from user's home directory
        QString homeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        QString modelPath = homeDir + "/.local/mixxx_models/htdemucs.onnx";

        if (!QFile::exists(modelPath)) {
            kLogger.warning() << "ONNX model not found at:" << modelPath;
            return false;
        }

        Ort::SessionOptions sessionOptions;
        sessionOptions.SetIntraOpNumThreads(4);
        sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        m_pOrtSession = std::make_unique<Ort::Session>(
                *m_pOrtEnv,
                modelPath.toStdString().c_str(),
                sessionOptions);

        kLogger.info() << "ONNX model loaded successfully from:" << modelPath;
        return true;
    } catch (const std::exception& e) {
        kLogger.warning() << "Failed to load ONNX model:" << e.what();
        return false;
    }
}

bool StemConverter::runInference(const std::vector<float>& inputAudio,
        int sampleRate,
        std::vector<std::vector<float>>& outputStems) {
    if (!m_pOrtSession) {
        kLogger.warning() << "ONNX session not initialized";
        return false;
    }

    try {
        // Prepare input tensor
        // HTDemucs expects input shape: (1, 2, 343980) for stereo
        // The model requires exactly 343980 samples per channel
        Q_UNUSED(sampleRate);
        const int64_t expected_samples_per_channel = 343980;
        std::vector<int64_t> inputShape = {1, 2, expected_samples_per_channel};

        Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
                m_allocator.GetInfo(),
                const_cast<float*>(inputAudio.data()),
                inputAudio.size(),
                inputShape.data(),
                inputShape.size());

        const char* inputNames[] = {"input"};
        const char* outputNames[] = {"output"};

        // Run inference
        auto outputTensors = m_pOrtSession->Run(
                Ort::RunOptions{nullptr},
                inputNames,
                &inputTensor,
                1,
                outputNames,
                1);

        // Extract output stems from the single output tensor
        // Output shape is expected to be [1, 4, 2, 343980] (batch, stems, channels, samples)
        auto& outputTensor = outputTensors[0];
        float* outputData = outputTensor.GetTensorMutableData<float>();
        auto tensorInfo = outputTensor.GetTensorTypeAndShapeInfo();
        auto outputShape = tensorInfo.GetShape();

        // Expected: [1, 4, 2, 343980]
        if (outputShape.size() != 4 || outputShape[1] != 4) {
            kLogger.warning() << "Unexpected output tensor shape";
            return false;
        }

        int64_t numStems = outputShape[1];
        int64_t numChannels = outputShape[2];
        int64_t numSamples = outputShape[3];

        outputStems.resize(numStems);
        for (int i = 0; i < numStems; ++i) {
            int64_t stemSize = numChannels * numSamples;
            float* stemStart = outputData + (i * stemSize);
            outputStems[i].assign(stemStart, stemStart + stemSize);
        }

        kLogger.info() << "ONNX inference completed successfully";
        return true;
    } catch (const std::exception& e) {
        kLogger.warning() << "ONNX inference failed:" << e.what();
        return false;
    }
}

bool StemConverter::decodeAudioFile(const QString& inputPath,
        std::vector<float>& audioData,
        int& sampleRate,
        int& channels,
        int& originalFrames) {
    // Use Mixxx's SoundSourceProxy to read the audio file
    // Create a temporary track for reading
    auto pTrack = Track::newTemporary(inputPath);

    SoundSourceProxy proxy(pTrack);
    auto pAudioSource = proxy.openAudioSource();
    if (!pAudioSource) {
        kLogger.warning() << "Failed to open audio file:" << inputPath;
        return false;
    }

    sampleRate = pAudioSource->getSignalInfo().getSampleRate();
    channels = pAudioSource->getSignalInfo().getChannelCount();

    // The htdemucs model expects fixed-size chunks of stereo audio.
    // The total number of samples per chunk is 343980 for each of the 2 channels.
    const int64_t expected_samples_per_channel = 343980;
    const int64_t expected_total_samples = expected_samples_per_channel * channels;

    // Create a buffer to read samples
    mixxx::SampleBuffer sampleBuffer(expected_total_samples);

    // Read samples from the audio source
    mixxx::WritableSampleFrames writableFrames(
            mixxx::IndexRange::between(0, expected_samples_per_channel),
            mixxx::SampleBuffer::WritableSlice(sampleBuffer));

    mixxx::ReadableSampleFrames readableFrames = pAudioSource->readSampleFrames(writableFrames);
    originalFrames = readableFrames.frameIndexRange().length();

    // Resize and pad the audio data to the expected size
    audioData.resize(expected_total_samples, 0.0f);

    // Copy the buffer data, ensuring we don't overflow
    size_t samplesToCopy = std::min((size_t)(originalFrames * channels),
            (size_t)expected_total_samples);
    std::copy(sampleBuffer.data(),
            sampleBuffer.data() + samplesToCopy,
            audioData.begin());

    kLogger.info() << "Audio file decoded. Original samples:"
                   << (originalFrames * channels)
                   << "Resized to (samples):" << audioData.size()
                   << "Padding applied:"
                   << (expected_total_samples - samplesToCopy);

    return true;
}

bool StemConverter::saveStemToWav(const std::vector<float>& audioData,
        const QString& outputPath,
        int sampleRate,
        int channels,
        int originalFrames) {
    SF_INFO sfInfo;
    memset(&sfInfo, 0, sizeof(sfInfo));

    sfInfo.samplerate = sampleRate;
    sfInfo.channels = channels;
    sfInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    SNDFILE* outfile = sf_open(outputPath.toStdString().c_str(), SFM_WRITE, &sfInfo);
    if (!outfile) {
        kLogger.warning() << "Failed to create WAV file:" << sf_strerror(nullptr);
        return false;
    }

    // Only write the original frames, not the padding
    sf_count_t framesToWrite = originalFrames;
    sf_count_t writtenFrames = sf_writef_float(outfile, audioData.data(), framesToWrite);

    if (writtenFrames != framesToWrite) {
        kLogger.warning() << "Failed to write all frames to WAV file";
        sf_close(outfile);
        return false;
    }

    sf_close(outfile);
    kLogger.info() << "Stem saved to WAV:" << outputPath;
    return true;
}

bool StemConverter::runOnnxSeparation(const QString& trackFilePath, const QString& outputDir) {
    kLogger.info() << "Starting ONNX separation for:" << trackFilePath;

    // Load model
    if (!loadOnnxModel()) {
        kLogger.warning() << "Failed to load ONNX model";
        return false;
    }

    // Decode audio file
    std::vector<float> audioData;
    int sampleRate = 0;
    int channels = 0;

    if (!decodeAudioFile(trackFilePath, audioData, sampleRate, channels, m_originalFrames)) {
        kLogger.warning() << "Failed to decode audio file";
        return false;
    }

    // Run inference
    std::vector<std::vector<float>> outputStems;
    if (!runInference(audioData, sampleRate, outputStems)) {
        kLogger.warning() << "Failed to run ONNX inference";
        return false;
    }

    // Save stems to WAV files
    QStringList stemNames = {"drums", "bass", "other", "vocals"};
    QDir().mkpath(outputDir);

    for (size_t i = 0; i < outputStems.size(); ++i) {
        QString stemPath = outputDir + "/" + stemNames[i] + ".wav";
        if (!saveStemToWav(outputStems[i], stemPath, sampleRate, channels, m_originalFrames)) {
            kLogger.warning() << "Failed to save stem:" << stemNames[i];
            return false;
        }
    }

    kLogger.info() << "ONNX separation completed successfully";
    return true;
}
#endif

QString StemConverter::getStemsDirectory(const QString& trackFilePath) {
    QFileInfo fileInfo(trackFilePath);
    QString fileName = fileInfo.baseName();
    QString dirPath = fileInfo.absolutePath();

    // Create stems directory
    return dirPath + "/" + fileName;
}

bool StemConverter::convertStemsToM4A(const QString& stemsDir) {
    kLogger.info() << "Converting stems to M4A:" << stemsDir;

    QDir dir(stemsDir);
    if (!dir.exists()) {
        kLogger.warning() << "Stems directory does not exist:" << stemsDir;
        return false;
    }

    QStringList stemNames = {"drums", "bass", "other", "vocals"};

    for (const QString& stemName : stemNames) {
        QString wavPath = stemsDir + "/" + stemName + ".wav";
        QString m4aPath = stemsDir + "/" + stemName + ".m4a";

        if (!QFile::exists(wavPath)) {
            kLogger.warning() << "WAV file not found:" << wavPath;
            continue;
        }

        QProcess ffmpegProcess;
        QStringList args;
        args << "-i" << wavPath
             << "-c:a" << "aac"
             << "-b:a" << "320k"
             << "-y" << m4aPath;

        ffmpegProcess.start("ffmpeg", args);
        if (!ffmpegProcess.waitForFinished(-1)) {
            kLogger.warning() << "ffmpeg timeout for:" << stemName;
            return false;
        }

        if (ffmpegProcess.exitCode() != 0) {
            kLogger.warning() << "ffmpeg failed for:" << stemName;
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
    arguments << "-c:a" << "alac";
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
    ffmpegArgs << "-map" << "0:a:0"; // mixdown
    ffmpegArgs << "-map" << "1:a:0"; // drums
    ffmpegArgs << "-map" << "2:a:0"; // bass
    ffmpegArgs << "-map" << "3:a:0"; // other
    ffmpegArgs << "-map" << "4:a:0"; // vocals

    ffmpegArgs << "-c:a" << "alac";
    ffmpegArgs << "-movflags" << "+faststart";
    ffmpegArgs << "-fflags" << "+bitexact";
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
    tempJsonFile.setAutoRemove(false); // We'll delete it manually

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

bool StemConverter::addMetadataTags(
        const QString& /* trackFilePath */, const QString& /* stemsDir */) {
    kLogger.info() << "Adding metadata tags";
    return true;
}

#include "moc_stemconverter.cpp"
