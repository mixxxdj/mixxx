/***************************************************************************
                          soundsourcemodplug.cpp  -  module tracker support
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

/* read files in 512k chunks */
#define CHUNKSIZE 524288

// reserve some static space for settings...
ModPlug::ModPlug_Settings SoundSourceModPlug::settings;
unsigned int SoundSourceModPlug::bufferSizeLimit;

SoundSourceModPlug::SoundSourceModPlug(QString qFilename) :
    SoundSource(qFilename)
{
    opened = false;
    filelength = 0;
    file = 0;

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

void SoundSourceModPlug::configure(unsigned int bLimit, const ModPlug::ModPlug_Settings &config)
{
    bufferSizeLimit = bLimit;
    settings = config;

    ModPlug::ModPlug_SetSettings(&settings);
}

int SoundSourceModPlug::open() {
    if (file == NULL) {
        // an error occured
        qDebug() << "Could not load module file: "
                 << m_qFilename;
        return ERR;
    }

    // temporary buffer to read samples
    char *tmpbuf = new char[CHUNKSIZE];
    int count = 1;
    while ((count != 0) && (smplbuf.length() < bufferSizeLimit))
    {
        /* Read sample data into the buffer.  Returns the number of bytes read.  If the end
         * of the module has been reached, zero is returned. */
        count = ModPlug::ModPlug_Read(file, tmpbuf, CHUNKSIZE);
        smplbuf.append(tmpbuf, count);
    }
    delete tmpbuf;
    qDebug() << "Filled Sample buffer with " << smplbuf.length() << " bytes.";

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
    if (filelength > 0)
    {
        seekpos = math_min((unsigned long)filepos, filelength);
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
        while ((seekpos < filelength) && (count < (size << 1 )))
        {
            // copy a 16bit sample
            dest[count++] = smplbuf[seekpos << 1];
            dest[count++] = smplbuf[(seekpos << 1) + 1];
            ++seekpos;
        }
        return count >> 1; ///< number of bytes divided by bytes per sample
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
    case NONE:
        setType(QString("None"));
        break;
    case MOD:
        setType(QString("Protracker"));
        break;
    case S3M:
        setType(QString("Scream Tracker 3"));
        break;
    case XM:
        setType(QString("FastTracker2"));
        break;
    case MED:
        setType(QString("OctaMed"));
        break;
    case IT:
        setType(QString("Impulse Tracker"));
        break;
    case STM:
        setType(QString("Scream Tracker"));
        break;
    case OKT:
        setType(QString("Oktalyzer"));
        break;
    default:
        setType(QString("Module"));
        break;
    }
    setComment(QString(ModPlug::ModPlug_GetMessage(file)));
    setTitle(QString(ModPlug::ModPlug_GetName(file)));
    setDuration(ModPlug::ModPlug_GetLength(file) / 1000);
    setBitrate(8); // not really, but fill in something...
    setSampleRate(44100);
    setChannels(2);
    return OK;
}

/*
   Return the length of the file in samples.
 */
inline long unsigned SoundSourceModPlug::length()
{
    return filelength;
}
