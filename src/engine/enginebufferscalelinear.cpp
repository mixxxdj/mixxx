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

#include <QtCore>
#include "engine/enginebufferscalelinear.h"
#include "mathstuff.h"
#include "sampleutil.h"

EngineBufferScaleLinear::EngineBufferScaleLinear(ReadAheadManager *pReadAheadManager) :
    EngineBufferScale(),
    m_pReadAheadManager(pReadAheadManager)
{
    m_dBaseRate = 0.0f;
    m_dTempo = 0.0f;
    m_fOldTempo = 1.0f;
    m_fOldBaseRate = 1.0f;
    m_dCurSampleIndex = 0.0f;
    m_dNextSampleIndex = 0.0f;

    for (int i=0; i<2; i++)
        m_fPrevSample[i] = 0.0f;

    buffer_int = new CSAMPLE[kiLinearScaleReadAheadLength];
    buffer_int_size = 0;

    /*df.setFileName("mixxx-debug-scaler.csv");
    df.open(QIODevice::WriteOnly | QIODevice::Text);
    writer.setDevice(&df);
    buffer_count=0;*/
    SampleUtil::applyGain(buffer_int, 0.0, kiLinearScaleReadAheadLength);
}

EngineBufferScaleLinear::~EngineBufferScaleLinear()
{
    //df.close();
    delete [] buffer_int;
}

double EngineBufferScaleLinear::setTempo(double _tempo)
{
//    if (m_fOldTempo != m_dTempo)
        m_fOldTempo = m_dTempo; //Save the old tempo when the tempo changes

    m_dTempo = _tempo;

    if (m_dTempo > MAX_SEEK_SPEED) {
        m_dTempo = MAX_SEEK_SPEED;
    } else if (m_dTempo < -MAX_SEEK_SPEED) {
        m_dTempo = -MAX_SEEK_SPEED;
    }

    // Determine playback direction
    if (m_dTempo < 0.) {
        m_bBackwards = true;
    } else {
        m_bBackwards = false;
    }

    return m_dTempo;
}

void EngineBufferScaleLinear::setBaseRate(double dBaseRate)
{
//    if (m_fOldBaseRate != m_dBaseRate)
        m_fOldBaseRate = m_dBaseRate; //Save the old baserate when it changes

    m_dBaseRate = dBaseRate*m_dTempo;
}

void EngineBufferScaleLinear::clear()
{
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
CSAMPLE * EngineBufferScaleLinear::getScaled(unsigned long buf_size) {
    float rate_add_new = m_dBaseRate;
    float rate_add_old = m_fOldBaseRate; //Smoothly interpolate to new playback rate
    int samples_read = 0;
    m_samplesRead = 0;

    // Guard against buf_size == 0
    if ((int)buf_size == 0)
        return m_buffer;

    if (rate_add_new * rate_add_old < 0) {
        //calculate half buffer going one way, and half buffer going
        //the other way.

        //first half: rate goes from old rate to zero
        m_fOldBaseRate = rate_add_old;
        m_dBaseRate = 0.0;
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
        m_fOldBaseRate = 0.0;
        m_dBaseRate = rate_add_new;
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
CSAMPLE * EngineBufferScaleLinear::do_scale(CSAMPLE* buf,
        unsigned long buf_size, int* samples_read) {
    float rate_add_new = m_dBaseRate;
    float rate_add_old = m_fOldBaseRate; //Smoothly interpolate to new playback rate
    float rate_add = rate_add_new;
    float rate_add_diff = rate_add_new - rate_add_old;

    //Update the old base rate because we only need to
    //interpolate/ramp up the pitch changes once.
    m_fOldBaseRate = m_dBaseRate;

    // Determine position in read_buffer to start from. (This is always 0 with
    // the new EngineBuffer implementation)

    int iRateLerpLength = static_cast<int>(buf_size);

    // Guard against buf_size == 0
    if (iRateLerpLength == 0)
        return buf;

    // We check for scratch condition in the public function, so this shouldn't
    // happen
    if (rate_add_new * rate_add_old < 0) {
        qDebug() << "ERROR: EBSL did not detect scratching correctly.";
    }

    // Special case -- no scaling needed!
    if (rate_add_old == 1.0 && rate_add_new == 1.0) {
        int samples_needed = buf_size;
        CSAMPLE* write_buf = buf;

        // Use up what's left of the internal buffer.
        int iNextSample = static_cast<int>(ceil(m_dNextSampleIndex)) * 2;
        if (iNextSample + 1 < buffer_int_size) {
            for (int i = iNextSample; samples_needed > 2 && i < buffer_int_size; i+=2) {
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
            int read_size = m_pReadAheadManager->getNextSamples(1.0, write_buf, samples_needed);
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
        SampleUtil::applyGain(write_buf, 0.0f, samples_needed);

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
        rate_add = (rate_add_diff) / (float)iRateLerpLength * (float)j + rate_add_old;
        samples += fabs(rate_add);
    }

    rate_add = rate_add_new;
    double rate_add_abs = fabs(rate_add);

    //we're calculating mono samples, so divide remaining buffer by 2;
    samples += (rate_add_abs * ((float)(buf_size - iRateLerpLength)/2));
    long unscaled_samples_needed = floor(samples);

    //if the current position fraction plus the future position fraction
    //loops over 1.0, we need to round up
    if (m_dNextSampleIndex - floor(m_dNextSampleIndex) + samples - floor(samples) > 1.0) {
        unscaled_samples_needed++;
    }

    // Multiply by 2 because it is predicting mono rates, while we want a stereo
    // number of samples.
    unscaled_samples_needed *= 2;

    //0 is never the right answer
    unscaled_samples_needed = math_max(2,unscaled_samples_needed);

    bool last_read_failed = false;
    CSAMPLE prev_sample[2];
    CSAMPLE cur_sample[2];

    prev_sample[0] = 0;
    prev_sample[1] = 0;
    cur_sample[0] = 0;
    cur_sample[1] = 0;

    int i = 0;
    int screwups = 0;
    while (i < buf_size) {
        //shift indicies
        m_dCurSampleIndex = m_dNextSampleIndex;

        //we're going to be interpolating between two samples, a lower (prev)
        //and upper (cur) sample.  If the lower sample is off the end of the buffer,
        //load it from the saved globals

        if ((int)floor(m_dCurSampleIndex)*2+1 < buffer_int_size && m_dCurSampleIndex >= 0.0) {
            m_fPrevSample[0] = prev_sample[0] = buffer_int[(int)floor(m_dCurSampleIndex)*2];
            m_fPrevSample[1] = prev_sample[1] = buffer_int[(int)floor(m_dCurSampleIndex)*2+1];
        } else {
            prev_sample[0] = m_fPrevSample[0];
            prev_sample[1] = m_fPrevSample[1];
        }

        //Smooth any changes in the playback rate over iRateLerpLength
        //samples. This prevents the change from being discontinuous and helps
        //improve sound quality.
        if (i < iRateLerpLength) {
            rate_add = (float)i * (rate_add_diff) / (float)iRateLerpLength + rate_add_old;
            //rate_add = sigmoid_zero((float)i,(float)iRateLerpLength) * rate_add_diff + rate_add_old;
        } else {
            rate_add = rate_add_new;
        }

        // if we don't have enough samples, load some more
        while ((int)ceil(m_dCurSampleIndex)*2+1 >= buffer_int_size) {
            int old_bufsize = buffer_int_size;
            //qDebug() << "buffer" << buffer_count << rate_add_old << rate_add_new << rate_add << i << m_dCurSampleIndex << buffer_int_size << unscaled_samples_needed;
            //Q_ASSERT(unscaled_samples_needed > 0);
            if (unscaled_samples_needed == 0) {
                //qDebug() << "screwup" << m_dCurSampleIndex << (int)ceil(m_dCurSampleIndex)*2+1 << buffer_int_size;
                unscaled_samples_needed = 2;
                screwups++;
            }

            int samples_to_read = math_min(kiLinearScaleReadAheadLength,
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
            m_dCurSampleIndex -= old_bufsize / 2;
            //fractions below 0 is ok, the ceil will bring it up to 0
            //this happens sometimes, somehow?
            //Q_ASSERT(m_dCurSampleIndex > -1.0);

            //not sure this actually does anything, but it seems to help
            if ((int)floor(m_dCurSampleIndex)*2 >= 0.0) {
                m_fPrevSample[0] = prev_sample[0] = buffer_int[(int)floor(m_dCurSampleIndex)*2];
                m_fPrevSample[1] = prev_sample[1] = buffer_int[(int)floor(m_dCurSampleIndex)*2+1];
            }
        }
        //I guess?
        if (last_read_failed)
            break;

        cur_sample[0] = buffer_int[(int)ceil(m_dCurSampleIndex)*2];
        cur_sample[1] = buffer_int[(int)ceil(m_dCurSampleIndex)*2+1];

        //rate_add was here

        //for the current index, what percentage is it between the previous and the next?
        CSAMPLE frac = m_dCurSampleIndex - floor(m_dCurSampleIndex);

        //Perform linear interpolation
        buf[i] = (float)prev_sample[0] + frac * ((float)cur_sample[0] - (float)prev_sample[0]);
        buf[i+1] = (float)prev_sample[1] + frac * ((float)cur_sample[1] - (float)prev_sample[1]);

        //at extremely low speeds, dampen the gain to hide pops and clicks
        //this does cause odd-looking linear waveforms that go to zero and back

        /*writer << QString("%1,%2,%3,%4\n").arg(buffer_count)
                                         .arg(buffer[i])
                                           .arg(prev_sample[0])
                                           .arg(cur_sample[0]);
        buffer_count++;*/

        //increment the index for the next loop
        m_dNextSampleIndex = m_dCurSampleIndex +
                (i < iRateLerpLength ? fabs(rate_add) : rate_add_abs);
        i+=2;
    }

    // If we broke out of the loop, zero the remaining samples
    // TODO(XXX) memset
    //for (; i < buf_size; i += 2) {
    //    buf[i] = 0.0f;
    //    buf[i+1] = 0.0f;
    //}

    //Q_ASSERT(i>=buf_size);
    SampleUtil::applyGain(&buf[i], 0.0f, buf_size-i);

    // It's possible that we will exit this function without having satisfied
    // this requirement. We may be trying to read past the end of the file.
    //Q_ASSERT(unscaled_samples_needed == 0);

    return buf;
}
