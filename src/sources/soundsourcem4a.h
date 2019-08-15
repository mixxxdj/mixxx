#ifndef MIXXX_SOUNDSOURCEM4A_H
#define MIXXX_SOUNDSOURCEM4A_H

#include "sources/soundsource.h"
#include "sources/soundsourceprovider.h"

#include "sources/libfaadloader.h"

#include "util/readaheadsamplebuffer.h"

#ifdef __MP4V2__
#include <mp4v2/mp4v2.h>
#else
#include <mp4.h>
#endif

#include <vector>

namespace mixxx {

class SoundSourceM4A : public SoundSource {
  public:
    explicit SoundSourceM4A(const QUrl& url);
    ~SoundSourceM4A() override;

    void close() override;

  protected:
    ReadableSampleFrames readSampleFramesClamped(
            WritableSampleFrames sampleFrames) override;

  private:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override;

    bool openDecoder();
    void closeDecoder();
    bool reopenDecoder();

    bool isValidSampleBlockId(MP4SampleId sampleBlockId) const;

    void restartDecoding(MP4SampleId sampleBlockId);

    MP4FileHandle m_hFile;
    MP4TrackId m_trackId;
    MP4Duration m_framesPerSampleBlock;
    MP4SampleId m_maxSampleBlockId;

    typedef std::vector<u_int8_t> InputBuffer;
    InputBuffer m_inputBuffer;
    SINT m_inputBufferLength;
    SINT m_inputBufferOffset;

    OpenParams m_openParams;

    LibFaadLoader::Handle m_hDecoder;
    SINT m_numberOfPrefetchSampleBlocks;
    MP4SampleId m_curSampleBlockId;

    ReadAheadSampleBuffer m_sampleBuffer;

    SINT m_curFrameIndex;

    LibFaadLoader* m_pFaad;
};

class SoundSourceProviderM4A : public SoundSourceProvider {
  public:
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override;
};

} // namespace mixxx

#endif // MIXXX_SOUNDSOURCEM4A_H
