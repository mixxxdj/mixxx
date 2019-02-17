#include "engine/bufferscalers/enginebufferscalest.h"

// Fixes redefinition warnings from SoundTouch.
#undef TRUE
#undef FALSE
#include <SoundTouch.h>

#include "control/controlobject.h"
#include "engine/engineobject.h"
#include "engine/readaheadmanager.h"
#include "track/keyutils.h"
#include "util/math.h"
#include "util/sample.h"

using namespace soundtouch;

namespace {

// Due to filtering and oversampling, SoundTouch is some samples behind.
// The value below was experimental identified using a saw signal and SoundTouch 1.8
// at a speed of 1.0
// 0.918 (upscaling 44.1 kHz to 48 kHz) will produce an additional offset of 3 Frames
// 0.459 (upscaling 44.1 kHz to 96 kHz) will produce an additional offset of 18 Frames
// (Rubberband does not suffer this issue)
const SINT kSeekOffsetFrames = 519;

}  // namespace

EngineBufferScaleST::EngineBufferScaleST(ReadAheadManager *pReadAheadManager)
    : m_pReadAheadManager(pReadAheadManager),
      m_pSoundTouch(std::make_unique<soundtouch::SoundTouch>()),
      buffer_back_size(getAudioSignal().frames2samples(kSeekOffsetFrames)),
      buffer_back(SampleUtil::alloc(buffer_back_size)),
      m_bBackwards(false) {
    DEBUG_ASSERT(getAudioSignal().verifyReadable());
    m_pSoundTouch->setChannels(getAudioSignal().channelCount());
    m_pSoundTouch->setSampleRate(getAudioSignal().sampleRate());
    m_pSoundTouch->setRate(m_dBaseRate);
    m_pSoundTouch->setPitch(1.0);
    m_pSoundTouch->setSetting(SETTING_USE_QUICKSEEK, 1);

    // Setting the tempo to a very low value will force SoundTouch
    // to preallocate buffers large enough to (almost certainly)
    // avoid memory reallocations during playback.
    m_pSoundTouch->setTempo(0.1);
    m_pSoundTouch->putSamples(buffer_back, kSeekOffsetFrames);
    m_pSoundTouch->clear();
    m_pSoundTouch->setTempo(m_dTempoRatio);
}

EngineBufferScaleST::~EngineBufferScaleST() {
    SampleUtil::free(buffer_back);
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

void EngineBufferScaleST::setSampleRate(SINT iSampleRate) {
    EngineBufferScale::setSampleRate(iSampleRate);
    m_pSoundTouch->setSampleRate(iSampleRate);
}

void EngineBufferScaleST::clear() {
    m_pSoundTouch->clear();

    // compensate seek offset for a rate of 1.0
    SampleUtil::clear(buffer_back, getAudioSignal().frames2samples(kSeekOffsetFrames));
    m_pSoundTouch->putSamples(buffer_back, kSeekOffsetFrames);
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

    SINT total_received_frames = 0;
    SINT total_read_frames = 0;

    SINT remaining_frames = getAudioSignal().samples2frames(iOutputBufferSize);
    CSAMPLE* read = pOutputBuffer;
    bool last_read_failed = false;
    while (remaining_frames > 0) {
        SINT received_frames = m_pSoundTouch->receiveSamples(
                read, remaining_frames);
        DEBUG_ASSERT(remaining_frames >= received_frames);
        remaining_frames -= received_frames;
        total_received_frames += received_frames;
        read += getAudioSignal().frames2samples(received_frames);

        if (remaining_frames > 0) {
            SINT iAvailSamples = m_pReadAheadManager->getNextSamples(
                        // The value doesn't matter here. All that matters is we
                        // are going forward or backward.
                        (m_bBackwards ? -1.0 : 1.0) * m_dBaseRate * m_dTempoRatio,
                        buffer_back,
                        buffer_back_size);
            SINT iAvailFrames = getAudioSignal().samples2frames(iAvailSamples);

            if (iAvailFrames > 0) {
                last_read_failed = false;
                total_read_frames += iAvailFrames;
                m_pSoundTouch->putSamples(buffer_back, iAvailFrames);
            } else {
                if (last_read_failed) {
                    m_pSoundTouch->flush();
                    break; // exit loop after failure
                }
                last_read_failed = true;
            }
        }
    }

    // framesRead is interpreted as the total number of virtual sample frames
    // consumed to produce the scaled buffer. Due to this, we do not take into
    // account directionality or starting point.
    // NOTE(rryan): Why no m_dPitchAdjust here? SoundTouch implements pitch
    // shifting as a tempo shift of (1/m_dPitchAdjust) and a rate shift of
    // (*m_dPitchAdjust) so these two cancel out.
    double framesRead = m_dBaseRate * m_dTempoRatio * total_received_frames;

    return framesRead;
}
