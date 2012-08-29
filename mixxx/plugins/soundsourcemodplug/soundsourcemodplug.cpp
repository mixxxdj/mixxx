/***************************************************************************
                          soundsourcemodplug.cpp  -  description
                             -------------------
    copyright            : (C) 2012 by Stefan Nuernberger
    email                : kabelfricker@gmail.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "soundsourcemodplug.h"
#include <stdlib.h>
#include <unistd.h>
#include <QFile>

/* read files in 256k chunks. Limit to 250 MB/file (~25min/track) */
#define BUFFERSIZE 262144
#define BUFFERSIZE_LIMIT 262144000

namespace Mixxx {

SoundSourceModPlug::SoundSourceModPlug(QString qFilename) :
    Mixxx::SoundSource(qFilename)
{
    opened = false;
    filelength = 0;
    file = 0;

    /* configure ModPlug settings */
    settings.mFlags = ModPlug::MODPLUG_ENABLE_OVERSAMPLING
                | ModPlug::MODPLUG_ENABLE_NOISE_REDUCTION
                | ModPlug::MODPLUG_ENABLE_MEGABASS;

    /* Note that ModPlug always decodes sound at 44100kHz, 32 bit, stereo and then
     * down-mixes to the settings you choose. */
    settings.mChannels = 2;        /* Number of channels - 1 for mono or 2 for stereo */
    settings.mBits = 16;           /* Bits per sample - 8, 16, or 32 */
    settings.mFrequency = 44100;   /* Sampling rate - 11025, 22050, or 44100 */
    settings.mResamplingMode = ModPlug::MODPLUG_RESAMPLE_SPLINE;

    settings.mStereoSeparation = 128; /* Stereo separation, 1 - 256 */
    settings.mMaxMixChannels = 128; /* Maximum number of mixing channels (polyphony), 32 - 256 */

    settings.mReverbDepth = 0;    /* Reverb level 0(quiet)-100(loud)      */
    settings.mReverbDelay = 50;   /* Reverb delay in ms, usually 40-200ms */
    settings.mBassAmount = 30;    /* XBass level 0(quiet)-100(loud)       */
    settings.mBassRange = 40;     /* XBass cutoff in Hz 10-100            */
    settings.mSurroundDepth = 0;  /* Surround level 0(quiet)-100(heavy)   */
    settings.mSurroundDelay = 5;  /* Surround delay in ms, usually 5-40ms */
    settings.mLoopCount = 0;      /* Number of times to loop.  Zero prevents looping.
                            -1 loops forever. */

    // apply modplug settings
    ModPlug::ModPlug_SetSettings(&settings);

    qDebug() << "Loading ModPlug module " << m_qFilename;

    // read module file to byte array
    QFile modfile(m_qFilename);
    modfile.open(QIODevice::ReadOnly);
    filebuf = modfile.readAll();
    modfile.close();
    // get ModPlugFile
    file = ModPlug::ModPlug_Load(filebuf.data(), filebuf.length());
}

SoundSourceModPlug::~SoundSourceModPlug()
{
    if (file) {
        ModPlug::ModPlug_Unload(file);
        file = NULL;
    }
}

QList<QString> SoundSourceModPlug::supportedFileExtensions()
{
    QList<QString> list;
    /* ModPlug supports more formats but file name
     * extensions are not always present with modules.
     */
    list.push_back("mod");
    list.push_back("med");
    list.push_back("okt");
    list.push_back("s3m");
    list.push_back("stm");
    list.push_back("xm");
    list.push_back("it");
    return list;
}

int SoundSourceModPlug::open() {
    if (file == NULL) {
        // an error occured
        qDebug() << "Could not load module file: "
                 << m_qFilename;
        return ERR;
    }

    // temporary buffer to read samples
    char *tmpbuf = new char[BUFFERSIZE];
    int count = 1;
    while ((count != 0) && (smplbuf.length() < BUFFERSIZE_LIMIT))
    {
        /* Read sample data into the buffer.  Returns the number of bytes read.  If the end
         * of the mod has been reached, zero is returned. */
        count = ModPlug::ModPlug_Read(file, tmpbuf, BUFFERSIZE);
        smplbuf.append(tmpbuf, count);

        qDebug() << "Filled Sample buffer with " << smplbuf.length() << " bytes.";
    }
    delete tmpbuf;

    // smplbuf holds 44.1kHz 16bit integer stereo samples.
    // We count the number of samples by dividing number of
    // bytes in smplbuf by 2 (bytes per sample).
    filelength = smplbuf.length() >> 1;
    m_iSampleRate = 44100; // ModPlug uses 44.1kHz
    opened = true;
    seekpos = 0;
    return OK;
}

long SoundSourceModPlug::seek(long filepos)
{
    unsigned long filepos2 = (unsigned long)filepos;
    if (filelength>0)
    {
        seekpos = math_min(filepos2,filelength);
        return seekpos;
    }
    return 0;
}

/*
   read <size> samples into <destination>, and return the number of
   samples actually read.
 */
unsigned SoundSourceModPlug::read(unsigned long size, const SAMPLE * destination)
{
    unsigned char * dest = (unsigned char *) destination;
    if (filelength > 0)
    {
        unsigned count = 0;
        while ((seekpos < filelength) && (count < 2*size))
        {
            dest[count++] = smplbuf[seekpos << 1];
            dest[count++] = smplbuf[(seekpos << 1) + 1];
            ++seekpos;
        }
        return count >> 1;
    }
    // The file has errors or is not open. Tell the truth and return 0.
    return 0;
}

int SoundSourceModPlug::parseHeader()
{
    if (file == NULL) {
        // an error occured
        qDebug() << "Could not parse module header of " << m_qFilename;
        return ERR;
    }

    switch (ModPlug::ModPlug_GetModuleType(file))
    {
    case 0x00: // none
        setType(QString("None"));
        break;
    case 0x01: // mod
        setType(QString("Protracker"));
        break;
    case 0x02: // s3m
        setType(QString("Scream Tracker 3"));
        break;
    case 0x04: // xm
        setType(QString("FastTracker2"));
        break;
    case 0x08: // med
        setType(QString("OctaMed"));
        break;
    case 0x20: // it
        setType(QString("Impulse Tracker"));
        break;
    case 0x100: // stm
        setType(QString("Scream Tracker"));
        break;
    case 0x8000: // okt
        setType(QString("Oktalyzer"));
        break;
    default:
        setType(QString("Module"));
        break;
    }
    setComment(QString(ModPlug::ModPlug_GetMessage(file)));
    setTitle(QString(ModPlug::ModPlug_GetName(file)));
    setDuration(ModPlug::ModPlug_GetLength(file) / 1000);
    // FIXME: BPM is always 0 when track is not running
    setBPM(ModPlug::ModPlug_GetCurrentTempo(file));
    return OK;
}

/*
   Return the length of the file in samples.
 */
inline long unsigned SoundSourceModPlug::length()
{
    return filelength;
}

} // ns Mixxx
