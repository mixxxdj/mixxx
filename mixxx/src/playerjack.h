/***************************************************************************
                          playerjack.h  -  description
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

#ifndef PLAYERJACK_H
#define PLAYERJACK_H

#include "player.h"
#include <jack/jack.h>
#include <qlibrary.h>

/**
  *@author Tue and Ken Haste Andersen
  */

class PlayerJack : public Player  {
public:
    PlayerJack(ConfigObject<ConfigValue> *config, ControlObject *pControl);
    ~PlayerJack();
    bool initialize();
    bool open();
    void close();
    void setDefaults();
    QStringList getInterfaces();
    QStringList getSampleRates();
    static QString getSoundApi();
    QString getSoundApiName() { return getSoundApi(); };
    /** Satisfy virtual declaration in EngineObject */
    void process(const CSAMPLE *, const CSAMPLE *, const int) {};
    /** Process samples. Called from jack callback */
    int callbackProcess(int iBufferSize);
    /** Used to set sample rate from Jack callback */
    void callbackSetSrate(int iSrate);
    /** Used to set current buffer size */
    void callbackSetBufferSize(int iBufferSize);
    /** Used to reinitialize when shut down by Jack server */
    void callbackShutdown();

protected:
    /** Pointer to QLibrary object */
    QLibrary mLibJack;
    /** Pointer to client info */
    jack_client_t *client;
    /** Null terminated array of port names */
    const char **ports;
    /** Playback ports */
    jack_port_t *output_master_left, *output_master_right, *output_head_left, *output_head_right;
    /** True if devices are open */
    bool m_bOpen;
    /** Current buffer size in use */
    int m_iBufferSize;

/*
    typedef void (*jack_set_error_function_t)(void (*func)(const char *));
    jack_set_error_function_t jack_set_error_function;

    typedef int (*jack_port_unregister_t)(jack_client_t *, jack_port_t *);
    jack_port_unregister_t jack_port_unregister;

    typedef int (*jack_client_close_t)(jack_client_t *client);
    jack_client_close_t jack_client_close;

    typedef jack_client_t* (*jack_client_new_t)(const char *client_name);
    jack_client_new_t jack_client_new;

    typedef int (*jack_set_process_callback_t)(jack_client_t *client,JackProcessCallback process_callback,void *arg);
    jack_set_process_callback_t jack_set_process_callback;

    typedef int (*jack_set_sample_rate_callback_t)(jack_client_t *client,JackSampleRateCallback srate_callback,void *arg);
    jack_set_sample_rate_callback_t jack_set_sample_rate_callback;

    typedef void (*jack_on_shutdown_t)(jack_client_t *client, void (*function)(void *arg), void *arg);
    jack_on_shutdown_t jack_on_shutdown;

    typedef jack_port_t* (*jack_port_register_t)(jack_client_t *,const char *port_name,const char *port_type,unsigned long flags,unsigned long buffer_size);
    jack_port_register_t jack_port_register;

    typedef int (*jack_activate_t)(jack_client_t *client);
    jack_activate_t jack_activate;

    typedef int (*jack_connect_t)(jack_client_t *,const char *source_port,const char *destination_port);
    jack_connect_t jack_connect;

    typedef const char* (*jack_port_name_t)(const jack_port_t *port);
    jack_port_name_t jack_port_name;

    typedef int (*jack_deactivate_t)(jack_client_t *client);
    jack_deactivate_t jack_deactivate;

    typedef const char** (*jack_get_ports_t)(jack_client_t *,const char *port_name_pattern,const char *type_name_pattern,unsigned long flags);
    jack_get_ports_t jack_get_ports;

    typedef jack_nframes_t (*jack_get_sample_rate_t)(jack_client_t *);
    jack_get_sample_rate_t jack_get_sample_rate;

    typedef void* (*jack_port_get_buffer_t)(jack_port_t *, jack_nframes_t);
    jack_port_get_buffer_t jack_port_get_buffer;
*/
};

// Jack callbacks:
void jackError(const char *desc);
int jackProcess(jack_nframes_t nframes, void *arg);
int jackSrate(jack_nframes_t nframes, void *arg);
void jackShutdown(void *arg);
void jackBufferSize(jack_nframes_t nframes, void *arg);

#endif
