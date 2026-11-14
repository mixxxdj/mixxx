#include "engine/bufferscalers/enginebufferscalelinear.h"

#include <QtDebug>

#include "engine/readaheadmanager.h"
#include "moc_enginebufferscalelinear.cpp"
#include "util/assert.h"
#include "util/math.h"
#include "util/sample.h"

EngineBufferScaleLinear::EngineBufferScaleLinear(ReadAheadManager *pReadAheadManager)
    : m_pReadAheadManager(pReadAheadManager),
      m_bufferInt(SampleUtil::alloc(kiLinearScaleReadAheadLength)),
      m_bufferIntSize(0),
      m_bClear(false),
      m_dRate(1.0),
      m_dOldRate(1.0),
      m_dCurrentFrame(0.0),
      m_dNextFrame(0.0) {
    onSignalChanged();
    SampleUtil::clear(m_bufferInt, kiLinearScaleReadAheadLength);
}

EngineBufferScaleLinear::~EngineBufferScaleLinear() {
    SampleUtil::free(m_bufferInt);
}

void EngineBufferScaleLinear::onSignalChanged() {
    // We only upscale the memory allocation to reduce the likelihood of
    // impacting the real-time thread. This way, on first load of a STEM (8
    // channels), we reallocate the right size and keep it allocated till the
    // scaler is destroyed.
    const auto channelCount = getOutputSignal().getChannelCount();

    if (m_floorSampleOld.size() < channelCount) {
        m_floorSampleOld = mixxx::SampleBuffer(channelCount);
    }

    if (m_floorSample.size() < channelCount) {
        m_floorSample = mixxx::SampleBuffer(channelCount);
    }

    if (m_ceilSample.size() < channelCount) {
        m_ceilSample = mixxx::SampleBuffer(channelCount);
    }
}

void EngineBufferScaleLinear::setScaleParameters(double baseRate,
        double* pTempoRatio,
        double* pPitchRatio) {
    Q_UNUSED(pPitchRatio);

    m_dOldRate = m_dRate;
    m_dRate = baseRate * *pTempoRatio;
}

void EngineBufferScaleLinear::clear() {
    m_bClear = true;
    // Clear out buffer and saved sample data
    m_bufferIntSize = 0;
    m_dNextFrame = 0;
    onSignalChanged();
}

static inline bool directionChanged(const double rate1, const double rate2) {
    return (rate1 * rate2 < 0);
}

// Determine if we're changing directions (scratching) and then perform
// a stretch
double EngineBufferScaleLinear::scaleBuffer(
        CSAMPLE* pOutputBuffer,
        SINT iOutputBufferSize) {
    if (iOutputBufferSize == 0) {
        return 0.0;
    }

    if (m_bClear) {
        m_dOldRate = m_dRate;  // If cleared, don't interpolate rate.
        m_bClear = false;
    }

    double rateOld = m_dOldRate; // Smoothly interpolate to new playback rate
    double rateNew = m_dRate;
    double framesRead = 0;

    if (directionChanged(rateNew, rateOld)) {
        // calculate half buffer going one way, and half buffer going
        // the other way.

        // first half: rate goes from old rate to zero
        framesRead += do_direction_change(pOutputBuffer, iOutputBufferSize, rateOld, rateNew);

        // second half: rate goes from zero to new rate
        // pass the address of the frame at the halfway point
        const SINT frameOffset = getOutputSignal().samples2frames(iOutputBufferSize) / 2;
        const SINT sampleOffset = getOutputSignal().frames2samples(frameOffset);

        framesRead += do_scale(pOutputBuffer + sampleOffset, iOutputBufferSize - sampleOffset);
    } else {
        framesRead += do_scale(pOutputBuffer, iOutputBufferSize);
    }

    return framesRead;
}

double EngineBufferScaleLinear::do_direction_change(
        CSAMPLE* buf, SINT bufSize, double rateOld, double rateNew) {
    double framesRead = 0;

    // first half: rate goes from old rate to zero
    m_dOldRate = rateOld;
    m_dRate = 0.0;
    framesRead += do_scale(buf, getOutputSignal().samples2frames(bufSize));

    // reset m_floorSampleOld in a way as we were coming from
    // the other direction
    SINT iNextSample = getOutputSignal().frames2samples(static_cast<SINT>(ceil(m_dNextFrame)));
    int chCount = getOutputSignal().getChannelCount();

    if (iNextSample >= 0 && iNextSample + chCount <= m_bufferIntSize) {
        SampleUtil::copy(m_floorSampleOld.data(), &m_bufferInt[iNextSample], chCount);
    } else {
        SampleUtil::clear(m_floorSampleOld.data(), chCount);
    }

    // if the buffer has extra samples, do a read so RAMAN ends up back where
    // it should be
    SINT iCurSample = getOutputSignal().frames2samples(static_cast<SINT>(ceil(m_dCurrentFrame)));
    SINT extraSamples = m_bufferIntSize - iCurSample - getOutputSignal().getChannelCount();

    if (extraSamples > 0) {
        if (extraSamples % getOutputSignal().getChannelCount() != 0) {
            // extra samples should include the whole frame
            extraSamples -= extraSamples % getOutputSignal().getChannelCount();
            extraSamples += getOutputSignal().getChannelCount();
        }

        // qDebug() << "extra samples" << extra_samples;

        SINT next_samples_read = m_pReadAheadManager->getNextSamples(
                rateNew, m_bufferInt, extraSamples, getOutputSignal().getChannelCount());
        framesRead += static_cast<double>(getOutputSignal().samples2frames(next_samples_read));
    }

    // force a buffer read:
    m_bufferIntSize = 0;

    // make sure the indexes stay correct for interpolation
    m_dCurrentFrame = 0.0 - (m_dCurrentFrame - floor(m_dCurrentFrame));
    m_dNextFrame = 1.0 - (m_dNextFrame - floor(m_dNextFrame));

    // second half: rate goes from zero to new rate
    m_dOldRate = 0.0;
    m_dRate = rateNew;

    return framesRead;
}

SINT EngineBufferScaleLinear::do_copy(CSAMPLE* buf, SINT bufSize) {
    SINT samplesNeeded = bufSize;
    CSAMPLE* writeBuf = buf;

    // Use up what's left of the internal buffer.
    auto iNextFrame = static_cast<SINT>(ceil(m_dNextFrame));
    SINT iNextSample = math_max<SINT>(getOutputSignal().frames2samples(iNextFrame), 0);
    SINT readSize = math_min<SINT>(m_bufferIntSize - iNextSample, samplesNeeded);

    if (readSize > 0) {
        SampleUtil::copy(writeBuf, &m_bufferInt[iNextSample], readSize);
        samplesNeeded -= readSize;
        writeBuf += readSize;
    }

    // Protection against infinite read loops when (for example) we are
    // reading from a broken file.
    int readFailedCount = 0;

    // We need to repeatedly call the RAMAN because the RAMAN does not bend
    // over backwards to satisfy our request. It assumes you will continue
    // to call getNextSamples until you receive the number of samples you
    // wanted.
    while (samplesNeeded > 0) {
        readSize = m_pReadAheadManager->getNextSamples(m_dRate,
                writeBuf,
                samplesNeeded,
                getOutputSignal().getChannelCount());

        if (readSize == 0) {
            if (++readFailedCount > 1) {
                break;
            } else {
                continue;
            }
        }

        samplesNeeded -= readSize;
        writeBuf += readSize;
    }

    // Instead of counting how many samples we got from the internal buffer
    // and the RAMAN calls, just measure the difference between what we
    // requested and what we still need.
    SINT readSamples = bufSize - samplesNeeded;

    // Zero the remaining samples if we didn't fill them.
    SampleUtil::clear(writeBuf, samplesNeeded);

    // update our class members so next time we need to scale it's ok. we do
    // blow away the fractional sample position here
    m_bufferIntSize = 0; // force buffer read
    m_dNextFrame = 0;
    int chCount = getOutputSignal().getChannelCount();

    if (readSamples > chCount - 1) {
        SampleUtil::copy(m_floorSampleOld.data(), &buf[readSamples - chCount], chCount);
    }

    return readSamples;
}

static inline void interpolateFrame(CSAMPLE* pOutput,
        const CSAMPLE* pFloorSample,
        const CSAMPLE* pCeilSample,
        const CSAMPLE frac,
        const int chCount) {
    for (int chIdx = 0; chIdx < chCount; chIdx++) {
        pOutput[chIdx] = pFloorSample[chIdx] +
                frac * (pCeilSample[chIdx] - pFloorSample[chIdx]);
    }
}

void EngineBufferScaleLinear::do_refill_buffer(double rate,
        SINT* pUnscaledFramesNeeded,
        double* pStartFrame,
        SINT* pCurrentFrameFloor) {
    const int chCount = getOutputSignal().getChannelCount();
    SINT sampleCount = 0;

    do {
        SINT oldBufferFrames = getOutputSignal().samples2frames(m_bufferIntSize);

        if (*pUnscaledFramesNeeded == 0) {
            // protection against infinite loop
            // This may happen due to double precision issues
            ++(*pUnscaledFramesNeeded);
        }

        SINT samplesToRead = math_min<SINT>(
                kiLinearScaleReadAheadLength,
                getOutputSignal().frames2samples(*pUnscaledFramesNeeded));

        m_bufferIntSize = m_pReadAheadManager->getNextSamples(
                rate,
                m_bufferInt,
                samplesToRead,
                getOutputSignal().getChannelCount());
        // Note we may get 0 samples once if we just hit a loop trigger,
        // e.g. when reloop_toggle jumps back to loop_in, or when
        // moving a loop causes the play position to be moved along.

        *pUnscaledFramesNeeded -= getOutputSignal().samples2frames(m_bufferIntSize);

        // adapt the frames values that are still relative to the old buffer for the new one
        m_dCurrentFrame -= static_cast<double>(oldBufferFrames);
        *pStartFrame -= static_cast<double>(oldBufferFrames);
        *pCurrentFrameFloor -= oldBufferFrames;

        sampleCount = getOutputSignal().frames2samples(*pCurrentFrameFloor);
    } while (sampleCount + 2 * chCount - 1 >= m_bufferIntSize);
}

SINT EngineBufferScaleLinear::do_fetch_samples(double rate,
        SINT* pUnscaledFramesNeeded,
        double* pStartFrame) {
    const int chCount = getOutputSignal().getChannelCount();

    // Because our index is a float value, we're going to be interpolating
    // between two samples, a lower (prev) and upper (cur) sample.
    // If the lower sample is off the end of the buffer (values between
    // -.999 and 0), load it from the saved globals.

    // The first bounds check (< m_bufferIntSize) is probably not needed.

    auto currentFrameFloor = static_cast<SINT>(floor(m_dCurrentFrame));

    auto sampleCount = static_cast<int>(getOutputSignal().frames2samples(currentFrameFloor));
    if (currentFrameFloor < 0) {
        // we have advanced to a new buffer in the previous run,
        // but the floor still points to the old buffer
        // so take the cached sample, this happens on slow rates
        SampleUtil::copy(m_floorSample.data(), m_floorSampleOld.data(), chCount);
        SampleUtil::copy(m_ceilSample.data(), m_bufferInt, chCount);
    } else if (sampleCount + 2 * chCount - 1 < m_bufferIntSize) {
        // take floorSample form the buffer of the previous run
        SampleUtil::copy(m_floorSample.data(), &m_bufferInt[sampleCount], chCount);
        SampleUtil::copy(m_ceilSample.data(), &m_bufferInt[sampleCount + chCount], chCount);
    } else {
        // if we don't have the ceilSample in buffer, load some more

        if (sampleCount + chCount - 1 < m_bufferIntSize) {
            // take floorSample form the buffer of the previous run
            SampleUtil::copy(m_floorSample.data(), &m_bufferInt[sampleCount], chCount);
        }

        do_refill_buffer(rate,
                pUnscaledFramesNeeded,
                pStartFrame,
                &currentFrameFloor);

        sampleCount = static_cast<int>(getOutputSignal().frames2samples(currentFrameFloor));

        // Now that the buffer is up to date, we can get the value of the sample
        // at the floor of our position.
        if (currentFrameFloor >= 0) {
            // the previous position is in the new buffer
            SampleUtil::copy(m_floorSample.data(), &m_bufferInt[sampleCount], chCount);
        }

        SampleUtil::copy(m_ceilSample.data(), &m_bufferInt[sampleCount + chCount], chCount);
    }

    return currentFrameFloor;
}

// Stretch a specified buffer worth of audio using linear interpolation
double EngineBufferScaleLinear::do_scale(CSAMPLE* buf, SINT bufSize) {
    double rateOld = m_dOldRate;
    const double rateNew = m_dRate;
    const double rateDiff = rateNew - rateOld;

    // Update the old base rate because we only need to
    // interpolate/ramp up the pitch changes once.
    m_dOldRate = m_dRate;

    // Determine position in read_buffer to start from. (This is always 0 with
    // the new EngineBuffer implementation)

    // We special case direction change in the calling function, so this
    // shouldn't happen
    VERIFY_OR_DEBUG_ASSERT(!directionChanged(rateNew, rateOld)) {
        // We cannot change direction here.
        qDebug() << "EBSL::do_scale() can't change direction";
        rateOld = 0;
    }

    // Special case -- no scaling needed!
    if (rateDiff == 0 && (rateNew == 1.0 || rateNew == -1.0)) {
        return static_cast<double>(do_copy(buf, bufSize));
    }

    // Simulate the loop to estimate how many frames we need
    double frames = 0;
    const SINT bufferSizeFrames = getOutputSignal().samples2frames(bufSize);
    const double rateDelta = rateDiff / static_cast<double>(bufferSizeFrames);

    // Use Gaussian sum formula (n(n+1))/2 for
    // for (int j = 0; j < bufferSizeFrames; ++j) {
    //     frames += (j * rateDelta) + rateOld;
    // }

    frames = static_cast<double>(bufferSizeFrames - 1) *
            static_cast<double>(bufferSizeFrames) / 2.0;
    frames *= rateDelta;
    frames += rateOld * static_cast<double>(bufferSizeFrames);
    frames = fabs(frames);

    // Intentional integer rounding: increases by one if the fractional part of
    // m_dNextFrame and frames are greater than one"
    auto unscaledFramesNeeded = static_cast<SINT>(frames +
            m_dNextFrame - floor(m_dNextFrame));

    int chCount = getOutputSignal().getChannelCount();
    m_floorSample.clear();
    m_ceilSample.clear();

    double startFrame = m_dNextFrame;
    SINT i = 0;

    double rateAdd = fabs(rateOld);
    const double rateDeltaAbs =
            rateOld < 0 || rateNew < 0 ? -rateDelta : rateDelta;
    const double readRate = rateNew == 0 ? rateOld : rateNew;

    // Hot frame loop
    while (i < bufSize) {
        // shift indices
        m_dCurrentFrame = m_dNextFrame;

        const SINT currentFrameFloor = do_fetch_samples(
                readRate, &unscaledFramesNeeded, &startFrame);

        // For the current index, what percentage is it
        // between the previous and the next?
        CSAMPLE frac = static_cast<CSAMPLE>(m_dCurrentFrame) -
                static_cast<CSAMPLE>(currentFrameFloor);

        // Perform linear interpolation
        interpolateFrame(&buf[i], m_floorSample.data(), m_ceilSample.data(), frac, chCount);

        m_floorSampleOld.swap(m_floorSample);

        // increment the index for the next loop
        m_dNextFrame = m_dCurrentFrame + rateAdd;

        // Smooth any changes in the playback rate over one buf_size
        // samples. This prevents the change from being discontinuous and helps
        // improve sound quality.
        rateAdd += rateDeltaAbs;
        i += chCount;
    }

    SampleUtil::clear(&buf[i], bufSize - i);

    return m_dNextFrame - startFrame;
}
