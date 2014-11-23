#include <rubberband/RubberBandStretcher.h>

#include <QtDebug>

#include "engine/enginebufferscalerubberband.h"

#include "controlobject.h"
#include "engine/readaheadmanager.h"
#include "sampleutil.h"
#include "track/keyutils.h"
#include "util/counter.h"
#include "util/math.h"
#include "util/defs.h"

using RubberBand::RubberBandStretcher;

// This is the default increment from RubberBand 1.8.1.
static size_t kRubberBandBlockSize = 256;

EngineBufferScaleRubberBand::EngineBufferScaleRubberBand(
    ReadAheadManager* pReadAheadManager)
        : m_bBackwards(false),
          m_buffer_back(SampleUtil::alloc(MAX_BUFFER_LEN)),
          m_pRubberBand(NULL),
          m_pReadAheadManager(pReadAheadManager) {
    qDebug() << "RubberBand version" << RUBBERBAND_VERSION;

    m_retrieve_buffer[0] = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_retrieve_buffer[1] = SampleUtil::alloc(MAX_BUFFER_LEN);

    // m_iSampleRate defaults to 44100.
    initializeRubberBand(m_iSampleRate);
}

EngineBufferScaleRubberBand::~EngineBufferScaleRubberBand() {
    SampleUtil::free(m_buffer_back);
    SampleUtil::free(m_retrieve_buffer[0]);
    SampleUtil::free(m_retrieve_buffer[1]);

    if (m_pRubberBand) {
        delete m_pRubberBand;
        m_pRubberBand = NULL;
    }
}

void EngineBufferScaleRubberBand::initializeRubberBand(int iSampleRate) {
    if (m_pRubberBand) {
        delete m_pRubberBand;
        m_pRubberBand = NULL;
    }
    m_pRubberBand = new RubberBandStretcher(
        iSampleRate, 2,
        RubberBandStretcher::OptionProcessRealTime);
    m_pRubberBand->setMaxProcessSize(kRubberBandBlockSize);
}

void EngineBufferScaleRubberBand::setScaleParameters(int iSampleRate,
                                                     double base_rate,
                                                     bool speed_affects_pitch,
                                                     double* speed_adjust,
                                                     double* pitch_adjust) {
    if (m_iSampleRate != iSampleRate) {
        initializeRubberBand(iSampleRate);
        m_iSampleRate = iSampleRate;
    }

    // Negative speed means we are going backwards. pitch_adjust does not affect
    // the playback direction.
    m_bBackwards = *speed_adjust < 0;

    // Due to a bug in RubberBand, setting the timeRatio to a large value can
    // cause division-by-zero SIGFPEs. We limit the minimum seek speed to
    // prevent exceeding RubberBand's limits.
    //
    // References:
    // https://bugs.launchpad.net/ubuntu/+bug/1263233
    // https://bitbucket.org/breakfastquay/rubberband/issue/4/sigfpe-zero-division-with-high-time-ratios
    const double kMinSeekSpeed = 1.0 / 128.0;
    double speed_abs = fabs(*speed_adjust);
    if (speed_abs < kMinSeekSpeed) {
        // Let the caller know we ignored their speed.
        speed_abs = *speed_adjust = 0;
    }

    // RubberBand handles checking for whether the change in pitchScale is a
    // no-op.
    double pitchScale = base_rate * KeyUtils::octaveChangeToPowerOf2(*pitch_adjust);

    // The only difference for speed_affects_pitch is that we include speed_abs
    // as part of the pitch scale.
    if (speed_affects_pitch) {
        pitchScale *= speed_abs;
    }

    if (pitchScale > 0) {
        //qDebug() << "EngineBufferScaleRubberBand setPitchScale" << *pitch_adjust << pitchScale;
        m_pRubberBand->setPitchScale(pitchScale);
    }

    // RubberBand handles checking for whether the change in timeRatio is a
    // no-op. Time ratio is the ratio of stretched to unstretched duration. So 1
    // second in real duration is 0.5 seconds in stretched duration if tempo is
    // 2.
    double timeRatioInverse = base_rate * speed_abs;
    if (timeRatioInverse > 0) {
        //qDebug() << "EngineBufferScaleRubberBand setTimeRatio" << 1 / timeRatioInverse;
        m_pRubberBand->setTimeRatio(1.0 / timeRatioInverse);
    }

    if (m_pRubberBand->getInputIncrement() == 0) {
        qWarning() << "EngineBufferScaleRubberBand inputIncrement is 0."
                   << "On RubberBand <=1.8.1 a SIGFPE is imminent despite"
                   << "our workaround. Taking evasive action."
                   << "Please report this message to mixxx-devel@lists.sourceforge.net.";

        // This is much slower than the minimum seek speed workaround above.
        while (m_pRubberBand->getInputIncrement() == 0) {
            timeRatioInverse += 0.001;
            m_pRubberBand->setTimeRatio(1.0 / timeRatioInverse);
        }
        speed_abs = timeRatioInverse / base_rate;
        *speed_adjust = m_bBackwards ? -speed_abs : speed_abs;
    }

    // Used by other methods so we need to keep them up to date.
    m_dBaseRate = base_rate;
    m_bSpeedAffectsPitch = speed_affects_pitch;
    m_dSpeedAdjust = speed_abs;
    m_dPitchAdjust = *pitch_adjust;
}

void EngineBufferScaleRubberBand::clear() {
    m_pRubberBand->reset();
}

size_t EngineBufferScaleRubberBand::retrieveAndDeinterleave(CSAMPLE* pBuffer,
                                                            size_t frames) {
    size_t frames_available = m_pRubberBand->available();
    size_t frames_to_read = math_min(frames_available, frames);
    size_t received_frames = m_pRubberBand->retrieve(
        (float* const*)m_retrieve_buffer, frames_to_read);

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

    m_pRubberBand->process((const float* const*)m_retrieve_buffer,
                           frames, flush);
}


CSAMPLE* EngineBufferScaleRubberBand::getScaled(unsigned long buf_size) {
    // qDebug() << "EngineBufferScaleRubberBand::getScaled" << buf_size
    //          << "m_dSpeedAdjust" << m_dSpeedAdjust;
    m_samplesRead = 0.0;

    if (m_dBaseRate == 0 || m_dSpeedAdjust == 0) {
        SampleUtil::clear(m_buffer, buf_size);
        m_samplesRead = buf_size;
        return m_buffer;
    }

    const int iNumChannels = 2;
    unsigned long total_received_frames = 0;
    unsigned long total_read_frames = 0;

    unsigned long remaining_frames = buf_size/iNumChannels;
    CSAMPLE* read = m_buffer;
    bool last_read_failed = false;
    bool break_out_after_retrieve_and_reset_rubberband = false;
    while (remaining_frames > 0) {
        // ReadAheadManager will eventually read the requested frames with
        // enough calls to retrieveAndDeinterleave because CachingReader returns
        // zeros for reads that are not in cache. So it's safe to loop here
        // without any checks for failure in retrieveAndDeinterleave.
        unsigned long received_frames = retrieveAndDeinterleave(
            read, remaining_frames);
        remaining_frames -= received_frames;
        total_received_frames += received_frames;
        read += received_frames * iNumChannels;

        if (break_out_after_retrieve_and_reset_rubberband) {
            //qDebug() << "break_out_after_retrieve_and_reset_rubberband";
            // If we break out early then we have flushed RubberBand and need to
            // reset it.
            m_pRubberBand->reset();
            break;
        }

        size_t iLenFramesRequired = m_pRubberBand->getSamplesRequired();
        if (iLenFramesRequired == 0) {
            // rubberband 1.3 (packaged up through Ubuntu Quantal) has a bug
            // where it can report 0 samples needed forever which leads us to an
            // infinite loop. To work around this, we check if available() is
            // zero. If it is, then we submit a fixed block size of
            // kRubberBandBlockSize.
            int available = m_pRubberBand->available();
            if (available == 0) {
                iLenFramesRequired = kRubberBandBlockSize;
            }
        }
        //qDebug() << "iLenFramesRequired" << iLenFramesRequired;

        if (remaining_frames > 0 && iLenFramesRequired > 0) {
            unsigned long iAvailSamples = m_pReadAheadManager
                    ->getNextSamples(
                        // The value doesn't matter here. All that matters is we
                        // are going forward or backward.
                        (m_bBackwards ? -1.0 : 1.0) * m_dBaseRate * m_dSpeedAdjust,
                        m_buffer_back,
                        iLenFramesRequired * iNumChannels);
            unsigned long iAvailFrames = iAvailSamples / iNumChannels;

            if (iAvailFrames > 0) {
                last_read_failed = false;
                total_read_frames += iAvailFrames;
                deinterleaveAndProcess(m_buffer_back, iAvailFrames, false);
            } else {
                if (last_read_failed) {
                    // Flush and break out after the next retrieval. If we are
                    // at EOF this serves to get the last samples out of
                    // RubberBand.
                    deinterleaveAndProcess(m_buffer_back, 0, true);
                    break_out_after_retrieve_and_reset_rubberband = true;
                }
                last_read_failed = true;
            }
        }
    }

    if (remaining_frames > 0) {
        SampleUtil::clear(read, remaining_frames * iNumChannels);
        Counter counter("EngineBufferScaleRubberBand::getScaled underflow");
        counter.increment();
    }

    // m_samplesRead is interpreted as the total number of virtual samples
    // consumed to produce the scaled buffer. Due to this, we do not take into
    // account directionality or starting point.
    // NOTE(rryan): Why no m_dPitchAdjust here? Pitch does not change the time
    // ratio. m_dSpeedAdjust is the ratio of unstretched time to stretched
    // time. So, if we used total_received_frames*iNumChannels in stretched
    // time, then multiplying that by the ratio of unstretched time to stretched
    // time will get us the unstretched samples read.
    m_samplesRead = m_dBaseRate * m_dSpeedAdjust *
            total_received_frames * iNumChannels;

    return m_buffer;
}
