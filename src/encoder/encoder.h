#pragma once

#include "util/memory.h"
#include "util/types.h"
#include "preferences/usersettings.h"
#include "encoder/encodersettings.h"
#include "encoder/encoderrecordingsettings.h"
#include "encoder/encodercallback.h"

class Encoder {
  public:
        class Format {
            public:
              Format(const QString& labelIn,
                      const QString& nameIn,
                      bool losslessIn,
                      const QString& fileExtIn)
                      : label(labelIn),
                        internalName(nameIn),
                        lossless(losslessIn),
                        fileExtension(fileExtIn) {
              }
            QString label;
            QString internalName;
            bool lossless;
            QString fileExtension;
        };

    Encoder() {}
    virtual ~Encoder() = default;

    virtual int initEncoder(int samplerate, QString* pUserErrorMessage) = 0;
    // encodes the provided buffer of audio.
    virtual void encodeBuffer(const CSAMPLE *samples, const int size) = 0;
    // Adds metadata to the encoded audio, i.e., the ID3 tag. Currently only used
    // by EngineRecord, ShoutConnection does something different.
    virtual void updateMetaData(const QString& artist, const QString& title, const QString& album) = 0;
    // called at the end when encoding is finished
    virtual void flush() = 0;
    // Setup the encoder with the specific settings
    virtual void setEncoderSettings(const EncoderSettings& settings) = 0;
};

typedef std::shared_ptr<Encoder> EncoderPointer;

class EncoderFactory {
  private:
    EncoderFactory();
  public:
    static const EncoderFactory& getFactory();

    const QList<Encoder::Format> getFormats() const;
    Encoder::Format getSelectedFormat(UserSettingsPointer pConfig) const;
    Encoder::Format getFormatFor(const QString& format) const;
    EncoderPointer createRecordingEncoder(
            const Encoder::Format& format,
            UserSettingsPointer pConfig,
            EncoderCallback* pCallback) const;
    EncoderPointer createEncoder(
            EncoderSettingsPointer pSettings,
            EncoderCallback* pCallback) const;
    EncoderRecordingSettingsPointer getEncoderRecordingSettings(
            Encoder::Format format,
            UserSettingsPointer pConfig) const;
  private:
    static EncoderFactory factory;
    QList<Encoder::Format> m_formats;
};
