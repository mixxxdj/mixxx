#include "engine/bufferscalers/enginebufferscalerubberband.h"

#include <rubberband/RubberBandStretcher.h>

#include <QtDebug>

#include "control/controlobject.h"
#include "engine/readaheadmanager.h"
#include "moc_enginebufferscalerubberband.cpp"
#include "track/keyutils.h"
#include "util/counter.h"
#include "util/defs.h"
#include "util/math.h"
#include "util/sample.h"

using RubberBand::RubberBandStretcher;

namespace {

// This is the default increment from RubberBand 1.8.1.
size_t kRubberBandBlockSize = 256;

}  // namespace

EngineBufferScaleRubberBand::EngineBufferScaleRubberBand(
        ReadAheadManager* pReadAheadManager)
        : m_pReadAheadManager(pReadAheadManager),
          m_buffer_back(SampleUtil::alloc(MAX_BUFFER_LEN)),
          m_bBackwards(false) {
    m_retrieve_buffer[0] = SampleUtil::alloc(MAX_BUFFER_LEN);
    m_retrieve_buffer[1] = SampleUtil::alloc(MAX_BUFFER_LEN);
    // Initialize the internal buffers to prevent re-allocations
    // in the real-time thread.
    onSampleRateChanged();
}

EngineBufferScaleRubberBand::~EngineBufferScaleRubberBand() {
    SampleUtil::free(m_buffer_back);
    SampleUtil::free(m_retrieve_buffer[0]);
    SampleUtil::free(m_retrieve_buffer[1]);
}

void EngineBufferScaleRubberBand::setScaleParameters(double base_rate,
                                                     double* pTempoRatio,
                                                     double* pPitchRatio) {
    // Negative speed means we are going backwards. pitch does not affect
    // the playback direction.
    m_bBackwards = *pTempoRatio < 0;

    // Due to a bug in RubberBand, setting the timeRatio to a large value can
    // cause division-by-zero SIGFPEs. We limit the minimum seek speed to
    // prevent exceeding RubberBand's limits.
    //
    // References:
    // https://bugs.launchpad.net/ubuntu/+bug/1263233
    // https://bitbucket.org/breakfastquay/rubberband/issue/4/sigfpe-zero-division-with-high-time-ratios
    const double kMinSeekSpeed = 1.0 / 128.0;
    double speed_abs = fabs(*pTempoRatio);
    if (speed_abs < kMinSeekSpeed) {
        // Let the caller know we ignored their speed.
        speed_abs = *pTempoRatio = 0;
    }

    // RubberBand handles checking for whether the change in pitchScale is a
    // no-op.
    double pitchScale = fabs(base_rate * *pPitchRatio);

    if (pitchScale > 0) {
        //qDebug() << "EngineBufferScaleRubberBand setPitchScale" << *pitch << pitchScale;
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
        *pTempoRatio = m_bBackwards ? -speed_abs : speed_abs;
    }

    // Used by other methods so we need to keep them up to date.
    m_dBaseRate = base_rate;
    m_dTempoRatio = speed_abs;
    m_dPitchRatio = *pPitchRatio;
}

void EngineBufferScaleRubberBand::onSampleRateChanged() {
    // TODO: Resetting the sample rate will cause internal
    // memory allocations that may block the real-time thread.
    // When is this function actually invoked??
    if (!getOutputSignal().isValid()) {
        m_pRubberBand.reset();
        return;
    }
    m_pRubberBand = std::make_unique<RubberBandStretcher>(
            getOutputSignal().getSampleRate(),
            getOutputSignal().getChannelCount(),
            RubberBandStretcher::OptionProcessRealTime);
    m_pRubberBand->setMaxProcessSize(kRubberBandBlockSize);
    // Setting the time ratio to a very high value will cause RubberBand
    // to preallocate buffers large enough to (almost certainly)
    // avoid memory reallocations during playback.
    m_pRubberBand->setTimeRatio(2.0);
    m_pRubberBand->setTimeRatio(1.0);
}

void EngineBufferScaleRubberBand::clear() {
    VERIFY_OR_DEBUG_ASSERT(m_pRubberBand) {
        return;
    }
    m_pRubberBand->reset();
}

SINT EngineBufferScaleRubberBand::retrieveAndDeinterleave(
        CSAMPLE* pBuffer,
        SINT frames) {
    SINT frames_available = m_pRubberBand->available();
    SINT frames_to_read = math_min(frames_available, frames);
    SINT received_frames = m_pRubberBand->retrieve(
            (float* const*)m_retrieve_buffer, frames_to_read);

    SampleUtil::interleaveBuffer(pBuffer,
                                 m_retrieve_buffer[0],
                                 m_retrieve_buffer[1],
                                 received_frames);
    return received_frames;
}

void EngineBufferScaleRubberBand::deinterleaveAndProcess(
        const CSAMPLE* pBuffer, SINT frames, bool flush) {

    SampleUtil::deinterleaveBuffer(
            m_retrieve_buffer[0], m_retrieve_buffer[1], pBuffer, frames);

    m_pRubberBand->process((const float* const*)m_retrieve_buffer,
                           frames, flush);
}

double EngineBufferScaleRubberBand::scaleBuffer(
        CSAMPLE* pOutputBuffer,
        SINT iOutputBufferSize) {
    if (m_dBaseRate == 0.0 || m_dTempoRatio == 0.0) {
        SampleUtil::clear(pOutputBuffer, iOutputBufferSize);
        // No actual samples/frames have been read from the
        // unscaled input buffer!
        return 0.0;
    }

    SINT total_received_frames = 0;
    SINT total_read_frames = 0;

    SINT remaining_frames = getOutputSignal().samples2frames(iOutputBufferSize);
    CSAMPLE* read = pOutputBuffer;
    bool last_read_failed = false;
    bool break_out_after_retrieve_and_reset_rubberband = false;
    while (remaining_frames > 0) {
        // ReadAheadManager will eventually read the requested frames with
        // enough calls to retrieveAndDeinterleave because CachingReader returns
        // zeros for reads that are not in cache. So it's safe to loop here
        // without any checks for failure in retrieveAndDeinterleave.
        SINT received_frames = retrieveAndDeinterleave(
                read, remaining_frames);
        remaining_frames -= received_frames;
        total_received_frames += received_frames;
        read += getOutputSignal().frames2samples(received_frames);

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
            SINT iAvailSamples = m_pReadAheadManager->getNextSamples(
                        // The value doesn't matter here. All that matters is we
                        // are going forward or backward.
                        (m_bBackwards ? -1.0 : 1.0) * m_dBaseRate * m_dTempoRatio,
                        m_buffer_back,
                        getOutputSignal().frames2samples(iLenFramesRequired));
            SINT iAvailFrames = getOutputSignal().samples2frames(iAvailSamples);

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
        SampleUtil::clear(read, getOutputSignal().frames2samples(remaining_frames));
        Counter counter("EngineBufferScaleRubberBand::getScaled underflow");
        counter.increment();
    }

    // framesRead is interpreted as the total number of virtual sample frames
    // consumed to produce the scaled buffer. Due to this, we do not take into
    // account directionality or starting point.
    // NOTE(rryan): Why no m_dPitchAdjust here? Pitch does not change the time
    // ratio. m_dSpeedAdjust is the ratio of unstretched time to stretched
    // time. So, if we used total_received_frames in stretched time, then
    // multiplying that by the ratio of unstretched time to stretched time
    // will get us the unstretched sample frames read.
    double framesRead = m_dBaseRate * m_dTempoRatio * total_received_frames;

    return framesRead;
}
