#ifndef SOUNDSOURCEMODPLUG_H
#define SOUNDSOURCEMODPLUG_H

#include "soundsource.h"

namespace ModPlug {
#include <libmodplug/modplug.h>
}

#include <vector>

// Class for reading tracker files using libmodplug.
// The whole file is decoded at once and saved
// in RAM to allow seeking and smooth operation in Mixxx.
class SoundSourceModPlug: public Mixxx::SoundSource {
    typedef SoundSource Super;

public:
    static QList<QString> supportedFileExtensions();

    // apply settings for decoding
    static void configure(unsigned int bufferSizeLimit,
            const ModPlug::ModPlug_Settings &settings);

    explicit SoundSourceModPlug(QString qFilename);
    ~SoundSourceModPlug();

    Result parseHeader() /*override*/;
    QImage parseCoverArt() /*override*/;

    Result open() /*override*/;

    diff_type seekFrame(diff_type frameIndex) /*override*/;
    size_type readFrameSamplesInterleaved(size_type frameCount,
            sample_type* sampleBuffer) /*override*/;

private:
    ModPlug::ModPlugFile *m_pModFile; // modplug file descriptor
    unsigned long m_fileLength; // length of file in samples
    unsigned long m_seekPos; // current read position
    QByteArray m_fileBuf; // original module file data
    std::vector<SAMPLE> m_sampleBuf; // 16bit stereo samples, 44.1kHz

    // identification of modplug module type
    enum ModuleTypes {
        NONE = 0x00,
        MOD = 0x01,
        S3M = 0x02,
        XM = 0x04,
        MED = 0x08,
        IT = 0x20,
        STM = 0x100,
        OKT = 0x8000
    };
};

#endif
