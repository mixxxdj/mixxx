/***************************************************************************
                          playeralsa.cpp  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "playeralsa.h"
#include "controlobject.h"

#ifndef PLAYERTEST
#include "rtthread.h"
#endif

// Tutorials /home/peter/pad-alsa-audio.html /home/peter/alsa090_howto.html
// Docs      /usr/share/doc/alsa-lib-1.0.4/doxygen/html/index.html
// Example   /usr/share/doc/alsa-lib-1.0.4/doxygen/html/_2test_2pcm_8c-example.html
// QT        /usr/share/doc/qt-devel-3.1.1/html/index.html

/** Maximum frame size used with ALSA. Used to determine no of buffers
 *   * when setting latency */
const int kiMaxFrameSize = 512;


#ifndef PLAYERTEST
PlayerALSA::PlayerALSA(ConfigObject<ConfigValue> *config) : Player(config)
#else
PlayerALSA::PlayerALSA()
#endif
{
    int err;

    handle = 0;
    err = snd_pcm_hw_params_malloc(&hwparams);
    if (err < 0)
        qCritical("Couldn't allocate memory for hw params: %s\n", snd_strerror(err));
    
    err = snd_pcm_sw_params_malloc(&swparams);
    if (err < 0)
        qCritical("Couldn't allocate memory for sw params: %s\n", snd_strerror(err));
    
    err = snd_pcm_sw_params_malloc(&swparams);
    if (err < 0)
        qCritical("Couldn't allocate memory for sw params: %s\n", snd_strerror(err));

    isopen = false;
    masterleft = masterright = -1;
    headleft = headright = -1;
    twrite = false;
    m_iChannels = alsa_channels;
    qDebug("Alsa constructed");
}

PlayerALSA::~PlayerALSA()
{
    if (isopen) close();

    qDebug("Alsa destroying...");
    if (hwparams)
    {
	snd_pcm_hw_params_free(hwparams);
    }
    if (swparams)
    {
	snd_pcm_sw_params_free(swparams);
    }
    qDebug("Alsa destroyed");
}

bool PlayerALSA::initialize()
{
    qDebug("Alsa initialized");

    return true;
}

bool PlayerALSA::open()
{
#ifndef PLAYERTEST
    Player::open();
#endif
    QString name;
    QString devname, devtmp;
    int err;

#ifndef PLAYERTEST
    QRegExp rx("(\\S+) \\(ch (\\d+)\\)");

    name = m_pConfig->getValueString(ConfigKey("[Soundcard]", "DeviceMasterLeft"));
    if (name != "None")
    {
        if (rx.search(name) < 0)
            qWarning() << "can't find device name or channel number in" << name;

        devname = rx.cap(1);
    } 
    else 
    {
        setDefaults(); // initialise defaults
        return open();
    }
#else
    devname = QString("surround40:0");
#endif

    // If no device was selected return false
    if (!devname)
        return false;
        
    qDebug() << "Alsa opening pcm_open:" << devname;

    if ((err = snd_pcm_open(&handle, devname.ascii(),
		    SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
	// AD: Try for default device instead
	if ((err = snd_pcm_open(&handle, "default",
			SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
            qWarning("Playback open error: %s\n", snd_strerror(err));
            return false;
	} else {
	    qWarning("Using default ALSA device, you may want to set up a ~/.asoundrc to enable all the features of your sound card");
	}
    }

    qDebug("Alsa setting hw");
    if ((err = set_hwparams()) < 0)
    {
        qWarning("Setting of hwparams failed: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return false;
    }

    qDebug("Alsa setting sw");
    if ((err = set_swparams()) < 0)
    {
        qWarning("Setting of swparams failed: %s\n", snd_strerror(err));
        snd_pcm_close(handle);
        return false;
    }

    masterleft = masterright = -1;
    headleft = headright = -1;

#ifndef PLAYERTEST
    int temp;

    // master left
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]", "DeviceMasterLeft"));
    qDebug() << "Alsa opening..." << name;
    if (name != "None")
    {
        if (rx.search(name) < 0)
        {
            qWarning() << "can't find device name or channel number in" << name;
        }
//      devname = rx.cap(1);
        temp = rx.cap(2).toInt();
        if (temp > 0 && temp <= m_iChannels)
        {
            masterleft = temp - 1;
        }
    }
    qDebug("Alsa opening...ML %d", masterleft);

    // master right
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]", "DeviceMasterRight"));
    if (name != "None")
    {
        if (rx.search(name) < 0)
        {
            qWarning() << "can't find device name or channel number in" << name;
        }
        devtmp = rx.cap(1);
        if (devname != devtmp)
        {
            qWarning("device name mismatch, only one device supported");
        }
        temp = rx.cap(2).toInt();
        if (temp > 0 && temp <= m_iChannels)
        {
            masterright = temp - 1;
        }
    }
    qDebug("Alsa opening...MR %d", masterright);

    // head left
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]", "DeviceHeadLeft"));
    if (name != "None")
    {
        if (rx.search(name) < 0)
        {
            qWarning() << "can't find device name or channel number in" << name;
        }
        devtmp = rx.cap(1);
        if (devname != devtmp)
        {
            qWarning("device name mismatch, only one device supported");
        }
        temp = rx.cap(2).toInt();
        if (temp > 0 && temp <= m_iChannels)
        {
            headleft = temp - 1;
        }
    }
    qDebug("Alsa opening...HL %d", headleft);

    // head right
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]", "DeviceHeadRight"));
    if (name != "None")
    {
        if (rx.search(name) < 0)
        {
            qWarning() << "can't find device name or channel number in" << name;
        }
        devtmp = rx.cap(1);
        if (devname != devtmp)
        {
            qWarning("device name mismatch, only one device supported");
        }
        temp = rx.cap(2).toInt();
        if (temp > 0 && temp <= m_iChannels)
        {
            headright = temp - 1;
        }
    }
    
    qDebug("Alsa opening...HR %d", headright);
    // Check if any of the devices in the config database needs to be set to "None"
    if (masterleft < 0)
        m_pConfig->set(ConfigKey("[Soundcard]", "DeviceMasterLeft"), ConfigValue("None"));
    if (masterright < 0)
        m_pConfig->set(ConfigKey("[Soundcard]", "DeviceMasterRight"), ConfigValue("None"));
    if (headleft < 0)
        m_pConfig->set(ConfigKey("[Soundcard]", "DeviceHeadLeft"), ConfigValue("None"));
    if (headright < 0)
        m_pConfig->set(ConfigKey("[Soundcard]", "DeviceHeadRight"), ConfigValue("None"));

#else
    masterleft = 0;
    masterright = 1;
    headleft = 2;
    headright = 3;
#endif

    twrite = true;
    isopen = true;
    
    QThread::start();

    return true;
}

/*
 *   Underrun and suspend recovery
 */
int PlayerALSA::xrun_recovery(int err)
{
    if (err == -EPIPE) {	/* under-run */
        err = snd_pcm_prepare(handle);
        if (err < 0)
            qWarning("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
        return 0;
    } 
    else if (err == -ESTRPIPE) 
    {
        while ((err = snd_pcm_resume(handle)) == -EAGAIN)
            sleep(1);   /* wait until the suspend flag is released */
        if (err < 0) 
        {
            err = snd_pcm_prepare(handle);
            if (err < 0)
                qWarning("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
        }
        return 0;
    }
    return err;
}

/**
 * thread for writing samples out
 */
void PlayerALSA::run()
{
    CSAMPLE *sptr;
    int err, ctr;

#ifndef PLAYERTEST
    rtThread();
#endif

    if (isformatfloat) {
        float *optr, *output;

        output = new float[buffer_size * m_iChannels];
        qDebug("Alsa allocating output buffer (FLOAT) %p", output);

        while (twrite)
        {
            sptr = prepareBuffer((int) period_size);
            optr = output;
            for (int i = 0; i < (int) period_size; i++)
            {
                for (int j = 0; j < m_iChannels; j++) optr[j] = 0;
                if (masterleft >= 0)  optr[masterleft]  = *sptr / 32768.;
                sptr++;
                if (masterright >= 0) optr[masterright] = *sptr / 32768.;
                sptr++;
                if (headleft >= 0)  optr[headleft]  = *sptr / 32768.;
                sptr++;
                if (headright >= 0) optr[headright] = *sptr / 32768.;
                sptr++;
                optr += m_iChannels;
            }
            ctr = period_size;
            optr = output;
            while (ctr > 0) 
            {
                err = snd_pcm_writei(handle, optr, ctr);
                if (err == -EAGAIN) continue;
                if (err < 0) 
                {
                    if (xrun_recovery(err) < 0)
                        qCritical("Write error: %s\n", snd_strerror(err));
                    
                    break;
                }
                optr += err*m_iChannels;
                ctr -= err;
            }
        }
        if (output) 
            delete output;
    } 
    else 
    {
        short int *optr, *output;

        output = new short int[buffer_size * m_iChannels];
        qDebug("Alsa allocating output buffer (S16) %p", output);

        while (twrite)
        {
            sptr = prepareBuffer((int) period_size);
            optr = output;
            for (int i = 0; i < (int) period_size; i++)
            {
                for (int j = 0; j < m_iChannels; j++) optr[j] = 0;
                if (masterleft >= 0)  optr[masterleft]  = (short int) *sptr;
                sptr++;
                if (masterright >= 0) optr[masterright] = (short int) *sptr;
                sptr++;
                if (headleft >= 0)  optr[headleft]  = (short int) *sptr;
                sptr++;
                if (headright >= 0) optr[headright] = (short int) *sptr;
                sptr++;
                optr += m_iChannels;
            }
            ctr = period_size;
            optr = output;
            while (ctr > 0) 
            {
                err = snd_pcm_writei(handle, optr, ctr);
                if (err == -EAGAIN) continue;
                if (err < 0) 
                {
                    if (xrun_recovery(err) < 0) 
                        qCritical("Write error: %s\n", snd_strerror(err));
                    break;
                }
                optr += err*m_iChannels;
                ctr -= err;
            }
        }
        if (output) delete output;
    }
    qDebug("Alsa leaving thread");
}

#ifdef PLAYERTEST
/**
 * generate a middle A slowly shifting across the channels
 */
CSAMPLE *PlayerALSA::prepareBuffer(int count)
{
    const double freq = 440.;
    const unsigned int rate = 44100;
    static CSAMPLE *out = 0;

    static double phase = 0;
    double max_phase = 1.0 / freq;
    double step = 1.0 / (double) rate;
    double res;
    CSAMPLE *sptr;
    int chn;
    static int channel = 0;
    static int ccount = 0;

    if (out == 0)
    out = new CSAMPLE[80000];
    sptr = out;
    while (count-- > 0)
    {
        res = 32768. * sin((phase * 2 * M_PI) / max_phase - M_PI);
        for (chn = 0; chn < alsa_channels; chn++)
        {
            if (chn == channel)
            *sptr = res;
            else
            *sptr = 0;
            sptr++;
        }
        phase += step;
        if (phase >= max_phase)
        {
            phase -= max_phase;
            ccount++;
            if (ccount == (3 * 440))
            {
                ccount = 0;
                channel++;
                if (channel == alsa_channels) channel = 0;
            }
        }
    }
    return out;
}
#endif

void PlayerALSA::close()
{
    if (!isopen)
        return;
        
    qDebug("Alsa closing");
    twrite = false;
    qDebug("Alsa waiting for thread for close");
    QThread::wait();

    snd_pcm_drop(handle);
    snd_pcm_close(handle);

    isopen = false;
}

void PlayerALSA::setDefaults()
{
    qDebug("Alsa setdefs");
#ifndef PLAYERTEST
    // Get list of interfaces
    QStringList interfaces = getInterfaces();

    // Set first interfaces to master left
    QStringList::iterator it = interfaces.begin();
    if (*it)
        m_pConfig->set(ConfigKey("[Soundcard]", "DeviceMasterLeft"), ConfigValue((*it)));
    else
        m_pConfig->set(ConfigKey("[Soundcard]", "DeviceMasterLeft"), ConfigValue("None"));
    
    // Set second interface to master right
    ++it;
    if (*it)
        m_pConfig->set(ConfigKey("[Soundcard]", "DeviceMasterRight"), ConfigValue((*it)));
    else
        m_pConfig->set(ConfigKey("[Soundcard]", "DeviceMasterRight"), ConfigValue("None"));

    // Set head left and right to none
    m_pConfig->set(ConfigKey("[Soundcard]", "DeviceHeadLeft"), ConfigValue("None"));
    m_pConfig->set(ConfigKey("[Soundcard]", "DeviceHeadRight"), ConfigValue("None"));

    // Set default sample rate
    QStringList srates = getSampleRates();
    it = srates.begin();
    while (*it)
    {
        m_pConfig->set(ConfigKey("[Soundcard]", "Samplerate"), ConfigValue((*it)));
        if ((*it).toInt() >= 44100)
            break;
        ++it;
    }

    // Set currently used latency in config database
    m_pConfig->set(ConfigKey("[Soundcard]", "Latency"), ConfigValue(default_latency));
#endif
}

QStringList PlayerALSA::getInterfaces()
{
    qDebug("Alsa getinter");
    QStringList result;

    result.append("mixxx (ch 1)");
    result.append("mixxx (ch 2)");
    result.append("mixxx (ch 3)");
    result.append("mixxx (ch 4)");

    return result;
}

QStringList PlayerALSA::getSampleRates()
{
    qDebug("Alsa getsample");
    QStringList result;
    result.append("11025");
    result.append("22050");
    result.append("44100");
    result.append("48000");
    return result;
}

QString PlayerALSA::getSoundApi()
{
    qDebug("Alsa getapi");
    return QString("Alsa");
}

int PlayerALSA::set_hwparams()
{
    unsigned int rate, rrate;
    unsigned int buffer_time = 500000;	/* ring buffer length in us */
    unsigned int period_time = 100000;	/* period time in us */

    int err, dir;

    /* choose all parameters */
    err = snd_pcm_hw_params_any(handle, hwparams);
    if (err < 0)
    {
        qWarning("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
        return err;
    }
    /* set the interleaved read/write format */
    err = snd_pcm_hw_params_set_access(handle, hwparams, alsa_access);
    if (err < 0)
    {
        qWarning("Access type not available for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* set the sample format */
    err = snd_pcm_hw_params_set_format(handle, hwparams, SND_PCM_FORMAT_FLOAT);
    if (err < 0)
    {
        qWarning("Sample format (FLOAT) not available for playback: %s\n", snd_strerror(err));
        qWarning("Falling back to S16");
        err = snd_pcm_hw_params_set_format(handle, hwparams, SND_PCM_FORMAT_S16);
        if (err < 0) 
        {
            qWarning("Sample format (S16) not available for playback: %s\n", snd_strerror(err));
            return err;
        }
        isformatfloat = false;
    } else {
        isformatfloat = true;
    }

    /* find min and max number of channels */
    err = snd_pcm_hw_params_get_channels_min(hwparams, &rate);
    if (err < 0)
    {
        qWarning("Channels count (%i) not available for playbacks: %s\n", rate, snd_strerror(err));
    }
    qDebug("Channels min %d", rate);
    err = snd_pcm_hw_params_get_channels_max(hwparams, &rate);
    if (err < 0)
    {
        qWarning("Channels count (%i) not available for playbacks: %s\n", rate, snd_strerror(err));
    }
    qDebug("Channels max %d", rate);
    m_iChannels = math_min(rate, alsa_channels);
    qDebug("Set channels = %d", m_iChannels);

    /* set the count of channels */
    err = snd_pcm_hw_params_set_channels(handle, hwparams, m_iChannels);
    if (err < 0)
    {
        qWarning("Channels count (%i) not available for playbacks: %s\n", m_iChannels, snd_strerror(err));
        return err;
    }
    /* set the stream rate */
#ifndef PLAYERTEST
    rate = m_pConfig->getValueString(ConfigKey("[Soundcard]", "Samplerate")).toUInt();
#else
    rate = 44100;
#endif
    rrate = rate;
    err = snd_pcm_hw_params_set_rate_near(handle, hwparams, &rrate, 0);
    if (err < 0)
    {
        qWarning("Rate %iHz not available for playback: %s\n", rate, snd_strerror(err));
        return err;
    }
    if (rrate != rate)
    {
        qWarning("Rate doesn't match (requested %iHz, get %iHz)\n", rate, err);
        return -EINVAL;
    }

#ifndef PLAYERTEST
//     setPlaySrate(rrate);
//     m_pControlObjectSampleRate->queueFromThread((double)rrate);

    // Update SRATE and Latency ControlObjects
    m_pControlObjectSampleRate->queueFromThread((double)rrate);
//     m_pControlObjectLatency->queueFromThread((double)iLatencyMSec);
// 
#endif

    /* set the buffer time */
#ifndef PLAYERTEST
    buffer_time = m_pConfig->getValueString(ConfigKey("[Soundcard]", "Latency")).toUInt() * 1000;
#else
    buffer_time = 50000;
#endif
    /* figure out size and make it even */
    buffer_size = (int) rrate*(1e-6*buffer_time) + 1;
   
    snd_pcm_uframes_t max = 0;
    snd_pcm_uframes_t min = 0;
    
    // FIXME: Should check return values to be on the safe side
    snd_pcm_hw_params_get_buffer_size_max(hwparams, &max);
    snd_pcm_hw_params_get_buffer_size_min(hwparams, &min);
    qDebug("Allowed buffer size range: %li -> %li", min, max);
  
    // Make sure buffer size we're aiming for is really allowed
    if (buffer_size < min) { buffer_size = (int)min; }
    if (buffer_size > max) { buffer_size = (int)max; }
    
    buffer_size -= (buffer_size & 1);

    if (buffer_size < min) { buffer_size += 2; }

    /* test buffer size first */
    err = snd_pcm_hw_params_test_buffer_size(handle, hwparams, buffer_size);
    if (err < 0) {
        qWarning("Unable to test buffer size %d", buffer_size);
        return err;
    }
    err = snd_pcm_hw_params_set_buffer_size(handle, hwparams, buffer_size);
    if (err < 0)
    {
        qWarning("Unable to set buffer size for playback: %s\n", snd_strerror(err));
        return err;
    }
    qWarning("Buffer %dus [actual %dus] (size: %d)", (int) buffer_time, (int) (1e6*buffer_size/rrate), buffer_size);

    /*
     * work out maximum number of periods so that only kiMaxFrameSize is
     * written out at a time, then use this as a starting point to find
     * period that works
     */
    int period_no_start = 2;

    if (buffer_size/kiMaxFrameSize>2)
        period_no_start = buffer_size/kiMaxFrameSize;

    for (period_no = period_no_start; period_no > 1; period_no--)
    {
        unsigned int period_time = buffer_time / period_no;
        qWarning("Period %dus (no: %d)", (int) period_time, period_no);
        
        int dir = 0;
            err = snd_pcm_hw_params_test_periods(handle, hwparams, period_no, dir);
        if (err < 0)
        {
            qWarning("Unable to test periods %i (dir %d) for playback: %s\n", period_no, dir,
                    snd_strerror(err));
        } 
        else 
        {
            err = snd_pcm_hw_params_set_periods(handle, hwparams, period_no, dir);
            if (err < 0)
            {
                qWarning("Unable to set periods %i for playback: %s\n", period_no, snd_strerror(err));
                return err;
            }
            err = snd_pcm_hw_params_get_period_size(hwparams, &period_size, &dir);
            if (err < 0)
            {
                qWarning("Unable to get period size for playback: %s\n", snd_strerror(err));
                return err;
            }
            qWarning("Period %df, buffer %df", (int) period_size, (int) buffer_size);
            break;
        }
    }
    
    if (period_no == 1)
        return -1;

    /* write the parameters to device */
    err = snd_pcm_hw_params(handle, hwparams);
    if (err < 0)
    {
        qWarning("Unable to set hw params for playback: %s\n", snd_strerror(err));
        return err;
    }
    qWarning("Rate: %d, buf %df (%dus), per %df (%dus)", rrate,
            (int) buffer_size, (int) (buffer_size / (rrate * 1e-6)),
            (int) period_size, (int) (period_size / (rrate * 1e-6)));
    
    // Update SRATE and Latency ControlObjects
    m_pControlObjectSampleRate->queueFromThread((double)rrate);
    m_pControlObjectLatency->queueFromThread((double)buffer_time/1000.);

    return 0;
}

int PlayerALSA::set_swparams()
{
    int err;

    /* get the current swparams */
    err = snd_pcm_sw_params_current(handle, swparams);
    if (err < 0)
    {
        qWarning("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* start the transfer when the buffer is full */
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size/period_size)*period_size);
    if (err < 0)
    {
        qWarning("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* allow the transfer when at least period_size samples can be processed */
    err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size);
    if (err < 0)
    {
        qWarning("Unable to set avail min for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* align all transfers to 1 sample */
    err = snd_pcm_sw_params_set_xfer_align(handle, swparams, 1);
    if (err < 0)
    {
        qWarning("Unable to set transfer align for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* write the parameters to the playback device */
    err = snd_pcm_sw_params(handle, swparams);
    if (err < 0)
    {
        qWarning("Unable to set sw params for playback: %s\n", snd_strerror(err));
        return err;
    }
    return 0;
}
