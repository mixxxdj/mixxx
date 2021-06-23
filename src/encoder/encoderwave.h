#pragma once

#include "encoder/encoderwavesettings.h"
#ifdef Q_OS_WIN
// Enable unicode in libsndfile on Windows
// (sf_open uses UTF-8 otherwise)
#include <windows.h>
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#endif
#include <sndfile.h>

#include "encoder/encoder.h"
#include "track/track_decl.h"
#include "util/types.h"

class EncoderCallback;

// WAVE/AIFF "encoder"
class EncoderWave : public Encoder {
  public:
    EncoderWave(EncoderCallback* pCallback = nullptr);
    ~EncoderWave() override;

    int initEncoder(int samplerate, QString* pUserErrorMessage) override;
    void encodeBuffer(const CSAMPLE *samples, const int size) override;
    void updateMetaData(const QString& artist, const QString& title, const QString& album) override;
    void flush() override;
    void setEncoderSettings(const EncoderSettings& settings) override;

  protected:
    virtual void initStream();
    TrackPointer m_pMetaData;
    EncoderCallback* m_pCallback;
    QString m_metaDataTitle;
    QString m_metaDataArtist;
    QString m_metaDataAlbum;

    SNDFILE* m_pSndfile;
    SF_INFO m_sfInfo;

    SF_VIRTUAL_IO m_virtualIo;
};
