/**
 * \file SoundSourceFLAC.cpp
 * \author Bill Good <bkgood at gmail dot com>
 * \date May 22, 2010
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "soundsourceflac.h"

SoundSourceFLAC::SoundSourceFLAC(QString fileName)
    : m_qFileName(fileName)
      SRATE(0)
      m_channels(0)
      m_samples(0)
      m_bps(0) {
    QByteArray fileNameBytes(fileName.toUtf8());
    m_decoder = FLAC__stream_decoder_new();
    if (m_decoder == NULL) return;
    
}

SoundSourceFLAC::~SoundSourceFLAC() {
    if (m_decoder) {
        FLAC__stream_decoder_finish(m_decoder);
        FLAC__stream_decoder_delete(m_decoder);
    }
}

long SoundSourceFLAC::seek(long) {

}

unsigned SoundSourceFLAC::read(unsigned long size, const SAMPLE*) {

}

inline long unsigned SoundSourceFLAC::length() {
    
}

static int SoundSourceFLAC::ParseHeader(TrackInfoObject*) {

}