
#include <QtDebug>

#include "engine/enginebufferscalelinear.h"
#include "sampleutil.h"
#include "track/keyutils.h"
#include "util/math.h"
#include "util/assert.h"

EngineBufferScaleLinear::EngineBufferScaleLinear(ReadAheadManager *pReadAheadManager)
    : EngineBufferScale(),
      m_bBackwards(false),
      m_bClear(false),
      m_dRate(1.0),
      m_dOldRate(1.0),
      m_pReadAheadManager(pReadAheadManager),
      m_dCurrentFrame(0.0),
      m_dNextFrame(0.0)
{
    for (int i=0; i<2; i++)
        m_floorSampleOld[i] = 0.0f;

    m_bufferInt = new CSAMPLE[kiLinearScaleReadAheadLength];
    m_bufferIntSize = 0;

    /*df.setFileName("mixxx-debug-scaler.csv");
    df.open(QIODevice::WriteOnly | QIODevice::Text);
    writer.setDevice(&df);
    buffer_count=0;*/
    SampleUtil::clear(m_bufferInt, kiLinearScaleReadAheadLength);
}

EngineBufferScaleLinear::~EngineBufferScaleLinear()
{
    //df.close();
    delete [] m_bufferInt;
}

void EngineBufferScaleLinear::setScaleParameters(double base_rate,
                                                 double* pTempoRatio,
                                                 double* pPitchRatio) {
    Q_UNUSED(pPitchRatio);

    m_dOldRate = m_dRate;
    m_dRate = base_rate * *pTempoRatio;

    // Determine playback direction
    m_bBackwards = m_dRate < 0.0;
}

void EngineBufferScaleLinear::clear() {
    m_bClear = true;
    // Clear out buffer and saved sample data
    m_bufferIntSize = 0;
    m_dNextFrame = 0;
    m_floorSampleOld[0] = 0;
    m_floorSampleOld[1] = 0;
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

/** Determine if we're changing directions (scratching) and then perform
    a stretch */
CSAMPLE* EngineBufferScaleLinear::getScaled(unsigned long buf_size) {
    if (m_bClear) {
        m_dOldRate = m_dRate;  // If cleared, don't interpolate rate.
        m_bClear = false;
    }
    float rate_add_old = m_dOldRate;  //Smoothly interpolate to new playback rate
    float rate_add_new = m_dRate;
    int samples_read = 0;

    // Guard against buf_size == 0
    if (static_cast<int>(buf_size) == 0) {
        return m_buffer;
    }

    if (rate_add_new * rate_add_old < 0) {
        //calculate half buffer going one way, and half buffer going
        //the other way.

        //first half: rate goes from old rate to zero
        m_dOldRate = rate_add_old;
        m_dRate = 0.0;
        m_buffer = do_scale(m_buffer, buf_size / 2, &samples_read);

        // reset prev sample so we can now read in the other direction (may not
        // be necessary?)
        int iCurSample = static_cast<int>(ceil(m_dCurrentFrame)) * 2;
        if (iCurSample + 1 < m_bufferIntSize) {
            int iNextSample = static_cast<int>(ceil(m_dNextFrame)) * 2;
            m_floorSampleOld[0] = m_bufferInt[iNextSample];
            m_floorSampleOld[1] = m_bufferInt[iNextSample + 1];
        }

        // if the buffer has extra samples, do a read so RAMAN ends up back where
        // it should be
        int extra_samples = m_bufferIntSize - iCurSample - 2;
        if (extra_samples > 0) {
            if (extra_samples % 2 != 0)
                extra_samples++;
            //qDebug() << "extra samples" << extra_samples;

            samples_read += m_pReadAheadManager->getNextSamples(
                    rate_add_new, m_bufferInt, extra_samples);
        }
        // force a buffer read:
        m_bufferIntSize=0;
        // make sure the indexes stay correct for interpolation
        m_dCurrentFrame = 0 - m_dCurrentFrame + floor(m_dCurrentFrame);
        m_dNextFrame = 1.0 - (m_dNextFrame - floor(m_dNextFrame));

        // second half: rate goes from zero to new rate
        m_dOldRate = 0.0;
        m_dRate = rate_add_new;
        // pass the address of the sample at the halfway point
        do_scale(&m_buffer[buf_size / 2], buf_size / 2, &samples_read);

        m_samplesRead = samples_read;
        return m_buffer;
    }

    CSAMPLE* result = do_scale(m_buffer, buf_size, &samples_read);
    m_samplesRead = samples_read;
    return result;
}

/** Stretch a specified buffer worth of audio using linear interpolation */
CSAMPLE* EngineBufferScaleLinear::do_scale(CSAMPLE* buf,
                                           int buf_size, int* samples_read) {
    const float rate_old = m_dOldRate;
    const float rate_new = m_dRate;
    const float rate_diff = rate_new - rate_old;

    // Update the old base rate because we only need to
    // interpolate/ramp up the pitch changes once.
    m_dOldRate = m_dRate;

    // Determine position in read_buffer to start from. (This is always 0 with
    // the new EngineBuffer implementation)

    // Guard against buf_size == 0
    if (buf_size == 0) {
        return buf;
    }

    // We check for scratch condition in the public function, so this shouldn't
    // happen
    if (rate_new * rate_old < 0) {
        qDebug() << "ERROR: EBSL did not detect scratching correctly.";
    }

    // Special case -- no scaling needed!
    if (rate_old == 1.0 && rate_new == 1.0) {
        int samples_needed = buf_size;
        CSAMPLE* write_buf = buf;

        // Use up what's left of the internal buffer.
        int iNextSample = static_cast<int>(ceil(m_dNextFrame)) * 2;
        int readSize = math_min<int>(m_bufferIntSize - iNextSample, samples_needed);
        if (readSize > 0) {
            SampleUtil::copy(write_buf, &m_bufferInt[iNextSample], readSize);
            samples_needed -= readSize;
            write_buf += readSize;
        }

        // Protection against infinite read loops when (for example) we are
        // reading from a broken file.
        bool last_read_failed = false;

        // We need to repeatedly call the RAMAN because the RAMAN does not bend
        // over backwards to satisfy our request. It assumes you will continue
        // to call getNextSamples until you receive the number of samples you
        // wanted.
        while (samples_needed > 0) {
            int read_size = m_pReadAheadManager->getNextSamples(
                    1.0, write_buf, samples_needed);
            samples_needed -= read_size;
            write_buf += read_size;

            if (read_size == 0) {
                if (last_read_failed) {
                    break;
                }
                last_read_failed = true;
            }
        }

        // Instead of counting how many samples we got from the internal buffer
        // and the RAMAN calls, just measure the difference between what we
        // requested and what we still need.
        int read_samples = buf_size - samples_needed;

        // Even though this code should not trigger for the special case in
        // getScaled for when the rate changes directions, the convention in the
        // rest of this method is that we increment samples_read rather than
        // assign it.
        *samples_read += read_samples;

        // Zero the remaining samples if we didn't fill them.
        SampleUtil::clear(write_buf, samples_needed);

        // update our class members so next time we need to scale it's ok. we do
        // blow away the fractional sample position here
        m_bufferIntSize = 0; // force buffer read
        m_dNextFrame = 0;
        m_floorSampleOld[0] = buf[read_samples-2];
        m_floorSampleOld[1] = buf[read_samples-1];
        return buf;
    }

    // Simulate the loop to estimate how many frames we need
    double frames = 0;
    // We're calculating frames = 2 samples, so divide remaining buffer by 2;
    for (int j = 0; j < buf_size / 2; ++j) {
        frames += (j * 2 * rate_diff / buf_size) + rate_old;
    }
    frames = abs(frames);

    int unscaled_frames_needed = floor(frames);

    // If the current position fraction plus the future position fraction
    // loops over 1.0, we need to round up
    if (m_dNextFrame - floor(m_dNextFrame) +
            frames - floor(frames) > 1.0) {
        unscaled_frames_needed++;
    }

    // Multiply by 2 because it is predicting mono rates, while we want a stereo
    // number of samples.
    // 0 is never the right answer
    int unscaled_samples_needed = math_max<int>(2, unscaled_frames_needed * 2);

    bool last_read_failed = false;
    CSAMPLE floor_sample[2];
    CSAMPLE ceil_sample[2];

    floor_sample[0] = 0;
    floor_sample[1] = 0;
    ceil_sample[0] = 0;
    ceil_sample[1] = 0;

    int i = 0;
    int screwups = 0;
    while (i < buf_size) {
        // shift indicies
        m_dCurrentFrame = m_dNextFrame;

        // Because our index is a float value, we're going to be interpolating
        // between two samples, a lower (prev) and upper (cur) sample.
        // If the lower sample is off the end of the buffer (values between
        // -.999 and 0), load it from the saved globals.

        // The first bounds check (< m_bufferIntSize) is probably not needed.

        if (static_cast<int>(floor(m_dCurrentFrame)) * 2 + 1 < m_bufferIntSize
                && m_dCurrentFrame >= 0.0) {
            // take floor_sample form the buffer of the previous run
            floor_sample[0] = m_bufferInt[static_cast<int>(
                    floor(m_dCurrentFrame)) * 2];
            floor_sample[1] = m_bufferInt[static_cast<int>(
                    floor(m_dCurrentFrame)) * 2 + 1];
        } else {
            // we have advanced to a new buffer in the previous run,
            // bud the floor still points to the old buffer
            // so take the cached sample, happens on slow rates
            floor_sample[0] = m_floorSampleOld[0];
            floor_sample[1] = m_floorSampleOld[1];
        }

        // if we don't have the ceil_sample in buffer, load some more
        while (static_cast<int>(ceil(m_dCurrentFrame)) * 2 + 1 >=
               m_bufferIntSize) {
            int old_bufsize = m_bufferIntSize;
            if (unscaled_samples_needed == 0) {
                unscaled_samples_needed = 2;
                screwups++;
            }

            int samples_to_read = math_min<int>(kiLinearScaleReadAheadLength,
                                                unscaled_samples_needed);

            m_bufferIntSize = m_pReadAheadManager->getNextSamples(
                    rate_new == 0 ? rate_old : rate_new,
                    m_bufferInt, samples_to_read);
            *samples_read += m_bufferIntSize;

            if (m_bufferIntSize == 0 && last_read_failed) {
                break;
            }
            last_read_failed = m_bufferIntSize == 0;

            unscaled_samples_needed -= m_bufferIntSize;

            // adapt the m_dCurrentFrame the index of the new buffer
            m_dCurrentFrame -= old_bufsize / 2.;
        }

        // I guess?
        if (last_read_failed) {
            break;
        }

        // Now that the buffer is up to date, we can get the value of the sample
        // at the floor of our position.
        if (static_cast<int>(floor(m_dCurrentFrame)) * 2 >= 0.0) {
            // the previous position is in the new buffer
            floor_sample[0] = m_bufferInt[static_cast<int>(
                    floor(m_dCurrentFrame)) * 2];
            floor_sample[1] = m_bufferInt[static_cast<int>(
                    floor(m_dCurrentFrame)) * 2 + 1];
        }

        ceil_sample[0] = m_bufferInt[static_cast<int>(
                ceil(m_dCurrentFrame)) * 2];
        ceil_sample[1] = m_bufferInt[static_cast<int>(
                ceil(m_dCurrentFrame)) * 2 + 1];

        // For the current index, what percentage is it
        // between the previous and the next?
        CSAMPLE frac = m_dCurrentFrame - floor(m_dCurrentFrame);

        // Perform linear interpolation
        buf[i] = static_cast<float>(floor_sample[0]) +
                 frac * (static_cast<float>(ceil_sample[0]) -
                 static_cast<float>(floor_sample[0]));
        buf[i + 1] = static_cast<float>(floor_sample[1]) +
                     frac * (static_cast<float>(ceil_sample[1]) -
                     static_cast<float>(floor_sample[1]));

        m_floorSampleOld[0] = floor_sample[0];
        m_floorSampleOld[1] = floor_sample[1];

        // Smooth any changes in the playback rate over one buf_size
        // samples. This prevents the change from being discontinuous and helps
        // improve sound quality.
        const double rate_add = fabs((i * rate_diff / buf_size) + rate_old);

        // increment the index for the next loop
        m_dNextFrame = m_dCurrentFrame + rate_add;
        i += 2 ;
    }

    SampleUtil::clear(&buf[i], buf_size - i);

    return buf;
}
