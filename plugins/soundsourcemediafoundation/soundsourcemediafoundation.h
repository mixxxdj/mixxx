/**
 * \file soundsourcemediafoundation.h
 * \class SoundSourceMediaFoundation
 * \brief Decodes MPEG4/AAC audio using the SourceReader interface of the
 * Media Foundation framework included in Windows 7.
 * \author Bill Good <bkgood at gmail dot com>
 * \author Albert Santoni <alberts at mixxx dot org>
 * \date Jan 10, 2011
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SOUNDSOURCEMEDIAFOUNDATION_H
#define SOUNDSOURCEMEDIAFOUNDATION_H

#include "sources/soundsourceplugin.h"

#include <windows.h>

class IMFSourceReader;
class IMFMediaType;
class IMFMediaSource;

namespace mixxx {

class SoundSourceMediaFoundation : public mixxx::SoundSourcePlugin {
public:
    explicit SoundSourceMediaFoundation(const QUrl& url);
    ~SoundSourceMediaFoundation() override;

    void close() override;

    SINT seekSampleFrame(SINT frameIndex) override;

    SINT readSampleFrames(SINT numberOfFrames, CSAMPLE* sampleBuffer) override;

private:
    OpenResult tryOpen(const mixxx::AudioSourceConfig& audioSrcCfg) override;

    bool configureAudioStream(const mixxx::AudioSourceConfig& audioSrcCfg);
    bool readProperties();

    void copyFrames(CSAMPLE *dest, SINT *destFrames, const CSAMPLE *src,
            SINT srcFrames);

    HRESULT m_hrCoInitialize;
    HRESULT m_hrMFStartup;
    IMFSourceReader *m_pReader;
    SINT m_nextFrame;
    CSAMPLE *m_leftoverBuffer;
    SINT m_leftoverBufferSize;
    SINT m_leftoverBufferLength;
    SINT m_leftoverBufferPosition;
    qint64 m_mfDuration;
    SINT m_iCurrentPosition;
    bool m_dead;
    bool m_seeking;
};

class SoundSourceProviderMediaFoundation: public SoundSourceProvider {
public:
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override;
};

} // namespace mixxx

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
mixxx::SoundSourceProvider* Mixxx_SoundSourcePluginAPI_createSoundSourceProvider();

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
void Mixxx_SoundSourcePluginAPI_destroySoundSourceProvider(mixxx::SoundSourceProvider*);

#endif // SOUNDSOURCEMEDIAFOUNDATION_H
