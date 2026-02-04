#include "engine/bufferscalers/enginebufferscalest.h"

#include "moc_enginebufferscalest.cpp"

// Fixes redefinition warnings from SoundTouch.
#include <soundtouch/SoundTouch.h>

#include "engine/readaheadmanager.h"
#include "util/math.h"
#include "util/sample.h"

using namespace soundtouch;

namespace {

// Due to filtering and oversampling, SoundTouch is some samples behind (offset).
// The value below was experimental identified using a saw signal and SoundTouch 2.1.1
constexpr SINT kSeekOffsetFramesV20101 = 429;

// From V 2.3.0 Soundtouch has no initial offset at unity, but it is too late with
// lowered pitch and too early with raised pitch up to ~+-2000 frames (~+-50 ms).
// This can be seen in a recording of a saw signal with changed pitch.
// The saws tooth are shifted from their input position depending on the pitch.
// TODO() Compensate that. This is probably cause by the delayed adoption of pitch changes due
// to the SoundTouch chunk size.

constexpr SINT kBackBufferFrameSize = 512;

}  // namespace

EngineBufferScaleST::EngineBufferScaleST(ReadAheadManager* pReadAheadManager)
        : m_pReadAheadManager(pReadAheadManager),
          m_pSoundTouch(std::make_unique<soundtouch::SoundTouch>()),
          m_bBackwards(false) {
    m_pSoundTouch->setRate(m_dBaseRate);
    m_pSoundTouch->setPitch(1.0);
    m_pSoundTouch->setSetting(SETTING_USE_QUICKSEEK, 1);
    // Initialize the internal buffers to prevent re-allocations
    // in the real-time thread.
    onSignalChanged();
}

EngineBufferScaleST::~EngineBufferScaleST() {
}

void EngineBufferScaleST::setScaleParameters(double base_rate,
                                             double* pTempoRatio,
                                             double* pPitchRatio) {

    // Negative speed means we are going backwards. pitch does not affect
    // the playback direction.
    m_bBackwards = *pTempoRatio < 0;

    // It's an error to pass a rate or tempo smaller than MIN_SEEK_SPEED to
    // SoundTouch (see definition of MIN_SEEK_SPEED for more details).
    double speed_abs = fabs(*pTempoRatio);
    if (speed_abs > MAX_SEEK_SPEED) {
        speed_abs = MAX_SEEK_SPEED;
    } else if (speed_abs < MIN_SEEK_SPEED) {
        speed_abs = 0;
    }

    // Let the caller know if we clamped their value.
    *pTempoRatio = m_bBackwards ? -speed_abs : speed_abs;

    // Include baserate in rate_abs so that we do samplerate conversion as part
    // of rate adjustment.
    if (speed_abs != m_dTempoRatio) {
        // Note: A rate of zero would make Soundtouch crash,
        // this is caught in scaleBuffer()
        m_pSoundTouch->setTempo(speed_abs);
        m_dTempoRatio = speed_abs;
    }
    if (base_rate != m_dBaseRate) {
        m_pSoundTouch->setRate(base_rate);
        m_dBaseRate = base_rate;
    }

    if (*pPitchRatio != m_dPitchRatio) {
        // Note: pitch ratio must be positive
        double pitch = fabs(*pPitchRatio);
        if (pitch > 0.0) {
            m_pSoundTouch->setPitch(pitch);
        }
        m_dPitchRatio = *pPitchRatio;
    }

    // NOTE(rryan) : There used to be logic here that clear()'d when the player
    // changed direction. I removed it because this is handled by EngineBuffer.
}

void EngineBufferScaleST::onSignalChanged() {
    int backBufferSize = kBackBufferFrameSize * getOutputSignal().getChannelCount();
    if (m_bufferBack.size() == backBufferSize) {
        m_bufferBack.clear();
    } else {
        m_bufferBack = mixxx::SampleBuffer(backBufferSize);
    }
    if (!getOutputSignal().isValid()) {
        return;
    }
    m_pSoundTouch->setSampleRate(getOutputSignal().getSampleRate());
    m_pSoundTouch->setChannels(getOutputSignal().getChannelCount());

    // Setting the tempo to a very low value will force SoundTouch
    // to preallocate buffers large enough to (almost certainly)
    // avoid memory reallocations during playback.
    m_pSoundTouch->setTempo(0.1);
    m_pSoundTouch->setTempo(m_dTempoRatio);
    clear();
}

void EngineBufferScaleST::clear() {
    m_pSoundTouch->clear();

    // compensate seek offset for a rate of 1.0
    if (SoundTouch::getVersionId() < 20302) {
        DEBUG_ASSERT(SoundTouch::getVersionId() >= 20101);
        // from SoundTouch 2.3.0 the initial offset is corrected internally
        m_effectiveRate = m_dBaseRate * m_dTempoRatio;
        SampleUtil::clear(m_bufferBack.data(), m_bufferBack.size());
        m_pSoundTouch->putSamples(m_bufferBack.data(), kSeekOffsetFramesV20101);
    }
}

double EngineBufferScaleST::scaleBuffer(
        CSAMPLE* pOutputBuffer,
        SINT iOutputBufferSize) {
    if (m_dBaseRate == 0.0 || m_dTempoRatio == 0.0 || m_dPitchRatio == 0.0) {
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
        SINT received_frames = m_pSoundTouch->receiveSamples(
                read, remaining_frames);
        DEBUG_ASSERT(remaining_frames >= received_frames);
        remaining_frames -= received_frames;
        readFramesProcessed += m_effectiveRate * received_frames;
        read += getOutputSignal().frames2samples(received_frames);

        if (remaining_frames > 0) {
            // The requested setting becomes effective after all previous frames have been processed
            m_effectiveRate = m_dBaseRate * m_dTempoRatio;
            SINT iAvailSamples = m_pReadAheadManager->getNextSamples(
                    // The value doesn't matter here. All that matters is we
                    // are going forward or backward.
                    (m_bBackwards ? -1.0 : 1.0) * m_effectiveRate,
                    m_bufferBack.data(),
                    m_bufferBack.size(),
                    getOutputSignal().getChannelCount());
            SINT iAvailFrames = getOutputSignal().samples2frames(iAvailSamples);

            if (iAvailFrames > 0) {
                last_read_failed = false;
                m_pSoundTouch->putSamples(m_bufferBack.data(),
                        iAvailFrames); // bookkeeping, no copy yet
            } else {
                // We may get 0 samples once if we just hit a loop trigger, e.g.
                // when reloop_toggle jumps back to loop_in, or when moving a
                // loop causes the play position to be moved along.
                if (last_read_failed) {
                    // If we get 0 samples repeatedly, add silence that allows
                    // to flush the last samples out of Soundtouch.
                    // m_pSoundTouch->flush() must not be used, because it allocates
                    // a temporary buffer in the heap which maybe locking
                    qDebug() << "ReadAheadManager::getNextSamples() returned "
                                "zero samples repeatedly. Padding with silence.";
                    SampleUtil::clear(m_bufferBack.data(), m_bufferBack.size());
                    m_pSoundTouch->putSamples(m_bufferBack.data(), m_bufferBack.size());
                }
                last_read_failed = true;
            }
        }
    }

    // readFramesProcessed is interpreted as the total number of frames
    // consumed to produce the scaled buffer. Due to this, we do not take into
    // account directionality or starting point.
    return readFramesProcessed;
}
