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

    struct Config {
        QString inputPath;
        QString modelPath;
        QString outputDir;
        int sampleRate = 44100;
        /// N18: Hann overlap ratio (0.5 default, 0.25 optional for speed).
        /// 0.25 triggers WOLA weight renormalization in run().
        double overlap = 0.5;
        /// N18: stem mode (4 default, 3 folds bass+other into slot 3
        /// "Instruments" and silences slot 2). Reader layout untouched.
        int stemMode = 4;
        ProgressCallback onProgress;
        FinishedCallback onFinished;
    };

    explicit OfflineSeparator(const Config& config);
    ~OfflineSeparator() override;

    void run() override;
    void start();

signals:
    void progressChanged(float progress);
    void finished(bool success, const StemCacheManager::StemFiles& files);

private:
    void doSeparation();

    Config m_config;
    bool m_cancelled = false;
};

} // namespace mixxx