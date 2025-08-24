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
    m_floorSampleOld = mixxx::SampleBuffer(getOutputSignal().getChannelCount());
    m_floorSample = mixxx::SampleBuffer(getOutputSignal().getChannelCount());
    m_ceilSample = mixxx::SampleBuffer(getOutputSignal().getChannelCount());
}

void EngineBufferScaleLinear::setScaleParameters(double base_rate,
                                                 double* pTempoRatio,
                                                 double* pPitchRatio) {
    Q_UNUSED(pPitchRatio);

    m_dOldRate = m_dRate;
    m_dRate = base_rate * *pTempoRatio;
}

void EngineBufferScaleLinear::clear() {
    m_bClear = true;
    // Clear out buffer and saved sample data
    m_bufferIntSize = 0;
    m_dNextFrame = 0;
    onSignalChanged();
}

// laurent de soras - punked from musicdsp.org (mad props)
inline float hermite4(float frac_pos, float xm1, float x0, float x1, float x2)
{
    const float c = (x1 - xm1) * 0.5f;
    const float v = x0 - x1;
    const float w = c + v;
    const float a = w + v + (x2 - x0) * 0.5f;
    const float b_neg = w + a;
    return ((((a * frac_pos) - b_neg) * frac_pos + c) * frac_pos + x0);
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
    double rate_add_old = m_dOldRate; // Smoothly interpolate to new playback rate
    double rate_add_new = m_dRate;
    double frames_read = 0;

    if (rate_add_new * rate_add_old < 0) {
        // Direction has changed!
        // calculate half buffer going one way, and half buffer going
        // the other way.

        // first half: rate goes from old rate to zero
        m_dOldRate = rate_add_old;
        m_dRate = 0.0;
        // updated for PreMix + stems -->
        // frames_read += do_scale(pOutputBuffer,
        // getOutputSignal().samples2frames(iOutputBufferSize));
        const SINT channels = getOutputSignal().getChannelCount().value(); // 10 now
        const SINT safeBufSize = iOutputBufferSize - (iOutputBufferSize % channels);

        frames_read = do_scale(pOutputBuffer, getOutputSignal().samples2frames(safeBufSize));
        // updated for PreMix + stems <--

        // reset m_floorSampleOld in a way as we were coming from
        // the other direction
        SINT iNextSample = getOutputSignal().frames2samples(static_cast<SINT>(ceil(m_dNextFrame)));
        // changed for premix
        // int chCount = getOutputSignal().getChannelCount();
        const SINT chCount = getOutputSignal().getChannelCount().value();
        if (iNextSample >= 0 && iNextSample + chCount <= m_bufferIntSize) {
            SampleUtil::copy(m_floorSampleOld.data(), &m_bufferInt[iNextSample], chCount);
        } else {
            SampleUtil::clear(m_floorSampleOld.data(), chCount);
        }

        // if the buffer has extra samples, do a read so RAMAN ends up back where
        // it should be
        SINT iCurSample = getOutputSignal().frames2samples(static_cast<SINT>(ceil(m_dCurrentFrame)));
        SINT extra_samples = m_bufferIntSize - iCurSample - getOutputSignal().getChannelCount();
        if (extra_samples > 0) {
            if (extra_samples % getOutputSignal().getChannelCount() != 0) {
                // extra samples should include the whole frame
                extra_samples -= extra_samples % getOutputSignal().getChannelCount();
                extra_samples += getOutputSignal().getChannelCount();
            }
            //qDebug() << "extra samples" << extra_samples;

            SINT next_samples_read = m_pReadAheadManager->getNextSamples(
                    rate_add_new, m_bufferInt, extra_samples, getOutputSignal().getChannelCount());

            frames_read += getOutputSignal().samples2frames(next_samples_read);
        }
        // force a buffer read:
        m_bufferIntSize = 0;
        // make sure the indexes stay correct for interpolation
        m_dCurrentFrame = 0.0 - (m_dCurrentFrame - floor(m_dCurrentFrame));
        m_dNextFrame = 1.0 - (m_dNextFrame - floor(m_dNextFrame));
        // Changed for PreMix -->
        // frames_read += do_scale(pOutputBuffer,
        // getOutputSignal().samples2frames(iOutputBufferSize)); const SINT
        // chCount = getOutputSignal().getChannelCount().value(); SINT
        // safeBufSize = iOutputBufferSize - (iOutputBufferSize % chCount);
        frames_read += do_scale(pOutputBuffer, safeBufSize);
        // Changed for PreMix <--

        // second half: rate goes from zero to new rate
        m_dOldRate = 0.0;
        m_dRate = rate_add_new;
        // pass the address of the frame at the halfway point
        SINT frameOffset =  getOutputSignal().samples2frames(iOutputBufferSize) / 2;
        SINT sampleOffset = getOutputSignal().frames2samples(frameOffset);
        // Changed for PreMix -->
        // frames_read += do_scale(pOutputBuffer + sampleOffset, iOutputBufferSize - sampleOffset);
        SINT secondHalfSize = safeBufSize - sampleOffset;
        secondHalfSize -= secondHalfSize % chCount;
        frames_read += do_scale(pOutputBuffer + sampleOffset, secondHalfSize);
        // Changed for PreMix <--

    } else {
        frames_read += do_scale(pOutputBuffer, iOutputBufferSize);
    }
    return frames_read;
}

// changed for PreMix -->
SINT EngineBufferScaleLinear::do_copy(CSAMPLE* buf, SINT buf_size) {
    SINT samples_needed = buf_size;
    CSAMPLE* write_buf = buf;
    const int chCount = getOutputSignal().getChannelCount(); // 10 for premix + 4 stems

    // Align samples_needed to full frames
    samples_needed -= samples_needed % chCount;

    // Use up what's left of the internal buffer.
    SINT iNextFrame = static_cast<SINT>(ceil(m_dNextFrame));
    SINT iNextSample = math_max<SINT>(getOutputSignal().frames2samples(iNextFrame), 0);
    SINT readSize = math_min<SINT>(m_bufferIntSize - iNextSample, samples_needed);
    if (readSize > 0) {
        SampleUtil::copy(write_buf, &m_bufferInt[iNextSample], readSize);
        samples_needed -= readSize;
        write_buf += readSize;
    }

    // Protection against infinite read loops when RAMAN cannot fill enough
    int read_failed_count = 0;
    while (samples_needed > 0) {
        SINT read_size = m_pReadAheadManager->getNextSamples(
                m_dRate,
                write_buf,
                samples_needed,
                mixxx::audio::ChannelCount(chCount));
        // SINT read_size = m_pReadAheadManager->getNextSamples(
        //         m_dRate,
        //         write_buf,
        //         samples_needed,
        //         chCount);

        if (read_size == 0) {
            if (++read_failed_count > 1)
                break;
            else
                continue;
        }

        samples_needed -= read_size;
        write_buf += read_size;
    }

    // Compute how many samples were actually read
    SINT read_samples = buf_size - samples_needed;

    // Ensure remaining samples are multiples of chCount before clearing
    if (samples_needed > 0) {
        SINT samples_to_clear = samples_needed - (samples_needed % chCount);
        if (samples_to_clear > 0) {
            SampleUtil::clear(write_buf, samples_to_clear);
        }
    }

    // Reset buffer state
    m_bufferIntSize = 0;
    m_dNextFrame = 0;

    // Store last frame for interpolation
    if (read_samples > chCount - 1) {
        SampleUtil::copy(m_floorSampleOld.data(), &buf[read_samples - chCount], chCount);
    }

    return read_samples;
}

// SINT EngineBufferScaleLinear::do_copy(CSAMPLE* buf, SINT buf_size) {
//     SINT samples_needed = buf_size;
//     CSAMPLE* write_buf = buf;
//     // Use up what's left of the internal buffer.
//     SINT iNextFrame = static_cast<SINT>(ceil(m_dNextFrame));
//     SINT iNextSample = math_max<SINT>(getOutputSignal().frames2samples(iNextFrame), 0);
//     SINT readSize = math_min<SINT>(m_bufferIntSize - iNextSample, samples_needed);
//     if (readSize > 0) {
//         SampleUtil::copy(write_buf, &m_bufferInt[iNextSample], readSize);
//         samples_needed -= readSize;
//         write_buf += readSize;
//     }
//     // Protection against infinite read loops when (for example) we are
//     // reading from a broken file.
//     int read_failed_count = 0;
//     // We need to repeatedly call the RAMAN because the RAMAN does not bend
//     // over backwards to satisfy our request. It assumes you will continue
//     // to call getNextSamples until you receive the number of samples you
//     // wanted.
//     while (samples_needed > 0) {
//         SINT read_size = m_pReadAheadManager->getNextSamples(m_dRate,
//                 write_buf,
//                 samples_needed,
//                 getOutputSignal().getChannelCount());
//         if (read_size == 0) {
//             if (++read_failed_count > 1) {
//                 break;
//             } else {
//                 continue;
//             }
//         }
//         samples_needed -= read_size;
//         write_buf += read_size;
//     }
//
//     // Instead of counting how many samples we got from the internal buffer
//     // and the RAMAN calls, just measure the difference between what we
//     // requested and what we still need.
//     SINT read_samples = buf_size - samples_needed;
//     // Zero the remaining samples if we didn't fill them.
//     SampleUtil::clear(write_buf, samples_needed);
//     // update our class members so next time we need to scale it's ok. we do
//     // blow away the fractional sample position here
//     m_bufferIntSize = 0; // force buffer read
//     m_dNextFrame = 0;
//     int chCount = getOutputSignal().getChannelCount();
//     if (read_samples > chCount - 1) {
//         SampleUtil::copy(m_floorSampleOld.data(), &buf[read_samples - chCount], chCount);
//     }
//     return read_samples;
// }

// Stretch a specified buffer worth of audio using linear interpolation
double EngineBufferScaleLinear::do_scale(CSAMPLE* buf, SINT buf_size) {
    double rate_old = m_dOldRate;
    const double rate_new = m_dRate;
    const double rate_diff = rate_new - rate_old;

    // Update the old base rate because we only need to
    // interpolate/ramp up the pitch changes once.
    m_dOldRate = m_dRate;

    // Determine position in read_buffer to start from. (This is always 0 with
    // the new EngineBuffer implementation)

    // We special case direction change in the calling function, so this
    // shouldn't happen
    VERIFY_OR_DEBUG_ASSERT(rate_new * rate_old >= 0) {
        // We cannot change direction here.
        qDebug() << "EBSL::do_scale() can't change direction";
        rate_old = 0;
    }

    // Special case -- no scaling needed!
    if (rate_diff == 0 && (rate_new == 1.0 || rate_new == -1.0)) {
        return do_copy(buf, buf_size);
    }

    // Simulate the loop to estimate how many frames we need
    double frames = 0;
    const SINT bufferSizeFrames = getOutputSignal().samples2frames(buf_size);
    const double rate_delta = rate_diff / bufferSizeFrames;
    // use Gaussian sum formula (n(n+1))/2 for
    //for (int j = 0; j < bufferSizeFrames; ++j) {
    //    frames += (j * rate_delta) + rate_old;
    //}
    frames = (bufferSizeFrames - 1) * bufferSizeFrames / 2.0;
    frames *= rate_delta;
    frames += rate_old * bufferSizeFrames;
    frames = fabs(frames);

    // Intentional integer rounding: increases by one if the fractional part of
    // m_dNextFrame and frames are greater than one"
    SINT unscaled_frames_needed = static_cast<SINT>(frames +
            m_dNextFrame - floor(m_dNextFrame));

    int chCount = getOutputSignal().getChannelCount();
    m_floorSample.clear();
    m_ceilSample.clear();

    double startFrame = m_dNextFrame;
    SINT i = 0;

    double rate_add = fabs(rate_old);
    const double rate_delta_abs =
            rate_old < 0 || rate_new < 0 ? -rate_delta : rate_delta;

    // Hot frame loop
    while (i < buf_size) {
        // shift indices
        m_dCurrentFrame = m_dNextFrame;

        // Because our index is a float value, we're going to be interpolating
        // between two samples, a lower (prev) and upper (cur) sample.
        // If the lower sample is off the end of the buffer (values between
        // -.999 and 0), load it from the saved globals.

        // The first bounds check (< m_bufferIntSize) is probably not needed.

        SINT currentFrameFloor = static_cast<SINT>(floor(m_dCurrentFrame));

        int sampleCount = getOutputSignal().frames2samples(currentFrameFloor);
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

            do {
                SINT oldBufferFrames = getOutputSignal().samples2frames(m_bufferIntSize);
                if (unscaled_frames_needed == 0) {
                    // protection against infinite loop
                    // This may happen due to double precision issues
                    ++unscaled_frames_needed;
                }

                SINT samples_to_read = math_min<SINT>(
                        kiLinearScaleReadAheadLength,
                        getOutputSignal().frames2samples(unscaled_frames_needed));

                m_bufferIntSize = m_pReadAheadManager->getNextSamples(
                        rate_new == 0 ? rate_old : rate_new,
                        m_bufferInt,
                        samples_to_read,
                        getOutputSignal().getChannelCount());
                // Note we may get 0 samples once if we just hit a loop trigger,
                // e.g. when reloop_toggle jumps back to loop_in, or when
                // moving a loop causes the play position to be moved along.

                unscaled_frames_needed -= getOutputSignal().samples2frames(m_bufferIntSize);

                // adapt the frames values that are still relative to the old buffer for the new one
                m_dCurrentFrame -= oldBufferFrames;
                startFrame -= oldBufferFrames;
                currentFrameFloor -= oldBufferFrames;

                sampleCount = getOutputSignal().frames2samples(currentFrameFloor);
            } while (sampleCount + 2 * chCount - 1 >= m_bufferIntSize);

            // Now that the buffer is up to date, we can get the value of the sample
            // at the floor of our position.
            if (currentFrameFloor >= 0) {
                // the previous position is in the new buffer
                SampleUtil::copy(m_floorSample.data(), &m_bufferInt[sampleCount], chCount);
            }
            SampleUtil::copy(m_ceilSample.data(), &m_bufferInt[sampleCount + chCount], chCount);
        }

        // For the current index, what percentage is it
        // between the previous and the next?
        CSAMPLE frac = static_cast<CSAMPLE>(m_dCurrentFrame) - currentFrameFloor;

        // Perform linear interpolation
        // changed for PreMix -->
        // for (int chIdx = 0; chIdx < chCount; chIdx++) {
        //    buf[i + chIdx] = m_floorSample[chIdx] +
        //            frac * (m_ceilSample[chIdx] - m_floorSample[chIdx]);
        //}
        if (i + chCount <= buf_size) {
            for (int chIdx = 0; chIdx < chCount; chIdx++) {
                buf[i + chIdx] = m_floorSample[chIdx] +
                        frac * (m_ceilSample[chIdx] - m_floorSample[chIdx]);
            }
        }
        // changed for PreMix -->

        m_floorSampleOld.swap(m_floorSample);

        // increment the index for the next loop
        m_dNextFrame = m_dCurrentFrame + rate_add;

        // Smooth any changes in the playback rate over one buf_size
        // samples. This prevents the change from being discontinuous and helps
        // improve sound quality.
        rate_add += rate_delta_abs;
        i += chCount;
    }

    // Changed for PReMix -->
    // SampleUtil::clear(&buf[i], buf_size - i);
    SINT remaining = buf_size - i;
    if (remaining > 0) {
        remaining -= remaining % chCount; // make multiple of channels
        SampleUtil::clear(&buf[i], remaining);
    }
    // Changed for PReMix <--

    return m_dNextFrame - startFrame;
}
