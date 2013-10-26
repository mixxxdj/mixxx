// loopfilemixer.cpp
// Create by Carl Pillot on 9/21/13

#include <QObject>
#include <QDebug>
#include <sndfile.h>

#include "looprecording/loopfilemixer.h"

#include "recording/defs_recording.h"
#include "sampleutil.h"

#define WORK_BUFFER_SIZE 16384

LoopFileMixer::LoopFileMixer(QString file1, int length, QString dest, QString encoding)
        : m_dest(dest),
          m_encoding(encoding),
          m_filePath1(file1),
          m_iLength(length),
          m_iNumFiles(1) {

    m_pWorkBufferIn1 = SampleUtil::alloc(WORK_BUFFER_SIZE);
    m_pWorkBufferOut = SampleUtil::alloc(WORK_BUFFER_SIZE);

    qDebug() << "LoopFileMixer:  File: " << m_filePath1 << " dest: " << dest
    << " Encoding " << encoding;
}

LoopFileMixer::~LoopFileMixer() {
    qDebug() << "~LoopFileMixer()";
    SampleUtil::free(m_pWorkBufferIn1);
    SampleUtil::free(m_pWorkBufferOut);
}

void LoopFileMixer::slotProcess() {

    qDebug() << "!~!~! LoopFileMixer::slotProcess() !~!~!~!";

    SF_INFO sfInfo1;
    SF_INFO sfOutInfo;

    sfInfo1.format = 0;

    // TODO(carl): don't hardcode samplerate
    sfOutInfo.samplerate = 44100;
    sfOutInfo.channels = 2;

    if (m_encoding == ENCODING_WAVE) {
        sfOutInfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    } else {
        sfOutInfo.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_16;
    }

    SNDFILE* pSndfile1 = sf_open(m_filePath1.toLocal8Bit(), SFM_READ, &sfInfo1);
    SNDFILE* pSndfileOut = sf_open(m_dest.toLocal8Bit(), SFM_WRITE, &sfOutInfo);

    if (pSndfile1 == NULL || pSndfileOut == NULL) {
        qDebug() << "LoopFileMixer::slotProcess() Error opening Sndfile";
        emit finished();
        return;
    }

    sf_command(pSndfile1, SFC_SET_NORM_FLOAT, NULL, SF_FALSE);
    sf_command(pSndfileOut, SFC_SET_NORM_FLOAT, NULL, SF_FALSE);

    bool samplesAvailable = true;
    int samples1 = 0;
    int samplesProcessed = 0;

    while (samplesAvailable) {
        samples1 = sf_read_float(pSndfile1, m_pWorkBufferIn1, WORK_BUFFER_SIZE);
        qDebug() << "Reading samples... " << samples1;

        if ((samplesProcessed + samples1) > m_iLength) {

            int remainingSamples = m_iLength - samplesProcessed;
            if (remainingSamples > 0) {
                SampleUtil::applyGain(m_pWorkBufferOut, 0.0, WORK_BUFFER_SIZE);
                SampleUtil::addWithGain(m_pWorkBufferOut, m_pWorkBufferIn1, 1.0, remainingSamples);
                int written = sf_write_float(pSndfileOut, m_pWorkBufferOut, remainingSamples);
                samplesProcessed += written;
                qDebug() << written << " Samples written. Trimmed end of file.";
            }
            samplesAvailable = false;
        } else {
            SampleUtil::applyGain(m_pWorkBufferOut, 0.0, WORK_BUFFER_SIZE);
            SampleUtil::addWithGain(m_pWorkBufferOut, m_pWorkBufferIn1, 1.0, samples1);

            int written = sf_write_float(pSndfileOut, m_pWorkBufferOut, samples1);
            samplesProcessed += written;
            qDebug() << written << " Samples written. Total: " << samplesProcessed;
        }
    }

    qDebug() << "Copy finished. Total Samples Written: " << samplesProcessed;

    sf_close(pSndfile1);
    sf_close(pSndfileOut);

    emit fileFinished(m_dest);
    emit finished();
}
