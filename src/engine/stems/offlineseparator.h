#pragma once

#include <QObject>
#include <QRunnable>
#include <QString>
#include <functional>

#include "engine/stems/stemcachemanager.h"

namespace mixxx {

class OfflineSeparator : public QObject, public QRunnable {
    Q_OBJECT
public:
    using ProgressCallback = std::function<void(float progress)>;
    using FinishedCallback = std::function<void(bool success, const StemCacheManager::StemFiles& files)>;
    // N21: chunk-streaming preview callback (partial .stem.mp4 ready,
    // background job still running).
    using PartialCallback = std::function<void(const StemCacheManager::StemFiles& files)>;

    struct Config {
        QString inputPath;
        QString modelPath;
        QString outputDir;
        int sampleRate = 44100;
        /// N18: Hann overlap ratio (0.25 default, 0.5 optional for quality).
        /// 0.25 triggers WOLA weight renormalization in run().
        double overlap = 0.25;
        /// N18: stem mode (3 default, 4 keeps the legacy 4-stem layout:
        /// slot 3 "Instruments" fold off). Reader layout untouched.
        int stemMode = 3;
        ProgressCallback onProgress;
        FinishedCallback onFinished;
        PartialCallback onPartial;
    };

    explicit OfflineSeparator(const Config& config);
    ~OfflineSeparator() override;

    void run() override;
    void start();

signals:
    void progressChanged(float progress);
    void finished(bool success, const StemCacheManager::StemFiles& files);
    // N21: emitted once after kPartialChunks chunks with a playable
    // preview .stem.mp4; the job keeps running in background with
    // progressChanged until finished().
    void partialReady(bool success, const StemCacheManager::StemFiles& files);

private:
    void doSeparation();

    Config m_config;
    bool m_cancelled = false;
};

} // namespace mixxx