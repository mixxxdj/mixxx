#pragma once

#include "sources/soundsourceprovider.h"

#ifdef _MSC_VER
// So mad.h doesn't try to use inline assembly which MSVC doesn't support.
// Notably, FPM_64BIT does not require a 64-bit machine. It merely requires a
// compiler that supports 64-bit types.
#define FPM_64BIT
#endif
#include <mad.h>

#include <QFile>

#include <vector>

namespace mixxx {

class SoundSourceMp3 final : public SoundSource {
  public:
    explicit SoundSourceMp3(const QUrl& url);
    ~SoundSourceMp3() override;

    void close() override;

  protected:
    ReadableSampleFrames readSampleFramesClamped(
            const WritableSampleFrames& sampleFrames) override;

  private:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override;

    QFile m_file;
    quint64 m_fileSize;
    unsigned char* m_pFileData;

    /** Struct used to store mad frames for seeking */
    struct SeekFrameType {
        SINT frameIndex;
        const unsigned char* pInputData;
    };

    /** It is not possible to make a precise seek in an mp3 file without decoding the whole stream.
     * To have precise seek within a limited range from the current decode position, we keep track
     * of past decoded frame, and their exact position. If a seek occurs and it is within the
     * range of frames we keep track of a precise seek occurs, otherwise an imprecise seek is performed
     */
    typedef std::vector<SeekFrameType> SeekFrameList;
    SeekFrameList m_seekFrameList; // ordered-by frameIndex
    SINT m_avgSeekFrameCount;      // avg. sample frames per MP3 frame

    void addSeekFrame(SINT frameIndex, const unsigned char* pInputData);

    /** Returns the position in m_seekFrameList of the requested frame index. */
    SINT findSeekFrameIndex(SINT frameIndex) const;

    SINT m_curFrameIndex;

    // NOTE(uklotzde): Each invocation of initDecoding() must be
    // followed by an invocation of finishDecoding().
    void initDecoding();
    void restartDecoding(const SeekFrameType& seekFrame);
    void finishDecoding();

    // MAD decoder
    mad_stream m_madStream;
    mad_frame m_madFrame;
    mad_synth m_madSynth;

    SINT m_madSynthCount; // left overs from the previous read

    std::vector<unsigned char> m_leftoverBuffer;
};

class SoundSourceProviderMp3 : public SoundSourceProvider {
  public:
    static const QString kDisplayName;
    static const QStringList kSupportedFileExtensions;

    QString getDisplayName() const override {
        return kDisplayName;
    }

    QStringList getSupportedFileExtensions() const override {
        return kSupportedFileExtensions;
    }

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return newSoundSourceFromUrl<SoundSourceMp3>(url);
    }
};

} // namespace mixxx
