#include "engine/bufferscalers/enginebufferscalerubberband.h"

#include <QFile>
#include <QtDebug>

#include "engine/readaheadmanager.h"
#include "moc_enginebufferscalerubberband.cpp"
#include "util/counter.h"
#include "util/defs.h"
#include "util/math.h"
#include "util/mutex.h"
#include "util/sample.h"
#include "util/timer.h"

using RubberBand::RubberBandStretcher;

#define RUBBERBANDV3 (RUBBERBAND_API_MAJOR_VERSION >= 3 || \
        (RUBBERBAND_API_MAJOR_VERSION == 2 && RUBBERBAND_API_MINOR_VERSION >= 7))

EngineBufferScaleRubberBand::EngineBufferScaleRubberBand(
        ReadAheadManager* pReadAheadManager)
        : m_pReadAheadManager(pReadAheadManager),
          m_buffers(),
          m_bufferPtrs(),
          m_interleavedReadBuffer(MAX_BUFFER_LEN),
          m_bBackwards(false),
          m_useEngineFiner(false) {
    // Initialize the internal buffers to prevent re-allocations
    // in the real-time thread.
    onSignalChanged();
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
    // https://todo.sr.ht/~breakfastquay/rubberband/5

    double speed_abs = fabs(*pTempoRatio);
    if (runningEngineVersion() == 2) {
        constexpr double kMinSeekSpeed = 1.0 / 128.0;
        if (speed_abs < kMinSeekSpeed) {
            // Let the caller know we ignored their speed.
            speed_abs = *pTempoRatio = 0;
        }
    }
    // RubberBand handles checking for whether the change in pitchScale is a
    // no-op.
    double pitchScale = fabs(base_rate * *pPitchRatio);

    if (pitchScale > 0) {
        //qDebug() << "EngineBufferScaleRubberBand setPitchScale" << *pitch << pitchScale;
        m_rubberBand.setPitchScale(pitchScale);
    }

    // RubberBand handles checking for whether the change in timeRatio is a
    // no-op. Time ratio is the ratio of stretched to unstretched duration. So 1
    // second in real duration is 0.5 seconds in stretched duration if tempo is
    // 2.
    double timeRatioInverse = base_rate * speed_abs;
    if (timeRatioInverse > 0) {
        //qDebug() << "EngineBufferScaleRubberBand setTimeRatio" << 1 / timeRatioInverse;
        m_rubberBand.setTimeRatio(1.0 / timeRatioInverse);
    }

    if (runningEngineVersion() == 2) {
        if (m_rubberBand.getInputIncrement() == 0) {
            qWarning() << "EngineBufferScaleRubberBand inputIncrement is 0."
                       << "On RubberBand <=1.8.1 a SIGFPE is imminent despite"
                       << "our workaround. Taking evasive action."
                       << "Please file an issue on https://github.com/mixxxdj/mixxx/issues";

            // This is much slower than the minimum seek speed workaround above.
            while (m_rubberBand.getInputIncrement() == 0) {
                timeRatioInverse += 0.001;
                m_rubberBand.setTimeRatio(1.0 / timeRatioInverse);
            }
            speed_abs = timeRatioInverse / base_rate;
            *pTempoRatio = m_bBackwards ? -speed_abs : speed_abs;
        }
    }
    // Used by other methods so we need to keep them up to date.
    m_dBaseRate = base_rate;
    m_dTempoRatio = speed_abs;
    m_dPitchRatio = *pPitchRatio;
}

void EngineBufferScaleRubberBand::onSignalChanged() {
    // TODO: Resetting the sample rate will cause internal
    // memory allocations that may block the real-time thread.
    // When is this function actually invoked??
    if (!getOutputSignal().isValid()) {
        return;
    }

    uint8_t channelCount = getOutputSignal().getChannelCount();
    if (m_buffers.size() != channelCount) {
        m_buffers.resize(channelCount);
    }

    if (m_bufferPtrs.size() != channelCount) {
        m_bufferPtrs.resize(channelCount);
    }

    m_rubberBand.clear();

    for (int chIdx = 0; chIdx < channelCount; chIdx++) {
        if (m_buffers[chIdx].size() == MAX_BUFFER_LEN) {
            continue;
        }
        m_buffers[chIdx] = mixxx::SampleBuffer(MAX_BUFFER_LEN);
        m_bufferPtrs[chIdx] = m_buffers[chIdx].data();
    }

    RubberBandStretcher::Options rubberbandOptions =
            RubberBandStretcher::OptionProcessRealTime;
#if RUBBERBANDV3
    if (m_useEngineFiner) {
        rubberbandOptions |=
                RubberBandStretcher::OptionEngineFiner |
                // Process Channels Together. otherwise the result is not
                // mono-compatible. See #11361
                RubberBandStretcher::OptionChannelsTogether;
    }
#endif

    m_rubberBand.setup(
            getOutputSignal().getSampleRate(),
            getOutputSignal().getChannelCount(),
            rubberbandOptions);
    // Setting the time ratio to a very high value will cause RubberBand
    // to preallocate buffers large enough to (almost certainly)
    // avoid memory reallocations during playback.
    m_rubberBand.setTimeRatio(2.0);
    m_rubberBand.setTimeRatio(1.0);
}

void EngineBufferScaleRubberBand::clear() {
    VERIFY_OR_DEBUG_ASSERT(m_rubberBand.isValid()) {
        return;
    }
    reset();
}

SINT EngineBufferScaleRubberBand::retrieveAndDeinterleave(
        CSAMPLE* pBuffer,
        SINT frames) {
    VERIFY_OR_DEBUG_ASSERT(m_rubberBand.isValid()) {
        return 0;
    }
    // NOTE: If we still need to throw away padding, then we can also
    //       immediately read those frames in addition to the frames we actually
    //       need for the output
    SINT received_frames;
    {
        ScopedTimer t(QStringLiteral("RubberBand::retrieve"));
        received_frames = static_cast<SINT>(m_rubberBand.retrieve(
                m_bufferPtrs.data(), frames + m_remainingPaddingInOutput, m_buffers[0].size()));
    }
    SINT frame_offset = 0;

    // As explained below in `reset()`, the first time this is called we need to
    // drop the silence we fed into the time stretcher as padding from the
    // output
    if (m_remainingPaddingInOutput > 0) {
        const SINT drop_num_frames = std::min(received_frames, m_remainingPaddingInOutput);

        m_remainingPaddingInOutput -= drop_num_frames;
        received_frames -= drop_num_frames;
        frame_offset += drop_num_frames;
    }

    DEBUG_ASSERT(received_frames <= frames);

    switch (getOutputSignal().getChannelCount()) {
    case mixxx::audio::ChannelCount::stereo():
        SampleUtil::interleaveBuffer(pBuffer,
                m_buffers[0].data(frame_offset),
                m_buffers[1].data(frame_offset),
                received_frames);
        break;
    case mixxx::audio::ChannelCount::stem():
        SampleUtil::interleaveBuffer(pBuffer,
                m_buffers[0].data(frame_offset),
                m_buffers[1].data(frame_offset),
                m_buffers[2].data(frame_offset),
                m_buffers[3].data(frame_offset),
                m_buffers[4].data(frame_offset),
                m_buffers[5].data(frame_offset),
                m_buffers[6].data(frame_offset),
                m_buffers[7].data(frame_offset),
                received_frames);
        break;
    default: {
        int chCount = getOutputSignal().getChannelCount();
        // The buffers samples are ordered as following
        //  m_buffers#1 = 11..
        //  m_buffers#2 = 22..
        //  m_buffers#3 = 33..
        //  m_buffers#4 = 44..
        //  m_buffers#X = XX..
        // And need to be reordered as following in pBuffer
        //  1234..X1234...X...
        //
        // Because of the unanticipated number of buffer and channel, we cannot
        // use any SampleUtil in this case
        for (SINT frameIdx = 0; frameIdx < frames; ++frameIdx) {
            for (int channel = 0; channel < chCount; channel++) {
                pBuffer[frameIdx * chCount + channel] = m_buffers[channel].data()[frameIdx];
            }
        }
    } break;
    }

    return received_frames;
}

void EngineBufferScaleRubberBand::deinterleaveAndProcess(
        const CSAMPLE* pBuffer,
        SINT frames) {
    VERIFY_OR_DEBUG_ASSERT(m_rubberBand.isValid()) {
        return;
    }
    DEBUG_ASSERT(frames <= static_cast<SINT>(m_buffers[0].size()));

    switch (getOutputSignal().getChannelCount()) {
    case mixxx::audio::ChannelCount::stereo():
        SampleUtil::deinterleaveBuffer(
                m_buffers[0].data(),
                m_buffers[1].data(),
                pBuffer,
                frames);
        break;
    case mixxx::audio::ChannelCount::stem():
        SampleUtil::deinterleaveBuffer(
                m_buffers[0].data(),
                m_buffers[1].data(),
                m_buffers[2].data(),
                m_buffers[3].data(),
                m_buffers[4].data(),
                m_buffers[5].data(),
                m_buffers[6].data(),
                m_buffers[7].data(),
                pBuffer,
                frames);
        break;
    default: {
        int chCount = getOutputSignal().getChannelCount();
        // The sampler are ordered as following in pBuffer
        //    1234..X1234...X...
        // And need to be reordered as following
        // m_buffers#1 = 11..
        // m_buffers#2 = 22..
        // m_buffers#3 = 33..
        // m_buffers#4 = 44..
        // m_buffers#X = XX..
        //
        // Because of the unanticipated number of buffer and channel, we cannot
        // use any SampleUtil in this case
        for (SINT frameIdx = 0; frameIdx < frames; ++frameIdx) {
            for (int channel = 0; channel < chCount; channel++) {
                m_buffers[channel].data()[frameIdx] =
                        pBuffer[frameIdx * chCount + channel];
            }
        }
    } break;
    }

    {
        ScopedTimer t(QStringLiteral("RubberBand::process"));
        m_rubberBand.process(m_bufferPtrs.data(),
                frames,
                false);
    }
}

double EngineBufferScaleRubberBand::scaleBuffer(
        CSAMPLE* pOutputBuffer,
        SINT iOutputBufferSize) {
    VERIFY_OR_DEBUG_ASSERT(m_rubberBand.isValid()) {
        return 0.0;
    }
    ScopedTimer t(QStringLiteral("EngineBufferScaleRubberBand::scaleBuffer"));
    if (m_dBaseRate == 0.0 || m_dTempoRatio == 0.0) {
        SampleUtil::clear(pOutputBuffer, iOutputBufferSize);
        // No actual samples/frames have been read from the
        // unscaled input buffer!
        return 0.0;
    }

    double readFramesProcessed = 0;
    SINT remaining_frames = getOutputSignal().samples2frames(iOutputBufferSize);
    CSAMPLE* read = pOutputBuffer;
    bool last_read_failed = false;
    while (remaining_frames > 0) {
        // ReadAheadManager will eventually read the requested frames with
        // enough calls to retrieveAndDeinterleave because CachingReader returns
        // zeros for reads that are not in cache. So it's safe to loop here
        // without any checks for failure in retrieveAndDeinterleave.
        // If the time stretcher has just been reset then this will throw away
        // the first `m_remainingPaddingInOutput` samples of silence padding
        // from the output.
        SINT received_frames = retrieveAndDeinterleave(
                read, remaining_frames);
        remaining_frames -= received_frames;
        readFramesProcessed += m_effectiveRate * received_frames;
        read += getOutputSignal().frames2samples(received_frames);

        const SINT next_block_frames_required =
                static_cast<SINT>(m_rubberBand.getSamplesRequired());
        if (remaining_frames > 0 && next_block_frames_required > 0) {
            // The requested setting becomes effective after all previous frames have been processed
            m_effectiveRate = m_dBaseRate * m_dTempoRatio;
            const SINT available_samples = m_pReadAheadManager->getNextSamples(
                    // The value doesn't matter here. All that matters is we
                    // are going forward or backward.
                    (m_bBackwards ? -1.0 : 1.0) * m_dBaseRate * m_dTempoRatio,
                    m_interleavedReadBuffer.data(),
                    getOutputSignal().frames2samples(next_block_frames_required),
                    getOutputSignal().getChannelCount());
            const SINT available_frames = getOutputSignal().samples2frames(available_samples);

            if (available_frames > 0) {
                last_read_failed = false;
                deinterleaveAndProcess(m_interleavedReadBuffer.data(), available_frames);
            } else {
                // We may get 0 samples once if we just hit a loop trigger, e.g.
                // when reloop_toggle jumps back to loop_in, or when moving a
                // loop causes the play position to be moved along.
                if (last_read_failed) {
                    // If we get 0 samples repeatedly, flush and break out after
                    // the next retrieval. If we are at EOF this serves to get
                    // the last samples out of RubberBand.
                    qDebug() << "ReadAheadManager::getNextSamples() returned "
                                "zero samples repeatedly. Padding with silence.";
                    SampleUtil::clear(
                            m_interleavedReadBuffer.data(),
                            getOutputSignal().frames2samples(next_block_frames_required));
                    deinterleaveAndProcess(m_interleavedReadBuffer.data(),
                            next_block_frames_required);
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

    // readFramesProcessed is interpreted as the total number of frames
    // consumed to produce the scaled buffer. Due to this, we do not take into
    // account directionality or starting point.
    return readFramesProcessed;
}

// static
bool EngineBufferScaleRubberBand::isEngineFinerAvailable() {
    return RUBBERBANDV3;
}

void EngineBufferScaleRubberBand::useEngineFiner(bool enable) {
    if (isEngineFinerAvailable()) {
        m_useEngineFiner = enable;
        onSignalChanged();
    }
}

size_t EngineBufferScaleRubberBand::getPreferredStartPad() const {
    return m_rubberBand.getPreferredStartPad();
}

size_t EngineBufferScaleRubberBand::getStartDelay() const {
    return m_rubberBand.getStartDelay();
}

int EngineBufferScaleRubberBand::runningEngineVersion() {
    return m_rubberBand.getEngineVersion();
}

void EngineBufferScaleRubberBand::reset() {
    m_rubberBand.reset();

    // As mentioned in the docs (https://breakfastquay.com/rubberband/code-doc/)
    // and FAQ (https://breakfastquay.com/rubberband/integration.html#faqs), you
    // need to run some silent samples through the time stretching engine first
    // before using it. Otherwise it will eat add a short fade-in, destroying
    // the initial transient.
    //
    // See https://github.com/mixxxdj/mixxx/pull/11120#discussion_r1050011104
    // for more information.
    size_t remaining_padding = getPreferredStartPad();
    const size_t block_size = std::min<size_t>(remaining_padding, m_buffers[0].size());
    for (auto& buffer : m_buffers) {
        buffer.clear();
    }
    while (remaining_padding > 0) {
        const size_t pad_samples = std::min<size_t>(remaining_padding, block_size);
        {
            ScopedTimer t(QStringLiteral("RubberBand::process"));
            m_rubberBand.process(m_bufferPtrs.data(), pad_samples, false);
        }

        remaining_padding -= pad_samples;
    }

    // The silence we just added covers half a window (see the last paragraph of
    // https://github.com/mixxxdj/mixxx/pull/11120#discussion_r1050011104). This
    // silence should be dropped from the result when the `retrieve()` in
    // `retrieveAndDeinterleave()` first starts producing audio.
    m_remainingPaddingInOutput = static_cast<SINT>(getStartDelay());
}
