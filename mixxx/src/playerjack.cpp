/***************************************************************************
                          playerjack.cpp  -  description
                             -------------------
    begin                : Sat Nov 15 2003
    copyright            : (C) 2003 by Tue Haste Andersen
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

#include "playerjack.h"

PlayerJack::PlayerJack(ConfigObject<ConfigValue> *config, ControlObject *pControl) : Player(config,pControl)
{
    ports = 0;
    m_iBufferSize = 1024;
    m_bOpen = false;

    jack_set_error_function(jackError);
}

PlayerJack::~PlayerJack()
{
    close();

    if (client)
    {
        jack_port_unregister(client, output_master_left);
        jack_port_unregister(client, output_master_right);
        jack_port_unregister(client, output_head_left);
        jack_port_unregister(client, output_head_right);

        jack_client_close(client);
    }

    if (ports)
        free(ports);
}

bool PlayerJack::initialize()
{
    if ((client = jack_client_new("Mixxx")) == 0)
        return false;

    jack_set_process_callback(client, jackProcess, this);
    jack_set_sample_rate_callback(client, jackSrate, this);
    jack_on_shutdown(client, jackShutdown, this);

    output_master_left  = jack_port_register(client, "Master left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    output_master_right = jack_port_register(client, "Master right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    output_head_left    = jack_port_register(client, "Head left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    output_head_right   = jack_port_register(client, "Head right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    return true;
}

bool PlayerJack::open()
{
    Player::open();

    /* tell the JACK server that we are ready to roll */
    if (jack_activate(client))
        qFatal("Jack: Cannot activate client");

    // Connect to the ports
    QString name;

    name = m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceMasterLeft"));
    if (name != "None")
    {
        qDebug("connecting %s to master left",name.latin1());
        if (jack_connect(client, jack_port_name(output_master_left), name.latin1()))
            m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterLeft"),ConfigValue("None"));
    }

    name = m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceMasterRight"));
    if (name != "None")
    {
        qDebug("connecting %s to master right",name.latin1());
        if (jack_connect(client, jack_port_name(output_master_right), name.latin1()))
            m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterRight"),ConfigValue("None"));
    }

    name = m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceHeadLeft"));
    if (name != "None")
    {
        qDebug("connecting %s to head left",name.latin1());
        if (jack_connect(client, jack_port_name(output_head_left), name.latin1()))
            m_pConfig->set(ConfigKey("[Soundcard]","DeviceHeadLeft"),ConfigValue("None"));
    }

    name = m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceHeadRight"));
    if (name != "None")
    {
        qDebug("connecting %s to head right",name.latin1());
        if (jack_connect(client, jack_port_name(output_head_right), name.latin1()))
            m_pConfig->set(ConfigKey("[Soundcard]","DeviceHeadRight"),ConfigValue("None"));
    }

    // Update the config database with the used sample rate
    QStringList srates = getSampleRates();
    QStringList::iterator it = srates.begin();
    if (*it)
    {
        m_pConfig->set(ConfigKey("[Soundcard]","Samplerate"),ConfigValue((*it)));

        // Set currently used latency in config database
        int msec = 1000*m_iBufferSize/(*it).toInt();
        m_pConfig->set(ConfigKey("[Soundcard]","Latency"), ConfigValue(msec));
    }

    m_bOpen = true;
    
    // FIX ME: RETURN FALSE IF NO DEVICES WERE OPENED!!!

    return true;
}

void PlayerJack::close()
{
    // Deactivate jack and disconnect all ports
    if (m_bOpen && client)
        jack_deactivate(client);
    m_bOpen = false;
}

void PlayerJack::setDefaults()
{
    // Get list of interfaces
    QStringList interfaces = getInterfaces();

    // Set first interfaces to master left
    QStringList::iterator it = interfaces.begin();
    if (*it)
    {
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterLeft"),ConfigValue((*it)));
    }
    else
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterLeft"),ConfigValue("None"));

    // Set second interface to master right
    ++it;
    if (*it)
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterRight"),ConfigValue((*it)));
    else
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterRight"),ConfigValue("None"));

    // Set head left and right to none
    m_pConfig->set(ConfigKey("[Soundcard]","DeviceHeadLeft"),ConfigValue("None"));
    m_pConfig->set(ConfigKey("[Soundcard]","DeviceHeadRight"),ConfigValue("None"));

    // Set default sample rate
    QStringList srates = getSampleRates();
    it = srates.begin();
    if (*it)
    {
        m_pConfig->set(ConfigKey("[Soundcard]","Samplerate"),ConfigValue((*it)));

        // Set currently used latency in config database
        int msec = 1000*m_iBufferSize/(*it).toInt();
        m_pConfig->set(ConfigKey("[Soundcard]","Latency"), ConfigValue(msec));
    }
}

QStringList PlayerJack::getInterfaces()
{
    QStringList result;

    if ((ports = jack_get_ports (client, 0, 0, JackPortIsPhysical|JackPortIsInput)) == 0)
        qFatal("Jack: Cannot find any physical playback ports");

    int i=0;
    while (ports[i]!=0)
    {
        result.append(ports[i]);
        ++i;
    }

    return result;
}

QStringList PlayerJack::getSampleRates()
{
    QStringList result;
    result.append(QString("%1").arg((int)jack_get_sample_rate(client)));
    return result;
}

QString PlayerJack::getSoundApi()
{
    return QString("Jack");
}

int PlayerJack::callbackProcess(int iBufferSize)
{
    //qDebug("master %i, slave %i",chMaster, chHead);
    jack_default_audio_sample_t *out_ml = (jack_default_audio_sample_t *)jack_port_get_buffer(output_master_left, iBufferSize);
    jack_default_audio_sample_t *out_mr = (jack_default_audio_sample_t *)jack_port_get_buffer(output_master_right, iBufferSize);
    jack_default_audio_sample_t *out_hl = (jack_default_audio_sample_t *)jack_port_get_buffer(output_head_left, iBufferSize);
    jack_default_audio_sample_t *out_hr = (jack_default_audio_sample_t *)jack_port_get_buffer(output_head_right, iBufferSize);

    if (m_bOpen)
    {
        CSAMPLE *buffer = prepareBuffer(iBufferSize);

        for (int i=0; i<iBufferSize; ++i)
        {
            *out_ml++=buffer[(i*4)  ]/32768.;
            *out_mr++=buffer[(i*4)+1]/32768.;
            *out_hl++=buffer[(i*4)+2]/32768.;
            *out_hr++=buffer[(i*4)+3]/32768.;
        }
    }

    return 0;
}

void PlayerJack::callbackSetSrate(int srate)
{
    setPlaySrate(srate);
}

void PlayerJack::callbackSetBufferSize(int iBufferSize)
{
    m_iBufferSize = iBufferSize;
}

void PlayerJack::callbackShutdown()
{
    qWarning("Jack is killing our connection.");
    client = 0;

    //exit(-1);
    
/*
    if ((client = jack_client_new("Mixxx")) == 0)
    {
        qFatal("Jack server not running.");
    }

    output_master_left  = jack_port_register(client, "Master left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    output_master_right = jack_port_register(client, "Master right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    output_head_left    = jack_port_register(client, "Head left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    output_head_right   = jack_port_register(client, "Head right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
*/
}

void jackError(const char *desc)
{
    qWarning("Jack experienced an error: %s", desc);
}

int jackProcess(jack_nframes_t nframes, void *arg)
{
    ((PlayerJack *)arg)->callbackProcess(nframes);
    return 0;
}

int jackSrate(jack_nframes_t nframes, void *arg)
{
    // Update SRATE in EngineObject
    ((PlayerJack *)arg)->callbackSetSrate(nframes);
    return 0;
}

void jackShutdown(void *arg)
{
    ((PlayerJack *)arg)->callbackShutdown();
}

void jackBufferSize(jack_nframes_t nframes, void *arg)
{
    ((PlayerJack *)arg)->callbackSetBufferSize(nframes);
}



