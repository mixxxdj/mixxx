// soundsourcemodplug.h  -  modplug tracker support
// created 2012 by Stefan Nuernberger <kabelfrickler@gmail.com>

#ifndef SOUNDSOURCEMODPLUG_H
#define SOUNDSOURCEMODPLUG_H

#include <QByteArray>
#include <QList>
#include <QString>

#include "soundsource.h"

namespace ModPlug {
#include <libmodplug/modplug.h>
}

// Class for reading tracker files using libmodplug.
// The whole file is decoded at once and saved
// in RAM to allow seeking and smooth operation in Mixxx.
class SoundSourceModPlug : public Mixxx::SoundSource
{
  public:
    SoundSourceModPlug(QString qFilename);
    ~SoundSourceModPlug();
    int open();
    long seek(long);
    unsigned read(unsigned long size, const SAMPLE*);
    inline long unsigned length();
    int parseHeader();
    static QList<QString> supportedFileExtensions();

    // apply settings for decoding
    static void configure(unsigned int bufferSizeLimit,
                          const ModPlug::ModPlug_Settings &settings);

  private:
    static int s_bufferSizeLimit; // max track buffer length (bytes)
    static ModPlug::ModPlug_Settings s_settings; // modplug decoder parameters

    bool m_opened;
    unsigned long m_fileLength; // length of file in samples
    unsigned long m_seekPos; // current read position
    ModPlug::ModPlugFile *m_pModFile; // modplug file descriptor
    QByteArray m_fileBuf; // original module file data
    QByteArray m_sampleBuf; // 16bit stereo samples, 44.1kHz

    // identification of modplug module type
    enum ModuleTypes {
        NONE = 0x00,
        MOD  = 0x01,
        S3M  = 0x02,
        XM   = 0x04,
        MED  = 0x08,
        IT   = 0x20,
        STM  = 0x100,
        OKT  = 0x8000
    };
};

#endif
