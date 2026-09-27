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