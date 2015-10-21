#ifndef MIXXX_SOUNDSOURCEWV_H
#define MIXXX_SOUNDSOURCEWV_H

#include "sources/soundsourceplugin.h"

#include "wavpack/wavpack.h"

class QFile;

namespace Mixxx {

class SoundSourceWV: public SoundSourcePlugin {
  public:
    explicit SoundSourceWV(const QUrl& url);
    ~SoundSourceWV();

    void close() override;

    SINT seekSampleFrame(SINT frameIndex) override;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) override;

  private:
    Result tryOpen(const AudioSourceConfig& audioSrcCfg) override;

    static int32_t ReadBytesCallback(void* id, void* data, int bcount);
    static uint32_t GetPosCallback(void* id);
    static int SetPosAbsCallback(void* id, unsigned int pos);
    static int SetPosRelCallback(void* id, int delta, int mode);
    static int PushBackByteCallback(void* id, int c);
    static uint32_t GetlengthCallback(void* id);
    static int CanSeekCallback(void* id);
    static int32_t WriteBytesCallback(void* id, void* data, int32_t bcount);
    static WavpackStreamReader s_streamReader;

    WavpackContext* m_wpc;
    CSAMPLE m_sampleScaleFactor;
    QFile* m_pWVFile;
    QFile* m_pWVCFile;
};

class SoundSourceProviderWV: public SoundSourceProvider {
public:
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override;
};

}  // namespace Mixxx

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
Mixxx::SoundSourceProvider* Mixxx_SoundSourcePluginAPI_createSoundSourceProvider();

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
void Mixxx_SoundSourcePluginAPI_destroySoundSourceProvider(Mixxx::SoundSourceProvider*);

#endif // MIXXX_SOUNDSOURCEWV_H
