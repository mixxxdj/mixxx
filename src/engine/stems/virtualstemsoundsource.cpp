#include "virtualstemsoundsource.h"

#include <QAudioDecoder>
#include <QAudioFormat>
#include <QEventLoop>
#include <QFile>
#include <QUrl>

#include "util/logger.h"

namespace {
const mixxx::Logger kLogger("VirtualStemSoundSource");
} // namespace

namespace mixxx {

VirtualStemSoundSource::VirtualStemSoundSource(const QString& filePath, int sampleRate)
    : AudioSource(QUrl::fromLocalFile(filePath)), m_filePath(filePath), m_sampleRate(sampleRate) {}

VirtualStemSoundSource::~VirtualStemSoundSource() = default;

bool VirtualStemSoundSource::load() {
    if (m_loaded) return true;
    
    QAudioFormat format;
    format.setSampleRate(m_sampleRate);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Float);
    
    QAudioDecoder decoder;
    decoder.setSource(QUrl::fromLocalFile(m_filePath));
    decoder.setAudioFormat(format);
    
    QVector<float> buffer;
    QEventLoop loop;
    bool decodeOk = false;
    
    QObject::connect(&decoder, &QAudioDecoder::bufferReady, [&]() {
        QAudioBuffer buf = decoder.read();
        if (buf.isValid()) {
            const float* data = buf.constData<float>();
            int samples = buf.sampleCount() * buf.format().channelCount();
            int oldSize = buffer.size();
            buffer.resize(oldSize + samples);
            std::copy_n(data, samples, buffer.begin() + oldSize);
        }
    });
    
    QObject::connect(&decoder, &QAudioDecoder::finished, [&]() {
        decodeOk = true;
        loop.quit();
    });
    QObject::connect(&decoder, qOverload<QAudioDecoder::Error>(&QAudioDecoder::error),
            [&](QAudioDecoder::Error err) {
                decodeOk = false;
                kLogger.warning() << "Decoder error:" << err;
                loop.quit();
            });
    
    decoder.start();
    loop.exec();
    
    if (!decodeOk || buffer.isEmpty()) {
        kLogger.warning() << "Failed to decode stem file:" << m_filePath;
        return false;
    }
    
    m_buffer = std::move(buffer);
    m_totalFrames = m_buffer.size() / 2; // stereo
    m_loaded = true;
    
    kLogger.info() << "Loaded virtual stem from" << m_filePath
                   << "frames:" << m_totalFrames << "@" << m_sampleRate << "Hz";
    return true;
}

VirtualStemSoundSource::OpenResult VirtualStemSoundSource::tryOpen(OpenMode mode, const OpenParams& params) {
    Q_UNUSED(mode);
    Q_UNUSED(params);
    if (!m_loaded) {
        if (!load()) return OpenResult::Aborted;
    }
    return OpenResult::Succeeded;
}

void VirtualStemSoundSource::close() {
    m_buffer.clear();
    m_readPosition = 0;
    m_totalFrames = 0;
    m_loaded = false;
}

mixxx::ReadableSampleFrames VirtualStemSoundSource::readSampleFramesClamped(const WritableSampleFrames& sampleFrames) {
    if (!m_loaded) {
        return mixxx::ReadableSampleFrames();
    }
    
    int numFrames = sampleFrames.frameLength();
    int framesAvailable = m_totalFrames - m_readPosition;
    int framesToRead = qMin(numFrames, framesAvailable);
    
    if (framesToRead <= 0) {
        return mixxx::ReadableSampleFrames();
    }
    
    mixxx::WritableSampleFrames writable = sampleFrames;
    if (writable.writableSlice().length() <= 0) {
        return mixxx::ReadableSampleFrames();
    }
    
    // Copy interleaved stereo data
    int srcPos = m_readPosition * 2;
    
    float* dst = writable.writableData();
    std::copy_n(m_buffer.constData() + srcPos, framesToRead * 2, dst);
    
    m_readPosition += framesToRead;
    
    SampleBuffer::ReadableSlice readableSlice(writable.writableData(), framesToRead * 2);
    return mixxx::ReadableSampleFrames(sampleFrames.frameIndexRange(), readableSlice);
}

void VirtualStemSoundSource::readFromBuffer(float* output, qint64 position, int frames) const {
    if (!m_loaded || position < 0 || frames <= 0) {
        return;
    }
    
    qint64 endPos = position + frames;
    if (endPos > m_totalFrames) {
        frames = m_totalFrames - position;
        if (frames <= 0) {
            return;
        }
    }
    
    int srcPos = position * 2;
    int samplesToCopy = frames * 2;
    std::copy_n(m_buffer.constData() + srcPos, samplesToCopy, output);
}

} // namespace mixxx