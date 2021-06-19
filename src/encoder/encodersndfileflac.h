#pragma once

#include "encoder/encoderflacsettings.h"
#ifdef Q_OS_WIN
//Enable unicode in libsndfile on Windows
//(sf_open uses UTF-8 otherwise)
#include <windows.h>
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#endif
#include <sndfile.h>

#include "util/types.h"
#include "encoder/encoderwave.h"

class EncoderCallback;

/// Encoder for FLAC using libsndfile
class EncoderSndfileFlac : public EncoderWave {
  public:
    EncoderSndfileFlac(EncoderCallback* pCallback = nullptr);
    ~EncoderSndfileFlac() override = default;

    void setEncoderSettings(const EncoderSettings& settings) override;
    void encodeBuffer(const CSAMPLE* samples, const int size) override;

  protected:
    void initStream() override;
  private:
    double m_compression;
    std::unique_ptr<int[]> m_pClampBuffer;
};
