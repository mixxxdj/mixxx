#ifndef MIXXX_SOUNDSOURCEWV_H
#define MIXXX_SOUNDSOURCEWV_H

#include "sources/soundsourceplugin.h"

#include "wavpack/wavpack.h"

namespace Mixxx {

class SoundSourceWV: public SoundSourcePlugin {
public:
    explicit SoundSourceWV(const QUrl& url);
    ~SoundSourceWV();

    void close() /*override*/;

    SINT seekSampleFrame(SINT frameIndex) /*override*/;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) /*override*/;

private:
    Result tryOpen(const AudioSourceConfig& audioSrcCfg) /*override*/;

    WavpackContext* m_wpc;

    CSAMPLE m_sampleScaleFactor;
};

}  // namespace Mixxx

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT QVector<QString> Mixxx_SoundSourcePluginAPI_getSupportedFileTypes();
extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT Mixxx::SoundSourcePointer Mixxx_SoundSourcePluginAPI_newSoundSource(const QUrl& url);

#endif // MIXXX_SOUNDSOURCEWV_H
