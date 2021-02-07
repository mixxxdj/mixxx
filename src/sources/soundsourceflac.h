#pragma once

#include <FLAC/stream_decoder.h>

#include <QFile>

#include "sources/soundsourceprovider.h"
#include "util/readaheadsamplebuffer.h"

namespace mixxx {

class SoundSourceFLAC final : public SoundSource {
  public:
    explicit SoundSourceFLAC(const QUrl& url);
    ~SoundSourceFLAC() override;

    void close() override;

    // Internal callbacks
    FLAC__StreamDecoderReadStatus flacRead(FLAC__byte buffer[], size_t* bytes);
    FLAC__StreamDecoderSeekStatus flacSeek(FLAC__uint64 offset);
    FLAC__StreamDecoderTellStatus flacTell(FLAC__uint64* offset);
    FLAC__StreamDecoderLengthStatus flacLength(FLAC__uint64* length);
    FLAC__bool flacEOF();
    FLAC__StreamDecoderWriteStatus flacWrite(const FLAC__Frame* frame,
            const FLAC__int32* const buffer[]);
    void flacMetadata(const FLAC__StreamMetadata* metadata);
    void flacError(FLAC__StreamDecoderErrorStatus status);

  protected:
    ReadableSampleFrames readSampleFramesClamped(
            const WritableSampleFrames& sampleFrames) override;

  private:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override;

    QFile m_file;

    FLAC__StreamDecoder* m_decoder;
    // misc bits about the flac format:
    // flac encodes from and decodes to LPCM in blocks, each block is made up of
    // subblocks (one for each chan)
    // flac stores in 'frames', each of which has a header and a certain number
    // of subframes (one for each channel)
    SINT m_maxBlocksize; // in time samples (audio samples = time samples * chanCount)
    SINT m_bitsPerSample;

    ReadAheadSampleBuffer m_sampleBuffer;

    void invalidateCurFrameIndex() {
        m_curFrameIndex = frameIndexMax();
    }

    SINT m_curFrameIndex;
};

class SoundSourceProviderFLAC : public SoundSourceProvider {
  public:
    static const QString kDisplayName;
    static const QStringList kSupportedFileExtensions;

    QString getDisplayName() const override {
        return kDisplayName;
    }

    QStringList getSupportedFileExtensions() const override {
        return kSupportedFileExtensions;
    }

    SoundSourceProviderPriority getPriorityHint(
            const QString& supportedFileExtension) const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return newSoundSourceFromUrl<SoundSourceFLAC>(url);
    }
};

} // namespace mixxx
