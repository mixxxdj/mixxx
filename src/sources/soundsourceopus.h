#ifndef MIXXX_SOUNDSOURCEOPUS_H
#define MIXXX_SOUNDSOURCEOPUS_H

#include "sources/soundsourceprovider.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <opus/opusfile.h>

namespace Mixxx {

class SoundSourceOpus: public Mixxx::SoundSource {
<<<<<<< HEAD
    typedef SoundSource Super;

<<<<<<< HEAD
class SoundSourceOpus : public Mixxx::SoundSource {
  public:
    explicit SoundSourceOpus(QString qFilename);
    virtual ~SoundSourceOpus();

    Result open();
    long seek(long);
    unsigned read(unsigned long size, const SAMPLE*);
    inline long unsigned length();
    Result parseHeader();
    QImage parseCoverArt();
    static QList<QString> supportedFileExtensions();

  private:
    OggOpusFile *m_ptrOpusFile;
    quint64 m_lFilelength;
=======
=======
>>>>>>> Delete typedef Super (review comments)
public:
    static const SINT kFrameRate;

    explicit SoundSourceOpus(QUrl url);
    ~SoundSourceOpus();

    Result parseTrackMetadataAndCoverArt(
            TrackMetadata* pTrackMetadata,
            QImage* pCoverArt) const /*override*/;

    void close() /*override*/;

<<<<<<< HEAD
<<<<<<< HEAD
    OggOpusFile *m_pOggOpusFile;
>>>>>>> New SoundSource/AudioSource API
=======
    Mixxx::AudioSourcePointer open() const /*override*/;
>>>>>>> Split AudioSource from SoundSource
=======
    SINT seekSampleFrame(SINT frameIndex) /*override*/;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) /*override*/;
    SINT readSampleFramesStereo(SINT numberOfFrames,
            CSAMPLE* sampleBuffer, SINT sampleBufferSize) /*override*/;

private:
    Result tryOpen(const AudioSourceConfig& audioSrcCfg) /*override*/;

    OggOpusFile *m_pOggOpusFile;

    SINT m_curFrameIndex;
>>>>>>> Move code from specialized AudioSources back into corresponding SoundSources
};

class SoundSourceProviderOpus: public SoundSourceProvider {
public:
    QString getName() const /*override*/;

    QStringList getSupportedFileExtensions() const /*override*/;

    SoundSourcePointer newSoundSource(const QUrl& url)  /*override*/ {
        return SoundSourcePointer(new SoundSourceOpus(url));
    }
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEOPUS_H
