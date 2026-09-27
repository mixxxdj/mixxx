#pragma once

#include <QString>
#include <memory>

#include "sources/audiosource.h"
#include "engine/stems/stemcachemanager.h"

namespace mixxx {

class VirtualStemSoundSource : public AudioSource {
public:
    explicit VirtualStemSoundSource(const QString& filePath, int sampleRate = 44100);
    ~VirtualStemSoundSource() override;

    bool load();

    // AudioSource interface
    OpenResult tryOpen(OpenMode mode, const OpenParams& params) override;
    void close() override;
    ReadableSampleFrames readSampleFramesClamped(const WritableSampleFrames& sampleFrames) override;

    // Direct buffer access for real-time mixing
    void readFromBuffer(float* output, qint64 position, int frames) const;

    int getTotalFrames() const { return m_totalFrames; }
    bool isLoaded() const { return m_loaded; }

private:
    QString m_filePath;
    int m_sampleRate;
    QVector<float> m_buffer;
    qint64 m_readPosition = 0;
    qint64 m_totalFrames = 0;
    bool m_loaded = false;
};

} // namespace mixxx