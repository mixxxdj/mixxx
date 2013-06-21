//soundsourcewv.h
// wavpack sound proxy for mixxx.
// fenugrec 12/2009


#ifndef SOUNDSOURCEWV_H
#define SOUNDSOURCEWV_H

#include <QString>
#include "soundsource.h"
#include "defs_version.h"

#include "wavpack/wavpack.h"

#ifdef Q_WS_WIN
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT
#endif

#define WV_BUF_LENGTH 65536

namespace Mixxx {

class SoundSourceWV : public SoundSource {
 public:
  SoundSourceWV(QString qFilename);
  ~SoundSourceWV();
  int open();
  long seek(long);
  unsigned read(unsigned long size, const SAMPLE*);
  inline long unsigned length();
  int parseHeader();
  static QList<QString> supportedFileExtensions();
 private:
  int Bps;
  unsigned long filelength;
  WavpackContext * filewvc;	//works as a file handle to access the wv file.
  int32_t tempbuffer[WV_BUF_LENGTH];	//hax ! legacy from cmus. this is 64k*4bytes.
  void format_samples(int, char *, int32_t *, uint32_t);
};


extern "C" MY_EXPORT const char* getMixxxVersion()
{
    return VERSION;
}

extern "C" MY_EXPORT int getSoundSourceAPIVersion()
{
    return MIXXX_SOUNDSOURCE_API_VERSION;
}

extern "C" MY_EXPORT SoundSource* getSoundSource(QString filename)
{
    return new SoundSourceWV(filename);
}

extern "C" MY_EXPORT char** supportedFileExtensions()
{
    QList<QString> exts = SoundSourceWV::supportedFileExtensions();
    //Convert to C string array.
    char** c_exts = (char**)malloc((exts.count() + 1) * sizeof(char*));
    for (int i = 0; i < exts.count(); i++)
    {
        QByteArray qba = exts[i].toUtf8();
        c_exts[i] = strdup(qba.constData());
        qDebug() << c_exts[i];
    }
    c_exts[exts.count()] = NULL; //NULL terminate the list

    return c_exts;
}

extern "C" MY_EXPORT void freeFileExtensions(char **exts)
{
    for (int i(0); exts[i]; ++i) free(exts[i]);
    free(exts);
}

}  // namespace Mixxx

#endif
