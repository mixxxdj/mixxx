#include <stdlib.h>
#include <unistd.h>

#include <QFile>
#include <QtDebug>

#include "soundsourcemodplug.h"

#include "util/timer.h"

/* read files in 512k chunks */
#define CHUNKSIZE (1 << 19)

// reserve some static space for settings...
ModPlug::ModPlug_Settings SoundSourceModPlug::s_settings;
int SoundSourceModPlug::s_bufferSizeLimit;

SoundSourceModPlug::SoundSourceModPlug(QString qFilename) :
    SoundSource(qFilename)
{
    m_opened = false;
    m_fileLength = 0;
    m_pModFile = 0;

    qDebug() << "[ModPlug] Loading ModPlug module " << getFilename();

    // read module file to byte array
    QFile modFile(getFilename());
    modFile.open(QIODevice::ReadOnly);
    m_fileBuf = modFile.readAll();
    modFile.close();
    // get ModPlugFile descriptor for later access
    m_pModFile = ModPlug::ModPlug_Load(m_fileBuf.constData(), m_fileBuf.length());

    if (m_pModFile==NULL) {
        qDebug() << "[ModPlug] Error while ModPlug_Load";
    }
}

SoundSourceModPlug::~SoundSourceModPlug()
{
    if (m_pModFile) {
        ModPlug::ModPlug_Unload(m_pModFile);
        m_pModFile = NULL;
    }
}

QList<QString> SoundSourceModPlug::supportedFileExtensions()
{
    QList<QString> list;
    // ModPlug supports more formats but file name
    // extensions are not always present with modules.
    list.push_back("mod");
    list.push_back("med");
    list.push_back("okt");
    list.push_back("s3m");
    list.push_back("stm");
    list.push_back("xm");
    list.push_back("it");
    return list;
}

void SoundSourceModPlug::configure(unsigned int bufferSizeLimit,
                                   const ModPlug::ModPlug_Settings &settings)
{
    s_bufferSizeLimit = bufferSizeLimit;
    s_settings = settings;

    ModPlug::ModPlug_SetSettings(&s_settings);
}

Result SoundSourceModPlug::open() {
    ScopedTimer t("SoundSourceModPlug::open()");

    if (m_pModFile == NULL) {
        // an error occured
        t.cancel();
        qDebug() << "[ModPlug] Could not load module file: "
                 << getFilename();
        return ERR;
    }

    // estimate size of sample buffer (for better performance)
    // beware: module length estimation is unreliable due to loops
    // song milliseconds * 2 (bytes per sample)
    //                   * 2 (channels)
    //                   * 44.1 (samples per millisecond)
    //                   + some more to accomodate short loops etc.
    // approximate and align with CHUNKSIZE yields:
    // (((milliseconds << 2) >> 10 /* to seconds */)
    //      div 11 /* samples to chunksize ratio */)
    //      << 19 /* align to chunksize */
    int estimate = ((ModPlug::ModPlug_GetLength(m_pModFile) >> 8) / 11) << 19;
    estimate = math_min(estimate, s_bufferSizeLimit);
    m_sampleBuf.reserve(estimate);
    qDebug() << "[ModPlug] Reserved " << m_sampleBuf.capacity()
             << " bytes for samples";

    // decode samples to sample buffer
    int bytesRead = -1;
    int currentSize = 0;
    while((bytesRead != 0) && (m_sampleBuf.length() < s_bufferSizeLimit)) {
        // reserve enough space in sample buffer
        m_sampleBuf.resize(currentSize + CHUNKSIZE);
        bytesRead = ModPlug::ModPlug_Read(m_pModFile,
                                          m_sampleBuf.data() + currentSize,
                                          CHUNKSIZE);
        // adapt to actual size
        currentSize += bytesRead;
        if (bytesRead != CHUNKSIZE) {
            m_sampleBuf.resize(currentSize);
            bytesRead = 0; // we reached the end of the file
        }
    }
    qDebug() << "[ModPlug] Filled Sample buffer with " << m_sampleBuf.length()
             << " bytes.";
    qDebug() << "[ModPlug] Sample buffer has "
             << m_sampleBuf.capacity() - m_sampleBuf.length()
             << " bytes unused capacity.";

    // The sample buffer holds 44.1kHz 16bit integer stereo samples.
    // We count the number of samples by dividing number of
    // bytes in m_sampleBuf by 2 (bytes per sample).
    m_fileLength = m_sampleBuf.length() >> 1;
    setSampleRate(44100); // ModPlug always uses 44.1kHz
    m_opened = true;
    m_seekPos = 0;
    return OK;
}

long SoundSourceModPlug::seek(long filePos)
{
    if (m_fileLength > 0) {
        m_seekPos = math_min((unsigned long)filePos, m_fileLength);
        return m_seekPos;
    }
    return 0;
}

unsigned SoundSourceModPlug::read(unsigned long size,
                                  const SAMPLE* pDestination)
{
    unsigned maxLength = m_sampleBuf.length() >> 1;
    unsigned copySamples = math_min(maxLength - m_seekPos, size);

    memcpy((unsigned char*) pDestination,
           m_sampleBuf.constData() + (m_seekPos << 1), copySamples << 1);

    m_seekPos += copySamples;
    return copySamples;
}

Result SoundSourceModPlug::parseHeader() {
    if (m_pModFile == NULL) {
        // an error occured
        qDebug() << "Could not parse module header of " << getFilename();
        return ERR;
    }

    switch (ModPlug::ModPlug_GetModuleType(m_pModFile)) {
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

    setComment(QString(ModPlug::ModPlug_GetMessage(m_pModFile)));
    setTitle(QString(ModPlug::ModPlug_GetName(m_pModFile)));
    setDuration(ModPlug::ModPlug_GetLength(m_pModFile) / 1000);
    setBitrate(8); // not really, but fill in something...
    setSampleRate(44100);
    setChannels(2);
    return OK;
}

QImage SoundSourceModPlug::parseCoverArt() {
    // The modplug library currently does not support reading cover-art from
    // modplug files -- kain88 (Oct 2014)
    return QImage();
}

inline long unsigned SoundSourceModPlug::length()
{
    return m_fileLength;
}
