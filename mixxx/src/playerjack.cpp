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
#include <qmessagebox.h>
#include <qapplication.h>

PlayerJack::PlayerJack(ConfigObject<ConfigValue> *config, ControlObject *pControl) : Player(config,pControl) //, mLibJack("libjack.so")
{
    ports = 0;
    m_iBufferSize = 1024;
    m_bOpen = false;

/*
    //mLibJack = new QLibrary("jack");
    if (!mLibJack.load())
        qDebug("lib path %s",mLibJack.library().latin1());

    //if (mLibJack.isLoaded())
    {
        jack_set_error_function = (jack_set_error_function_t) mLibJack.resolve("jack_set_error_function");
        jack_port_unregister = (jack_port_unregister_t) mLibJack.resolve("jack_port_unregister");
        jack_client_close = (jack_client_close_t) mLibJack.resolve("jack_client_close");
        jack_client_new = (jack_client_new_t) mLibJack.resolve("jack_client_new");
        jack_set_process_callback = (jack_set_process_callback_t) mLibJack.resolve("jack_set_process_callback");
        jack_set_sample_rate_callback = (jack_set_sample_rate_callback_t) mLibJack.resolve("jack_set_sample_rate_callback");
        jack_on_shutdown =(jack_on_shutdown_t) mLibJack.resolve("jack_on_shutdown");
        jack_port_register =(jack_port_register_t) mLibJack.resolve("jack_port_register");
        jack_activate =(jack_activate_t) mLibJack.resolve("jack_activate");
        jack_connect =(jack_connect_t) mLibJack.resolve("jack_connect");
        jack_port_name =(jack_port_name_t) mLibJack.resolve("jack_port_name");
        jack_deactivate =(jack_deactivate_t) mLibJack.resolve("jack_deactivate");
        jack_get_ports =(jack_get_ports_t) mLibJack.resolve("jack_get_ports");
        jack_get_sample_rate =(jack_get_sample_rate_t) mLibJack.resolve("jack_get_sample_rate");
        jack_port_get_buffer =(jack_port_get_buffer_t) mLibJack.resolve("jack_port_get_buffer");
    }
*/
}

PlayerJack::~PlayerJack()
{
//    if (!mLibJack.isLoaded())
//        return;

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
    // Verify that library is loaded, and all function pointers has been retreived
    /*
    if (!mLibJack.isLoaded())
    {
        qDebug("lib not loaded");
        return false;
    }
    else
    if (!jack_set_error_function |
            !jack_port_unregister |
            !jack_client_close |
            !jack_client_new |
            !jack_set_process_callback |
            !jack_set_sample_rate_callback |
            !jack_on_shutdown |
            !jack_port_register |
            !jack_activate |
            !jack_connect |
            !jack_port_name |
            !jack_deactivate |
            !jack_get_ports |
            !jack_get_sample_rate |
            !jack_port_get_buffer)
    {
        qDebug("API function pointer error");
        return false;
    }
    */

    jack_set_error_function(jackError);

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
    {
        qDebug("Jack: Cannot activate client");
        return false;
    }

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
    if (*it) ++it;
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

    if (!client && !initialize())
    {
        qDebug("Jack: Failed to reinitialize the connection to Jack");
        return result;
    }

    if ((ports = jack_get_ports (client, 0, 0, JackPortIsPhysical|JackPortIsInput)) == 0)
        qDebug("Jack: Cannot find any physical playback ports");
    else
    {
        int i=0;
        while (ports[i]!=0)
        {
            result.append(ports[i]);
            ++i;
        }
    }

    return result;
}

QStringList PlayerJack::getSampleRates()
{
    QStringList result;

    if (client)
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
    m_pConfig->set(ConfigKey("[Soundcard]","SoundApi"), ConfigValue("None"));

    qWarning("Jack connection was killed.\n\nThis *could* be due to a high CPU load. Try reducing\nthe sound quality and/or disable the waveform displays.");
}

void jackError(const char *desc)
{
    qDebug("Jack experienced an error: %s", desc);
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



