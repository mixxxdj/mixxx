/***************************************************************************
                     encodervorbis.h  -  vorbis encoder for mixxx
                             -------------------
    copyright            : (C) 2007 by Wesley Stessens
                           (C) 1994 by Xiph.org (encoder example)
                           (C) 1994 Tobias Rafreider (broadcast and recording fixes)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENCODERWAVE_H
#define ENCODERWAVE_H


#include "encoder/encoderwavesettings.h"
#ifdef Q_OS_WIN
//Enable unicode in libsndfile on Windows
//(sf_open uses UTF-8 otherwise)
#include <windows.h>
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#endif
#include <sndfile.h>

#include "util/types.h"
#include "encoder/encoder.h"
#include "track/track.h"

class EncoderCallback;

class EncoderWave : public Encoder {
  public:
    EncoderWave(EncoderCallback* pCallback=nullptr);
    virtual ~EncoderWave();

    int initEncoder(int samplerate, QString errorMessage) override;
    void encodeBuffer(const CSAMPLE *samples, const int size) override;
    void updateMetaData(const char* artist, const char* title, const char* album) override;
    void flush() override;
    void setEncoderSettings(const EncoderSettings& settings) override;

  protected:
    virtual void initStream();
    TrackPointer m_pMetaData;
    EncoderCallback* m_pCallback;
    const char* m_metaDataTitle;
    const char* m_metaDataArtist;
    const char* m_metaDataAlbum;

    SNDFILE* m_pSndfile;
    SF_INFO m_sfInfo;

    SF_VIRTUAL_IO m_virtualIo;
};

#endif //ENCODERWAVE_H
