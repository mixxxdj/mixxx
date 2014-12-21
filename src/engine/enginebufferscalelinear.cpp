/***************************************************************************
                          enginebufferscalelinear.cpp  -  description
                            -------------------
    begin                : Mon Apr 14 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

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
      m_dCurSampleIndex(0.0),
      m_dNextSampleIndex(0.0)
{
    for (int i=0; i<2; i++)
        m_fPrevSample[i] = 0.0f;

    buffer_int = new CSAMPLE[kiLinearScaleReadAheadLength];
    buffer_int_size = 0;

    /*df.setFileName("mixxx-debug-scaler.csv");
    df.open(QIODevice::WriteOnly | QIODevice::Text);
    writer.setDevice(&df);
    buffer_count=0;*/
    SampleUtil::clear(buffer_int, kiLinearScaleReadAheadLength);
}

EngineBufferScaleLinear::~EngineBufferScaleLinear()
{
    //df.close();
    delete [] buffer_int;
}

void EngineBufferScaleLinear::setScaleParameters(int iSampleRate,
                                                 double base_rate,
                                                 bool speed_affects_pitch,
                                                 double* speed_adjust,
                                                 double* pitch_adjust) {
    m_iSampleRate = iSampleRate;

    // EBSL doesn't support pitch-independent tempo adjustment.
    if (!speed_affects_pitch) {
        qWarning() << "WARNING: EngineBufferScaleLinear requested to adjust"
                   << "tempo independent of pitch. Ignoring.";
    }

    m_dOldRate = m_dRate;
    // pitch_adjust is measured in octave change. Convert it to a rate using
    // octaveChangeToPowerOf2.
    m_dRate = base_rate * *speed_adjust * KeyUtils::octaveChangeToPowerOf2(*pitch_adjust);

    // Determine playback direction
    m_bBackwards = m_dRate < 0.0;
}

void EngineBufferScaleLinear::clear() {
    m_bClear = true;
    // Clear out buffer and saved sample data
    buffer_int_size = 0;
    m_dNextSampleIndex = 0;
    m_fPrevSample[0] = 0;
    m_fPrevSample[1] = 0;
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
    m_samplesRead = 0;

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
        m_buffer = do_scale(m_buffer, buf_size/2, &samples_read);

        // reset prev sample so we can now read in the other direction (may not
        // be necessary?)
        int iCurSample = static_cast<int>(ceil(m_dCurSampleIndex)) * 2;
        if (iCurSample + 1 < buffer_int_size) {
            int iNextSample = static_cast<int>(ceil(m_dNextSampleIndex)) * 2;
            m_fPrevSample[0] = buffer_int[iNextSample];
            m_fPrevSample[1] = buffer_int[iNextSample + 1];
        }

        // if the buffer has extra samples, do a read so RAMAN ends up back where
        // it should be
        int extra_samples = buffer_int_size - iCurSample - 2;
        if (extra_samples > 0) {
            if (extra_samples % 2 != 0)
                extra_samples++;
            //qDebug() << "extra samples" << extra_samples;

            samples_read += m_pReadAheadManager->getNextSamples(
                rate_add_new, buffer_int, extra_samples);
        }
        //force a buffer read:
        buffer_int_size=0;
        //make sure the indexes stay correct for interpolation
        m_dCurSampleIndex = 0 - m_dCurSampleIndex + floor(m_dCurSampleIndex);
        m_dNextSampleIndex = 1.0 - (m_dNextSampleIndex - floor(m_dNextSampleIndex));

        //second half: rate goes from zero to new rate
        m_dOldRate = 0.0;
        m_dRate = rate_add_new;
        //pass the address of the sample at the halfway point
        do_scale(&m_buffer[buf_size/2], buf_size/2, &samples_read);

        m_samplesRead = samples_read;
        return m_buffer;
    }

    CSAMPLE* result = do_scale(m_buffer, buf_size, &samples_read);
    m_samplesRead = samples_read;
    return result;
}

/** Stretch a specified buffer worth of audio using linear interpolation */
CSAMPLE* EngineBufferScaleLinear::do_scale(CSAMPLE* buf,
                                           unsigned long buf_size, int* samples_read) {
    float rate_add_old = m_dOldRate;
    float rate_add_new = m_dRate;
    float rate_add_diff = rate_add_new - rate_add_old;

    //Update the old base rate because we only need to
    //interpolate/ramp up the pitch changes once.
    m_dOldRate = m_dRate;

    // Determine position in read_buffer to start from. (This is always 0 with
    // the new EngineBuffer implementation)

    int iRateLerpLength = static_cast<int>(buf_size);

    // Guard against buf_size == 0
    if (iRateLerpLength == 0) {
        return buf;
    }

    // We check for scratch condition in the public function, so this shouldn't
    // happen
    if (rate_add_new * rate_add_old < 0) {
        qDebug() << "ERROR: EBSL did not detect scratching correctly.";
    }

    // Special case -- no scaling needed!
    if (rate_add_old == 1.0 && rate_add_new == 1.0) {
        int samples_needed = iRateLerpLength;
        CSAMPLE* write_buf = buf;

        // Use up what's left of the internal buffer.
        int iNextSample = static_cast<int>(ceil(m_dNextSampleIndex)) * 2;
        if (iNextSample + 1 < buffer_int_size) {
            for (int i = iNextSample;
                 samples_needed > 2 && i < buffer_int_size; i += 2) {
                *write_buf = buffer_int[i]; write_buf++;
                *write_buf = buffer_int[i+1]; write_buf++;
                samples_needed -= 2;
            }
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
        int read_samples = iRateLerpLength - samples_needed;

        // Even though this code should not trigger for the special case in
        // getScaled for when the rate changes directions, the convention in the
        // rest of this method is that we increment samples_read rather than
        // assign it.
        *samples_read += read_samples;

        // Zero the remaining samples if we didn't fill them.
        SampleUtil::clear(write_buf, samples_needed);

        // update our class members so next time we need to scale it's ok. we do
        // blow away the fractional sample position here
        buffer_int_size = 0; // force buffer read
        m_dNextSampleIndex = 0;
        m_fPrevSample[0] = buf[read_samples-2];
        m_fPrevSample[1] = buf[read_samples-1];
        return buf;
    }

    // Simulate the loop to estimate how many samples we need
    double samples = 0;

    for (int j = 0; j < iRateLerpLength; j += 2) {
        samples += fabs((rate_add_diff * static_cast<float>(j)) /
                        static_cast<float>(iRateLerpLength) + rate_add_old);
    }

    // We're calculating mono samples, so divide remaining buffer by 2;
    long unscaled_samples_needed = floor(samples);

    // If the current position fraction plus the future position fraction
    // loops over 1.0, we need to round up
    if (m_dNextSampleIndex - floor(m_dNextSampleIndex) +
        samples - floor(samples) > 1.0) {
        unscaled_samples_needed++;
    }

    // Multiply by 2 because it is predicting mono rates, while we want a stereo
    // number of samples.
    unscaled_samples_needed *= 2;

    // 0 is never the right answer
    unscaled_samples_needed = math_max<long>(2, unscaled_samples_needed);

    bool last_read_failed = false;
    CSAMPLE prev_sample[2];
    CSAMPLE cur_sample[2];

    prev_sample[0] = 0;
    prev_sample[1] = 0;
    cur_sample[0] = 0;
    cur_sample[1] = 0;

    int i = 0;
    int screwups = 0;
    while (i < iRateLerpLength) {
        //shift indicies
        m_dCurSampleIndex = m_dNextSampleIndex;

        // Because our index is a float value, we're going to be interpolating
        // between two samples, a lower (prev) and upper (cur) sample.
        // If the lower sample is off the end of the buffer (values between
        // -.999 and 0), load it from the saved globals.

        // The first bounds check (< buffer_int_size) is probably not needed.

        if (static_cast<int>(floor(m_dCurSampleIndex)) * 2 + 1 < buffer_int_size
            && m_dCurSampleIndex >= 0.0) {
            prev_sample[0] = buffer_int[static_cast<int>(
                    floor(m_dCurSampleIndex)) * 2];
            prev_sample[1] = buffer_int[static_cast<int>(
                    floor(m_dCurSampleIndex)) * 2 + 1];
        } else {
            prev_sample[0] = m_fPrevSample[0];
            prev_sample[1] = m_fPrevSample[1];
        }

        // Smooth any changes in the playback rate over iRateLerpLength
        // samples. This prevents the change from being discontinuous and helps
        // improve sound quality.
        float rate_add = static_cast<float>(i) * (rate_add_diff) /
                         static_cast<float>(iRateLerpLength) + rate_add_old;

        // if we don't have enough samples, load some more
        while (static_cast<int>(ceil(m_dCurSampleIndex)) * 2 + 1 >=
               buffer_int_size) {
            int old_bufsize = buffer_int_size;
            if (unscaled_samples_needed == 0) {
                unscaled_samples_needed = 2;
                screwups++;
            }

            int samples_to_read = math_min<int>(kiLinearScaleReadAheadLength,
                                                unscaled_samples_needed);

            buffer_int_size = m_pReadAheadManager->getNextSamples(
                rate_add_new == 0 ? rate_add_old : rate_add_new,
                buffer_int, samples_to_read);
            *samples_read += buffer_int_size;

            if (buffer_int_size == 0 && last_read_failed) {
                break;
            }
            last_read_failed = buffer_int_size == 0;

            unscaled_samples_needed -= buffer_int_size;
            //shift the index by the size of the old buffer
            m_dCurSampleIndex -= old_bufsize / 2.;
        }

        // Now that the buffer is up to date, we can get the value of the sample
        // at the floor of our position.
        if (static_cast<int>(floor(m_dCurSampleIndex)) * 2 >= 0.0) {
            prev_sample[0] = buffer_int[static_cast<int>(
                    floor(m_dCurSampleIndex)) * 2];
            prev_sample[1] = buffer_int[static_cast<int>(
                    floor(m_dCurSampleIndex)) * 2 + 1];
        }

        //I guess?
        if (last_read_failed) {
            break;
        }

        cur_sample[0] = buffer_int[static_cast<int>(
                ceil(m_dCurSampleIndex)) * 2];
        cur_sample[1] = buffer_int[static_cast<int>(
                ceil(m_dCurSampleIndex)) * 2 + 1];

        // For the current index, what percentage is it
        // between the previous and the next?
        CSAMPLE frac = m_dCurSampleIndex - floor(m_dCurSampleIndex);

        //Perform linear interpolation
        buf[i] = static_cast<float>(prev_sample[0]) +
                 frac * (static_cast<float>(cur_sample[0]) -
                 static_cast<float>(prev_sample[0]));
        buf[i+1] = static_cast<float>(prev_sample[1]) +
                   frac * (static_cast<float>(cur_sample[1]) -
                   static_cast<float>(prev_sample[1]));
        m_fPrevSample[0] = prev_sample[0];
        m_fPrevSample[1] = prev_sample[1];

        //increment the index for the next loop
        m_dNextSampleIndex = m_dCurSampleIndex +
                (i < iRateLerpLength ? fabs(rate_add) : fabs(rate_add_new));
        i +=2 ;
    }

    SampleUtil::clear(&buf[i], buf_size - i);

    return buf;
}
