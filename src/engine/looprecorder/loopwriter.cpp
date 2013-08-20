//  loopwriter.cpp
//  Created by Carl Pillot on 8/15/13.
//  Writes audio to a file for loop recording.

#include "engine/looprecorder/loopwriter.h"

#include "defs.h"
#include "sampleutil.h"
#include "util/counter.h"

#define LOOP_BUFFER_SIZE 16384
#define WORK_BUFFER_SIZE 4096

LoopWriter::LoopWriter()
        : m_sampleFifo(LOOP_BUFFER_SIZE),
        m_pWorkBuffer(SampleUtil::alloc(WORK_BUFFER_SIZE)),
        m_bIsFileAvailable(false),
        m_bIsRecording(false),
        m_iBreakPoint(0),
        m_iLoopLength(0),
        //m_iLoopRemainder(0),
        m_iSamplesRecorded(0),
        m_pSndfile(NULL) {
    connect(this, SIGNAL(samplesAvailable()), this, SLOT(slotProcessSamples()));
}

LoopWriter::~LoopWriter() {
    qDebug() << "!~!~!~!~!~! Loop writer deleted !~!~!~!~!~!~!";
    SampleUtil::free(m_pWorkBuffer);
    emit(finished());
}

void LoopWriter::process(const CSAMPLE* pBuffer, const int iBufferSize) {
    //qDebug() << "!~!~!~!~!~! LoopWriter::process !~!~!~!~!~!~!";
    //ScopedTimer t("EngineLoopRecorder::writeSamples");
    int samples_written = m_sampleFifo.write(pBuffer, iBufferSize);

    if (samples_written != iBufferSize) {
        // Should this check overrun vs underrun?
        qDebug() << "!~!~!~!~!~! LoopWriter::process Buffer Overrun";
        Counter("LoopWriter::process buffer overrun").increment();
    }

    if (m_sampleFifo.writeAvailable() < LOOP_BUFFER_SIZE/4) {
        // Signal to the loop recorder that samples are available.
        emit(samplesAvailable());
    }
}

void LoopWriter::slotClearWriter() {
    slotStopRecording(false);
    emit(clearRecorder());
}

void LoopWriter::slotSetFile(SNDFILE* pFile) {
    qDebug() << "!~!~!~!~!~! LoopWriter::slotSetFile";

    if (!m_bIsFileAvailable) {
        if (pFile != NULL) {
            m_pSndfile = pFile;
            m_bIsFileAvailable = true;
        }
    }
}

void LoopWriter::slotStartRecording(int samples) {
    m_iSamplesRecorded = 0;

    // TODO(carl): figure out extra padding needed.
    // I think the loops are getting shortened by the crossfade.
    m_iLoopLength = samples;
    m_iBreakPoint = m_iLoopLength - WORK_BUFFER_SIZE;

    qDebug() << "!~!~!~!~!~! LoopWriter::slotStartRecording Length: " << m_iLoopLength <<
                "Break: " << m_iBreakPoint << " !~!~!~!~!~!~!";
    m_bIsRecording = true;
    emit(isRecording(true));
}

void LoopWriter::slotStopRecording(bool playLoop) {
    qDebug() << "!~!~!~!~!~! LoopWriter::slotStopRecording Samples Recorded: " << m_iSamplesRecorded << " !~!~!~!~!~!~!";

    m_bIsRecording = false;
    emit(isRecording(false));
    // TODO(carl) check if temp buffers are open and clear them.

    m_iBreakPoint = 0;
    m_iLoopLength = 0;
    //m_iLoopRemainder = 0;
    m_iSamplesRecorded = 0;

    if (m_bIsFileAvailable) {
        closeFile();
    }

    if (playLoop) {
        emit(loadAudio());
    }
}

void LoopWriter::slotProcessSamples() {
    //qDebug() << "!~!~!~!~!~! LoopWriter::slotProcessSamples !~!~!~!~!~!~!";
    int iSamplesRead;
    if ((iSamplesRead = m_sampleFifo.read(m_pWorkBuffer, WORK_BUFFER_SIZE))) {

        // States for recording:
        // 1. not recording
        // 2. Recording, but file not yet open
        // 3. Recording, file open
        // 4. Stopping recording (closing file, sending to deck)

        if (m_bIsRecording) {
            writeBuffer(m_pWorkBuffer, iSamplesRead);
        }

        // More samples may be available, so we signal for another event.
        emit(samplesAvailable());
    }
}

void LoopWriter::closeFile() {
    qDebug() << "!~!~!~!~!~! LoopWriter::closeFile !~!~!~!~!~!~!";
    if(m_pSndfile != NULL) {
        m_bIsFileAvailable = false;
        sf_close(m_pSndfile);
        // is this NULL assignment necessary?
        m_pSndfile = NULL;
    }
}

void LoopWriter::writeBuffer(const CSAMPLE* pBuffer, const int iBufferSize) {
    
    if (m_bIsFileAvailable) {

        // TODO(carl) check for buffers to flush
        if ((m_iLoopLength > 0) && (m_iSamplesRecorded >= m_iBreakPoint)) {
            qDebug () << "Passed breakpoint.";
            if ((m_iSamplesRecorded + iBufferSize) >= m_iLoopLength) {
                // Trim loop to exact length specified.
                unsigned int iRemainder = m_iLoopLength - m_iSamplesRecorded;
                qDebug() << "!~!~!~!~!~! Trimming Loop. Remainder: " << iRemainder
                        << " Samples Rec: " << m_iSamplesRecorded << " !~!~!~!~!~!~!";
                sf_write_float(m_pSndfile, pBuffer, iRemainder);
                m_iSamplesRecorded += iRemainder;
                qDebug() << "!~!~!~!~!~! Samples Recorded: " << m_iSamplesRecorded
                        << "Remainder: " << iRemainder << " !~!~!~!~!~!~!";
                slotStopRecording(true);
            } else {
                sf_write_float(m_pSndfile, pBuffer, iBufferSize);
                m_iSamplesRecorded += iBufferSize;
                qDebug() << "!~!~!~!~!~! Samples Recorded: " << m_iSamplesRecorded <<  " !~!~!~!~!~!~!";
            }
        } else {
            sf_write_float(m_pSndfile, pBuffer, iBufferSize);
            m_iSamplesRecorded += iBufferSize;
            qDebug() << "!~!~!~!~!~! Samples Recorded: " << m_iSamplesRecorded <<  " !~!~!~!~!~!~!";
        }
    } else {
        // TODO(carl) write to temporary buffer.
        qDebug() << "!~!~!~!~!~! LoopWriter::writeBuffer Buffer dropped !~!~!~!~!~!~!";
    }

}
