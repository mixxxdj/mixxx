#ifndef SOUNDSOURCEWV_H
#define SOUNDSOURCEWV_H

#include "defs_version.h"
#include "soundsource.h"

#include "wavpack/wavpack.h"

#ifdef Q_OS_WIN
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT
#endif

#define WV_BUF_LENGTH 65536

namespace Mixxx {

class SoundSourceWV: public SoundSource {
    typedef SoundSource Super;

public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceWV(QString qFilename);
    ~SoundSourceWV();

    Result parseHeader();
    QImage parseCoverArt();

    Result open();

    diff_type seekFrame(diff_type frameIndex) /*override*/;

    size_type readFrameSamplesInterleaved(size_type frameCount,
            sample_type* sampleBuffer) /*override*/;

private:
    WavpackContext* m_wpc;

    sample_type m_sampleScale;
};

extern "C" MY_EXPORT const char* getMixxxVersion() {
    return VERSION;
}

extern "C" MY_EXPORT int getSoundSourceAPIVersion() {
    return MIXXX_SOUNDSOURCE_API_VERSION;
}

extern "C" MY_EXPORT SoundSource* getSoundSource(QString filename) {
    return new SoundSourceWV(filename);
}

extern "C" MY_EXPORT char** supportedFileExtensions() {
    QList<QString> exts = SoundSourceWV::supportedFileExtensions();
    //Convert to C string array.
    char** c_exts = (char**) malloc((exts.count() + 1) * sizeof(char*));
    for (int i = 0; i < exts.count(); i++) {
        QByteArray qba = exts[i].toUtf8();
        c_exts[i] = strdup(qba.constData());
        qDebug() << c_exts[i];
    }
    c_exts[exts.count()] = NULL; //NULL terminate the list

    return c_exts;
}

extern "C" MY_EXPORT void freeFileExtensions(char **exts) {
    for (int i(0); exts[i]; ++i)
        free(exts[i]);
    free(exts);
}

}  // namespace Mixxx

#endif
