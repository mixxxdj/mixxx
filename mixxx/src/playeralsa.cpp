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
#include <qvalidator.h>

// Tutorials /home/peter/pad-alsa-audio.html /home/peter/alsa090_howto.html
// Docs      /usr/share/doc/alsa-lib-1.0.4/doxygen/html/index.html
// Example   /usr/share/doc/alsa-lib-1.0.4/doxygen/html/_2test_2pcm_8c-example.html
// QT        /usr/share/doc/qt-devel-3.1.1/html/index.html

#ifndef PLAYERTEST
PlayerALSA::PlayerALSA(ConfigObject<ConfigValue> *config, ControlObject *pControl) : Player(config, pControl)
#else
PlayerALSA::PlayerALSA()
#endif
{
    int err;

    handle = 0;
    err = snd_pcm_hw_params_malloc(&hwparams);
    if (err < 0)
    {
	qFatal("Couldn't allocate memory for hw params: %s\n", snd_strerror(err));
    }
    err = snd_pcm_sw_params_malloc(&swparams);
    if (err < 0)
    {
	qFatal("Couldn't allocate memory for sw params: %s\n", snd_strerror(err));
    }
    err = snd_pcm_sw_params_malloc(&swparams);
    if (err < 0)
    {
	qFatal("Couldn't allocate memory for sw params: %s\n", snd_strerror(err));
    }

    ahandler = 0;

    isopen = false;
    masterleft = masterright = -1;
    headleft = headright = -1;
    output = 0;
    twrite = false;
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

    masterleft = masterright = -1;
    headleft = headright = -1;

    qDebug("Alsa opening...2");
//#ifndef PLAYERTEST
// XXX: use pre-determined output until fixed
#if 1
    int temp;
    QRegExp rx("\(\\S+\) (ch \(\\d+\))");
    // XXX: crashing somewhere near here!
    // master left
    qDebug("Alsa opening...2.5");
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]", "DeviceMasterLeft"));
    qDebug("Alsa opening...3 %s", name.latin1());
    if (name != "None")
    {
	if (rx.search(name) < 0)
	{
	    qWarning("can't find device name or channel number in (%s)",name.latin1());
	}
	devname = rx.cap(1);
	temp = rx.cap(2).toInt();
	if (temp > 0)
	{
	    masterleft = temp - 1;
	}
    }
    qDebug("Alsa opening...ML");

    // master right
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]", "DeviceMasterRight"));
    if (name != "None")
    {
	if (rx.search(name) < 0)
	{
	    qWarning("can't find device name or channel number in (%s)",name.latin1());
	}
	devtmp = rx.cap(1);
	if (devname != devtmp)
	{
	    qWarning("device name mismatch, only one device supported");
	}
	temp = rx.cap(2).toInt();
	if (temp > 0)
	{
	    masterright = temp - 1;
	}
    }
    qDebug("Alsa opening...MR");

    // head left
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]", "DeviceHeadLeft"));
    if (name != "None")
    {
	if (rx.search(name) < 0)
	{
	    qWarning("can't find device name or channel number in (%s)",name.latin1());
	}
	devtmp = rx.cap(1);
	if (devname != devtmp)
	{
	    qWarning("device name mismatch, only one device supported");
	}
	temp = rx.cap(2).toInt();
	if (temp > 0)
	{
	    headleft = temp - 1;
	}
    }
    qDebug("Alsa opening...HL");

    // head right
    name = m_pConfig->getValueString(ConfigKey("[Soundcard]", "DeviceHeadRight"));
    if (name != "None")
    {
	if (rx.search(name) < 0)
	{
	    qWarning("can't find device name or channel number in (%s)",name.latin1());
	}
	devtmp = rx.cap(1);
	if (devname != devtmp)
	{
	    qWarning("device name mismatch, only one device supported");
	}
	temp = rx.cap(2).toInt();
	if (temp > 0)
	{
	    headright = temp - 1;
	}
    }
    qDebug("Alsa opening...MR");
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
#ifdef S16_OUTPUT
    devname = QString("surround40:0");
#else
    devname = QString("plug:surround40:0");
#endif
#endif

    qDebug("Alsa opening pcm_open: %s", devname.ascii());

    if ((err = snd_pcm_open(&handle, devname.ascii(), SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
	qWarning("Playback open error: %s\n", snd_strerror(err));
	return false;
    }

    qDebug("Alsa settting hw");
    if ((err = set_hwparams()) < 0)
    {
	qWarning("Setting of hwparams failed: %s\n", snd_strerror(err));
	return false;
    }

    qDebug("Alsa settting sw");
    if ((err = set_swparams()) < 0)
    {
	qWarning("Setting of swparams failed: %s\n", snd_strerror(err));
	return false;
    }

    qDebug("Alsa allocating output buffer %p", output);
    if (output)
	delete output;
    output = new OSAMPLE[buffer_size * alsa_channels];

    twrite = true;
    
    m_iBufferSize = buffer_size;

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
    } else if (err == -ESTRPIPE) {
	while ((err = snd_pcm_resume(handle)) == -EAGAIN)
	    sleep(1);	/* wait until the suspend flag is released */
        if (err < 0) {
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
    OSAMPLE *optr;
    int err, ctr;
    
    
    while (twrite)
    {
	sptr = prepareBuffer((int) period_size);
	optr = output;
	for (int i = 0; i < (int) period_size; i++)
	{
#ifndef S16_OUTPUT
	    optr[0] = optr[1] = optr[2] = optr[3] = 0.;
	    if (masterleft >= 0)  optr[masterleft]  = *sptr / 32768.;
	    sptr++;
	    if (masterright >= 0) optr[masterright] = *sptr / 32768.;
	    sptr++;
	    if (headleft >= 0)  optr[headleft]  = *sptr / 32768.;
	    sptr++;
	    if (headright >= 0) optr[headright] = *sptr / 32768.;
	    sptr++;
#else
	    optr[0] = optr[1] = optr[2] = optr[3] = 0;
	    if (masterleft >= 0)  optr[masterleft]  = (OSAMPLE) *sptr;
	    sptr++;
	    if (masterright >= 0) optr[masterright] = (OSAMPLE) *sptr;
	    sptr++;
	    if (headleft >= 0)  optr[headleft]  = (OSAMPLE) *sptr;
	    sptr++;
	    if (headright >= 0) optr[headright] = (OSAMPLE) *sptr;
	    sptr++;
#endif
	    optr += 4;
	}
        ctr = period_size;
        optr = output;
        while (ctr > 0) {
	    err = snd_pcm_writei(handle, optr, ctr);
	    if (err == -EAGAIN) continue;
	    if (err < 0) {
	        if (xrun_recovery(err) < 0) {
		    qFatal("Write error: %s\n", snd_strerror(err));
		}
	       break;
	    }
	    optr += err*4;
	    ctr -= err;
	}
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
    qDebug("Alsa closing");
    twrite = false;
    qDebug("Alsa waiting for thread for close");
    QThread::wait();
    // XXX: what should the shutdown routine be?
    // Stop audio
    //	snd_pcm_drain(handle);
    snd_pcm_drop(handle);
    snd_pcm_close(handle);
    if (output)
    {
	delete output;
        output = 0;
    }
    // XXX: this segfaults!
    // snd_async_del_handler(ahandler);
    //	ahandler = 0;
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
    if (*it)
    {
	m_pConfig->set(ConfigKey("[Soundcard]", "Samplerate"), ConfigValue((*it)));

	// Set currently used latency in config database
	m_pConfig->set(ConfigKey("[Soundcard]", "Latency"), ConfigValue(default_latency));
    }
#endif
}

QStringList PlayerALSA::getInterfaces()
{
    qDebug("Alsa getinter");
    QStringList result;
//   result.append("plug:front:0 (ch 1)");
//   result.append("plug:front:0 (ch 2)");
    result.append("plug:surround40:0 (ch 1)");
    result.append("plug:surround40:0 (ch 2)");
    result.append("plug:surround40:0 (ch 3)");
    result.append("plug:surround40:0 (ch 4)");
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
    err = snd_pcm_hw_params_set_format(handle, hwparams, alsa_format);
    if (err < 0)
    {
	qWarning("Sample format not available for playback: %s\n", snd_strerror(err));
	return err;
    }
    /* set the count of channels */
    err = snd_pcm_hw_params_set_channels(handle, hwparams, alsa_channels);
    if (err < 0)
    {
	qWarning("Channels count (%i) not available for playbacks: %s\n", alsa_channels, snd_strerror(err));
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
    setPlaySrate(rrate);
#endif

    /* set the buffer time */
#ifndef PLAYERTEST
    buffer_time = m_pConfig->getValueString(ConfigKey("[Soundcard]", "Latency")).toUInt() * 1000;
#else
    buffer_time = 50000;
#endif
    err = snd_pcm_hw_params_set_buffer_time_near(handle, hwparams, &buffer_time, &dir);
    if (err < 0)
    {
	qWarning("Unable to set buffer time %i for playback: %s\n", buffer_time, snd_strerror(err));
	return err;
    }
    err = snd_pcm_hw_params_get_buffer_size(hwparams, &buffer_size);
    if (err < 0)
    {
	qWarning("Unable to get buffer size for playback: %s\n", snd_strerror(err));
	return err;
    }

    // XXX: maybe using set_periods (and its ilk) is a better option
    /* set the period size */
    // the config space seems to reduce in extent as as we try out possible
    // choices so work backwards from a maximum number of periods per buffer of 4
    for (period_no = 4; period_no > 1; period_no--)
    {
	period_time = buffer_time / period_no;
//      period_time = (unsigned int) (buffer_size/(rrate*4e-6));
//      period_time = buffer_time/4;
	qWarning("Period %dus (no: %d)", (int) period_time, period_no);

	err = snd_pcm_hw_params_set_period_time_near(handle, hwparams, &period_time, &dir);
	if (err < 0)
	{
	    qWarning("Unable to set period time %i for playback: %s\n", period_time, snd_strerror(err));
	    return err;
	}
//      qWarning("-> %dus\n", (int) period_time);
	err = snd_pcm_hw_params_get_period_size(hwparams, &period_size, &dir);
	if (err < 0)
	{
	    qWarning("Unable to get period size for playback: %s\n", snd_strerror(err));
	    return err;
	}
	qWarning("Period %df, buffer %df", (int) period_size, (int) buffer_size);
	if (period_size < buffer_size)
	    break;
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

    m_iBufferSize = buffer_size*period_size; // send back latency value
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
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, buffer_size);
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
