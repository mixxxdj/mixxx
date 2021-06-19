#include "encoder/encoder.h"

#include <QList>

#include "preferences/usersettings.h"
#include "recording/defs_recording.h"
// TODO(XXX): __FFMPEGFILE_ENCODERS__ is currently undefined because
// FFMPEG encoders provide less features than the other encoders and currently
// we don't have a good fallback for using them. That's why it's a bad idea
// to have the worse encoders to take precedence over the good ones.
#ifdef __FFMPEGFILE_ENCODERS__
#include "encoder/encoderffmpegmp3.h"
#include "encoder/encoderffmpegvorbis.h"
#else
#include "encoder/encodermp3.h"
#include "encoder/encodervorbis.h"
#endif
#include "encoder/encoderwave.h"
#include "encoder/encodersndfileflac.h"

#include "encoder/encodermp3settings.h"
#include "encoder/encodervorbissettings.h"
#include "encoder/encoderwavesettings.h"
#include "encoder/encoderflacsettings.h"

#ifdef __OPUS__
#include "encoder/encoderopus.h"
#include "encoder/encoderopussettings.h"
#endif

#include "encoder/encoderfdkaac.h"
#include "encoder/encoderfdkaacsettings.h"

EncoderFactory EncoderFactory::factory;

const EncoderFactory& EncoderFactory::getFactory()
{
    return factory;
}

EncoderFactory::EncoderFactory() {
    // Add new supported formats here. Also modify the getNewEncoder/getEncoderSettings method.
    m_formats.append(Encoder::Format("WAV PCM", ENCODING_WAVE, true, "wav"));
    m_formats.append(Encoder::Format("AIFF PCM", ENCODING_AIFF, true, "aiff"));
    m_formats.append(Encoder::Format("FLAC", ENCODING_FLAC, true, "flac"));
    m_formats.append(Encoder::Format("MP3", ENCODING_MP3, false, "mp3"));
    m_formats.append(Encoder::Format("OGG Vorbis", ENCODING_OGG, false, "ogg"));
#ifdef __OPUS__
    m_formats.append(Encoder::Format("Opus", ENCODING_OPUS, false, "opus"));
#endif
    m_formats.append(Encoder::Format("AAC", ENCODING_AAC, false, "aac"));
    m_formats.append(Encoder::Format("HE-AAC", ENCODING_HEAAC, false, "aac"));
    m_formats.append(Encoder::Format("HE-AACv2", ENCODING_HEAACV2, false, "aac"));
}

const QList<Encoder::Format> EncoderFactory::getFormats() const
{
    return m_formats;
}

Encoder::Format EncoderFactory::getSelectedFormat(UserSettingsPointer pConfig) const
{
    return getFormatFor(pConfig->getValueString(ConfigKey(RECORDING_PREF_KEY, "Encoding")));
}
Encoder::Format EncoderFactory::getFormatFor(const QString& formatText) const {
    for (const auto& format : m_formats) {
        if (format.internalName == formatText) {
            return format;
        }
    }
    qWarning() << "Format: " << formatText << " not recognized! Returning format "
        << m_formats.first().internalName;
    return m_formats.first();
}

EncoderPointer EncoderFactory::createRecordingEncoder(
        const Encoder::Format& format,
        UserSettingsPointer pConfig,
        EncoderCallback* pCallback) const {
    EncoderRecordingSettingsPointer pSettings =
            getEncoderRecordingSettings(format, pConfig);
    return createEncoder(pSettings, pCallback);
}

EncoderPointer EncoderFactory::createEncoder(
        EncoderSettingsPointer pSettings,
        EncoderCallback* pCallback) const {
    EncoderPointer pEncoder;
    if (pSettings && pSettings->getFormat() == ENCODING_WAVE) {
        pEncoder = std::make_shared<EncoderWave>(pCallback);
        pEncoder->setEncoderSettings(*pSettings);
    } else if (pSettings && pSettings->getFormat() == ENCODING_AIFF) {
        pEncoder = std::make_shared<EncoderWave>(pCallback);
        pEncoder->setEncoderSettings(*pSettings);
    } else if (pSettings && pSettings->getFormat() == ENCODING_FLAC) {
        pEncoder = std::make_shared<EncoderSndfileFlac>(pCallback);
        pEncoder->setEncoderSettings(*pSettings);
    } else if (pSettings && pSettings->getFormat() == ENCODING_MP3) {
#ifdef __FFMPEGFILE_ENCODERS__
        pEncoder = std::make_shared<EncoderFfmpegMp3>(pCallback);
#else
        pEncoder = std::make_shared<EncoderMp3>(pCallback);
#endif
        pEncoder->setEncoderSettings(*pSettings);
    } else if (pSettings && pSettings->getFormat() == ENCODING_OGG) {
#ifdef __FFMPEGFILE_ENCODERS__
        pEncoder = std::make_shared<EncoderFfmpegVorbis>(pCallback);
#else
        pEncoder = std::make_shared<EncoderVorbis>(pCallback);
#endif
        pEncoder->setEncoderSettings(*pSettings);
#ifdef __OPUS__
    } else if (pSettings && pSettings->getFormat() == ENCODING_OPUS) {
        pEncoder = std::make_shared<EncoderOpus>(pCallback);
        pEncoder->setEncoderSettings(*pSettings);
#endif
    } else if (pSettings &&
            (pSettings->getFormat() == ENCODING_AAC ||
                    pSettings->getFormat() == ENCODING_HEAAC ||
                    pSettings->getFormat() == ENCODING_HEAACV2)) {
        pEncoder = std::make_shared<EncoderFdkAac>(pCallback);
        pEncoder->setEncoderSettings(*pSettings);
    } else {
        qWarning() << "Unsupported format requested! "
                << QString(pSettings ? pSettings->getFormat() : QString("NULL"));
        DEBUG_ASSERT(false);
        pEncoder = std::make_shared<EncoderWave>(pCallback);
    }
    return pEncoder;
}

EncoderRecordingSettingsPointer EncoderFactory::getEncoderRecordingSettings(Encoder::Format format,
        UserSettingsPointer pConfig) const {
    if (format.internalName == ENCODING_WAVE) {
        return std::make_shared<EncoderWaveSettings>(pConfig, format.internalName);
    } else if (format.internalName == ENCODING_AIFF) {
        return std::make_shared<EncoderWaveSettings>(pConfig, format.internalName);
    } else if (format.internalName == ENCODING_FLAC) {
        return std::make_shared<EncoderFlacSettings>(pConfig);
    } else if (format.internalName == ENCODING_MP3) {
        return std::make_shared<EncoderMp3Settings>(pConfig);
    } else if (format.internalName == ENCODING_OGG) {
        return std::make_shared<EncoderVorbisSettings>(pConfig);
#ifdef __OPUS__
    } else if (format.internalName == ENCODING_OPUS) {
        return std::make_shared<EncoderOpusSettings>(pConfig);
#endif
    } else if (format.internalName == ENCODING_AAC ||
            format.internalName == ENCODING_HEAAC ||
            format.internalName == ENCODING_HEAACV2) {
        return std::make_shared<EncoderFdkAacSettings>(pConfig, format.internalName);
    } else {
        qWarning() << "Unsupported format requested! " << format.internalName;
        DEBUG_ASSERT(false);
        return std::make_shared<EncoderWaveSettings>(pConfig, ENCODING_WAVE);
    }
}
