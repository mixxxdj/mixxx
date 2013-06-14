#include <rubberband/RubberBandStretcher.h>

#include <QMutexLocker>
#include <QtDebug>

#include "engine/enginebufferscalerubberband.h"
#include "engine/readaheadmanager.h"
#include "sampleutil.h"
#include "controlobject.h"

using RubberBand::RubberBandStretcher;

EngineBufferScaleRubberBand::EngineBufferScaleRubberBand(ReadAheadManager* pReadAheadManager)
        : m_bBackwards(false),
          m_buffer_back(SampleUtil::alloc(MAX_BUFFER_LEN)),
          m_bClear(true),
          m_pRubberBand(NULL),
          m_pReadAheadManager(pReadAheadManager) {
    m_retrieve_buffer[0] = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_retrieve_buffer[1] = SampleUtil::alloc(MAX_BUFFER_LEN);

    ControlObject* p = ControlObject::getControl(ConfigKey("[Master]", "samplerate"));
    connect(p, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetSamplerate(double)));
    initializeRubberBand(p->get());
}

EngineBufferScaleRubberBand::~EngineBufferScaleRubberBand() {
    SampleUtil::free(m_buffer_back);
    SampleUtil::free(m_retrieve_buffer[0]);
    SampleUtil::free(m_retrieve_buffer[1]);

    QMutexLocker locker(&m_qMutex);
    if (m_pRubberBand) {
        delete m_pRubberBand;
        m_pRubberBand = NULL;
    }
}


void EngineBufferScaleRubberBand::initializeRubberBand(int iSampleRate) {
    QMutexLocker locker(&m_qMutex);
    if (m_pRubberBand) {
        delete m_pRubberBand;
        m_pRubberBand = NULL;
    }
    m_pRubberBand = new RubberBandStretcher(
        iSampleRate, 2,
        RubberBandStretcher::OptionProcessRealTime);
}

void EngineBufferScaleRubberBand::setScaleParameters(double* rate_adjust,
                                                     double* tempo_adjust,
                                                     double* pitch_adjust) {
    // Assumes rate_adjust is just baserate (which cannot be negative) and
    // pitch_adjust cannot be negative because octave change conversion to pitch
    // ratio is an exp(x) function.
    m_bBackwards = (*tempo_adjust * *rate_adjust) < 0;


    // It's an error to pass a rate or tempo smaller than MIN_SEEK_SPEED to
    // SoundTouch (see definition of MIN_SEEK_SPEED for more details).
    double tempo_abs = fabs(*tempo_adjust);
    double rate_abs = fabs(*rate_adjust);

    bool tempo_changed = tempo_abs != m_dTempoAdjust;
    bool rate_changed = rate_abs != m_dRateAdjust;
    bool pitch_changed = *pitch_adjust != m_dPitchAdjust;

    QMutexLocker locker(&m_qMutex);

    if ((tempo_changed || rate_changed)) {
        // Time ratio is the ratio of stretched to unstretched duration. So 1
        // second in real duration is 0.5 seconds in stretched duration if tempo
        // is 2.
        double timeRatioInverse = rate_abs * tempo_abs;
        if (timeRatioInverse > 0) {
            //qDebug() << "EngineBufferScaleRubberBand setTimeRatio" << 1 / timeRatioInverse;
            m_pRubberBand->setTimeRatio(1 / timeRatioInverse);
        }
        m_dTempoAdjust = tempo_abs;
    }

    if (pitch_changed || rate_changed) {
        double pitchScale = pow(2, *pitch_adjust) * rate_abs;
        if (pitchScale > 0) {
            //qDebug() << "EngineBufferScaleRubberBand setPitchScale" << *pitch_adjust << pitchScale;
            m_pRubberBand->setPitchScale(pitchScale);
        }
        m_dPitchAdjust = *pitch_adjust;
    }

    // Accounted for in the above two blocks, but we need to set m_dRateAdjust
    // so we don't repeat if the adjustments don't change.
    if (rate_changed) {
        m_dRateAdjust = rate_abs;
    }
}

void EngineBufferScaleRubberBand::slotSetSamplerate(double dSampleRate) {
    initializeRubberBand(dSampleRate);
}

void EngineBufferScaleRubberBand::clear() {
    QMutexLocker locker(&m_qMutex);
    m_pRubberBand->reset();
    m_bClear = true;
}

size_t EngineBufferScaleRubberBand::retrieveAndDeinterleave(CSAMPLE* pBuffer, size_t frames) {
    size_t frames_available = m_pRubberBand->available();
    size_t frames_to_read = math_min(frames_available, frames);
    size_t received_frames = m_pRubberBand->retrieve((float* const*)m_retrieve_buffer,
                                                     frames_to_read);

    for (size_t i = 0; i < received_frames; ++i) {
        pBuffer[i*2] = m_retrieve_buffer[0][i];
        pBuffer[i*2+1] = m_retrieve_buffer[1][i];
    }

    return received_frames;
}

void EngineBufferScaleRubberBand::deinterleaveAndProcess(
    const CSAMPLE* pBuffer, size_t frames, bool flush) {

    for (size_t i = 0; i < frames; ++i) {
        m_retrieve_buffer[0][i] = pBuffer[i*2];
        m_retrieve_buffer[1][i] = pBuffer[i*2+1];
    }

    m_pRubberBand->process((const float* const*)m_retrieve_buffer, frames, flush);
}


CSAMPLE* EngineBufferScaleRubberBand::getScaled(unsigned long buf_size) {
    // qDebug() << "EngineBufferScaleRubberBand::getScaled" << buf_size
    //          << "m_dRateAdjust" << m_dRateAdjust
    //          << "m_dTempoAdjust" << m_dTempoAdjust;
    m_samplesRead = 0.0;

    if (m_dRateAdjust == 0 || m_dTempoAdjust == 0) {
        memset(m_buffer, 0, sizeof(m_buffer[0]) * buf_size);
        m_samplesRead = buf_size;
        return m_buffer;
    }

    QMutexLocker locker(&m_qMutex);

    const int iNumChannels = 2;
    unsigned long total_received_frames = 0;
    unsigned long total_read_frames = 0;

    unsigned long remaining_frames = buf_size/iNumChannels;
    //long remaining_source_frames = iBaseLength/2;
    CSAMPLE* read = m_buffer;
    bool last_read_failed = false;
    while (remaining_frames > 0) {
        unsigned long received_frames = retrieveAndDeinterleave(
            read, remaining_frames);
        remaining_frames -= received_frames;
        total_received_frames += received_frames;
        read += received_frames * iNumChannels;

        if (remaining_frames > 0) {
            // math_min(kiSoundTouchReadAheadLength,remaining_source_frames);
            unsigned long iLenFramesRequired = m_pRubberBand->getSamplesRequired();
            unsigned long iAvailSamples = m_pReadAheadManager
                    ->getNextSamples(
                        // The value doesn't matter here. All that matters is we
                        // are going forward or backward.
                        (m_bBackwards ? -1.0f : 1.0f) * m_dRateAdjust * m_dTempoAdjust,
                        m_buffer_back,
                        iLenFramesRequired * iNumChannels);
            unsigned long iAvailFrames = iAvailSamples / iNumChannels;

            if (iAvailFrames > 0) {
                last_read_failed = false;
                total_read_frames += iAvailFrames;
                deinterleaveAndProcess(m_buffer_back, iAvailFrames, false);
            } else {
                if (last_read_failed)
                    break;
                last_read_failed = true;
                // Flush
                deinterleaveAndProcess(m_buffer_back, 0, true);
            }
        }
    }

    // Feed more samples into SoundTouch until it has processed enough to
    // fill the audio buffer that we need to fill.
    // SoundTouch::numSamples() returns the number of _FRAMES_ that
    // are in its FIFO audio buffer...

    // Calculate new playpos

    //Get the stretched _frames_ (not Samples, as the function call
    //erroroneously implies)
    //long receivedFrames = m_pSoundTouch->receiveSamples((SAMPLETYPE*)buffer, buf_size/2);

    // qDebug() << "Fed ST" << total_read_frames*2
    //          << "samples to get" << total_received_frames*2 << "samples";
    if (total_received_frames != buf_size/2)
    {
        qDebug() << __FILE__ << "- only wrote" << total_received_frames << "frames instead of requested" << buf_size;
    }

    //for (unsigned long i = 0; i < buf_size; i++)
    //    qDebug() << buffer[i];

    // new_playpos is now interpreted as the total number of virtual samples
    // consumed to produce the scaled buffer. Due to this, we do not take into
    // account directionality or starting point.
    // NOTE(rryan): Why no m_dPitchAdjust here? SoundTouch implements pitch
    // shifting as a tempo shift of (1/m_dPitchAdjust) and a rate shift of
    // (*m_dPitchAdjust) so these two cancel out.
    m_samplesRead = m_dTempoAdjust * m_dRateAdjust *
            total_received_frames * 2;

    return m_buffer;
}

