//  loopwriter.cpp
//  Created by Carl Pillot on 8/15/13.
//  Writes audio to a file for loop recording.

#include "engine/looprecorder/loopwriter.h"

#include "defs.h"
#include "sampleutil.h"
#include "util/counter.h"

#define LOOP_BUFFER_SIZE 16384
#define WORK_BUFFER_SIZE 2048

LoopWriter::LoopWriter()
        : m_sampleFifo(LOOP_BUFFER_SIZE),
          m_pWorkBuffer(SampleUtil::alloc(WORK_BUFFER_SIZE)),
          m_bRecording(false),
          m_iLoopLength(0),
          m_iSamplesRecorded(0),
          m_pSndfile(NULL) {
    connect(this, SIGNAL(samplesAvailable()), this, SLOT(slotProcessSamples()));
}

LoopWriter::~LoopWriter() {
    qDebug() << "!~!~!~!~!~! Loop writer deleted !~!~!~!~!~!~!";

    if(m_bRecording) {
        slotStopRecording(false);
    }

    SampleUtil::free(m_pWorkBuffer);
}

void LoopWriter::process(const CSAMPLE* pBuffer, const int iBufferSize) {
    //qDebug() << "!~!~!~!~!~! LoopWriter::process !~!~!~!~!~!~!";
    int samples_written = m_sampleFifo.write(pBuffer, iBufferSize);

    if (samples_written != iBufferSize) {
        // Should this check overrun vs underrun?
        qDebug() << "!~!~!~!~!~! LoopWriter::process Buffer Overrun";
        Counter("LoopWriter::process buffer overrun").increment();
    }

    if (m_sampleFifo.writeAvailable() < LOOP_BUFFER_SIZE/4) {
        // Signal to the loop writer that samples are available.
        emit(samplesAvailable());
    }
}

void LoopWriter::slotClearWriter() {
    slotStopRecording(false);
    emit(clearRecorder());
}

void LoopWriter::slotStartRecording(int samples, SNDFILE* pSndfile) {
    m_iSamplesRecorded = 0;
    m_iLoopLength = samples;
    m_pSndfile = pSndfile;
    m_bRecording = true;
    emit(isRecording(true));
}

void LoopWriter::slotStopRecording(bool playLoop) {
    qDebug() << "!~!~!~!~!~! LoopWriter::slotStopRecording Samples Recorded: " << m_iSamplesRecorded << " !~!~!~!~!~!~!";

    int iTotalSamples = m_iSamplesRecorded;

    m_bRecording = false;
    emit(isRecording(false));
    // TODO(carl) check if temp buffers are open and clear them.

    m_iLoopLength = 0;
    m_iSamplesRecorded = 0;

    closeFile();

    if (playLoop) {
        emit(loadAudio(iTotalSamples));
    }
}

void LoopWriter::slotProcessSamples() {
    //qDebug() << "!~!~!~!~!~! LoopWriter::slotProcessSamples !~!~!~!~!~!~!";
    int iSamplesRead;
    if ((iSamplesRead = m_sampleFifo.read(m_pWorkBuffer, WORK_BUFFER_SIZE))) {

        if (m_bRecording) {
            writeBuffer(m_pWorkBuffer, iSamplesRead);
        }

        // More samples may be available, so we signal for another event.
        emit(samplesAvailable());
    }
}

void LoopWriter::closeFile() {
    qDebug() << "!~!~!~!~!~! LoopWriter::closeFile !~!~!~!~!~!~!";
    if(m_pSndfile == NULL) {
        return;
    }

    sf_close(m_pSndfile);
    m_pSndfile = NULL;
}

void LoopWriter::writeBuffer(const CSAMPLE* pBuffer, const int iBufferSize) {

    if (m_pSndfile == NULL) {
        // TODO(carl) write to temporary buffer.
        qDebug() << "!~!~!~!~!~! LoopWriter::writeBuffer Buffer dropped !~!~!~!~!~!~!";
        return;
    }

    unsigned int iNewSize = m_iSamplesRecorded + iBufferSize;

    // TODO(carl) maybe add empty padding at end of file?
    if ((m_iLoopLength > 0) && (iNewSize >= m_iLoopLength)) {
        // We trim the loop to the correct length by setting loop points in the loop recorder deck.
        sf_write_float(m_pSndfile, pBuffer, iBufferSize);
        m_iSamplesRecorded = iNewSize;
        slotStopRecording(true);
    } else {
        sf_write_float(m_pSndfile, pBuffer, iBufferSize);
        m_iSamplesRecorded = iNewSize;
    }
}
