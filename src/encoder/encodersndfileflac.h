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

#ifndef ENCODERSNDFILEFLAC_H
#define ENCODERSNDFILEFLAC_H


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
#include "track/track.h"

class EncoderCallback;

class EncoderSndfileFlac : public EncoderWave {
  public:
    EncoderSndfileFlac(EncoderCallback* pCallback=nullptr);
    virtual ~EncoderSndfileFlac();

    void setEncoderSettings(const EncoderSettings& settings) override;
    
  protected:
    void initStream() override;
  private:
    double m_compression;
};

#endif //ENCODERWAVE_H
