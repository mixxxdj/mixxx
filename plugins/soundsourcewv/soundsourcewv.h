#ifndef SOUNDSOURCEWV_H
#define SOUNDSOURCEWV_H

#include "sources/soundsource.h"
#include "defs_version.h"

#ifdef Q_OS_WIN
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT
#endif

namespace Mixxx {

class SoundSourceWV: public SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceWV(QString fileName);

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;
    QImage parseCoverArt() const /*override*/;

    Mixxx::AudioSourcePointer open() const /*override*/;
};

}  // namespace Mixxx

extern "C" MY_EXPORT const char* getMixxxVersion();
extern "C" MY_EXPORT int getSoundSourceAPIVersion();
extern "C" MY_EXPORT Mixxx::SoundSource* getSoundSource(QString fileName);
extern "C" MY_EXPORT char** supportedFileExtensions();
extern "C" MY_EXPORT void freeFileExtensions(char **exts);

#endif
