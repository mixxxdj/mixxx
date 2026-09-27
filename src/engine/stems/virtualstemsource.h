#pragma once

#include <QObject>
#include <QString>
#include <memory>

#include "engine/stems/stemcachemanager.h"

namespace mixxx {
class VirtualStemSoundSource;
}

class VirtualStemSource : public QObject {
    Q_OBJECT
public:
    explicit VirtualStemSource(const StemCacheManager::StemFiles& files, int sampleRate = 44100);
    ~VirtualStemSource() override;

    // Load all 4 stem files into AudioSources
    bool load();

    // Get AudioSource for stem (0=vocals, 1=drums, 2=bass, 3=other)
    mixxx::VirtualStemSoundSource* getStemSource(int stemIdx) const;
    bool isLoaded() const { return m_loaded; }
    qint64 getTotalFrames() const { return m_totalFrames; }

private:
    int m_sampleRate;
    QString m_stemPaths[4];
    std::unique_ptr<mixxx::VirtualStemSoundSource> m_sources[4];
    qint64 m_totalFrames = 0;
    bool m_loaded = false;
};