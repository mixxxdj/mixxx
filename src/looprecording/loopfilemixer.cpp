// loopfilemixer.cpp
// Create by Carl Pillot on 8/22/13

//#include <QDebug>
//#include <QDir>
//#include <QFile>
//#include <QMutex>
#include <QObject>
#include <QDebug>
#include <sndfile.h>

#include "looprecording/loopfilemixer.h"

#include "recording/defs_recording.h"
#include "sampleutil.h"

#define WORK_BUFFER_SIZE 16384

LoopFileMixer::LoopFileMixer(QString file1, QString file2, QString dest, QString encoding)
        : m_filePath1(file1),
        m_filePath2(file2),
        m_dest(dest),
        m_encoding(encoding) {

    m_pWorkBufferIn1 = SampleUtil::alloc(WORK_BUFFER_SIZE);
    m_pWorkBufferIn2 = SampleUtil::alloc(WORK_BUFFER_SIZE);
    m_pWorkBufferOut = SampleUtil::alloc(WORK_BUFFER_SIZE);

    qDebug() << "Mix file.  File 1: " << m_filePath1 << " file2: " << m_filePath2 << " dest: " << dest
            << " Encoding " << encoding;
}

LoopFileMixer::~LoopFileMixer() {
    qDebug() << "~LoopFileMixer()";
    SampleUtil::free(m_pWorkBufferIn1);
    SampleUtil::free(m_pWorkBufferIn2);
    SampleUtil::free(m_pWorkBufferOut);
}

void LoopFileMixer::slotProcess() {

    qDebug() << "!~!~! LoopFileMixer::slotProcess() !~!~!~!";

    SF_INFO sfInfo1;
    SF_INFO sfInfo2;
    SF_INFO sfOutInfo;

    sfInfo1.format = 0;
    sfInfo2.format = 0;

    // TODO: don't hardcode samplerate
    sfOutInfo.samplerate = 44100;
    sfOutInfo.channels = 2;

    if (m_encoding == ENCODING_WAVE) {
        sfOutInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    } else {
        sfOutInfo.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16;
    }

    SNDFILE* pSndfile1 = sf_open(m_filePath1.toLocal8Bit(), SFM_READ, &sfInfo1);
    SNDFILE* pSndfile2 = sf_open(m_filePath2.toLocal8Bit(), SFM_READ, &sfInfo2);
    SNDFILE* pSndfileOut = sf_open(m_filePath2.toLocal8Bit(), SFM_WRITE, &sfOutInfo);

    if (pSndfile1 && pSndfile2 && pSndfileOut) {

        sf_command(pSndfile1, SFC_SET_NORM_FLOAT, NULL, SF_FALSE);
        sf_command(pSndfile2, SFC_SET_NORM_FLOAT, NULL, SF_FALSE);
        sf_command(pSndfileOut, SFC_SET_NORM_FLOAT, NULL, SF_FALSE);

    } else {
        // error...
        qDebug() << "sf_open error";
        emit finished();
        return;
    }

    while (int samples1 = sf_read_float(pSndfile1, m_pWorkBufferIn1, WORK_BUFFER_SIZE)) {
        qDebug() << "Reading samples...";

        int samples2 = sf_read_float(pSndfile2, m_pWorkBufferIn2, WORK_BUFFER_SIZE);

        qDebug() << "Reading samples... Samples 1: " << samples1 << " Samples 2: " << samples2;

        SampleUtil::applyGain(m_pWorkBufferOut, 0.0, WORK_BUFFER_SIZE);
        SampleUtil::addWithGain(m_pWorkBufferOut, m_pWorkBufferIn1, 1.0, samples1);

        if (samples2 > 0) {
            SampleUtil::addWithGain(m_pWorkBufferOut, m_pWorkBufferIn2, 1.0, samples2);
        }
        int written = sf_write_float(pSndfileOut, m_pWorkBufferOut, samples1);
        qDebug() << written << " Samples written";
    }

    sf_close(pSndfile1);
    sf_close(pSndfile2);
    sf_close(pSndfileOut);

    emit fileFinished(m_dest);
    emit finished();
}