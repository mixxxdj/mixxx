/***************************************************************************
                          playeralsa.h  -  description
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

#ifndef PLAYERALSA_H
#define PLAYERALSA_H

/**
  *@author Tue and Ken Haste Andersen, Peter Chang
  */

#ifndef PLAYERTEST
#include "player.h"
#else
// some definitions to avoid using player.h in test mode

#include <math.h>

#include <qstring.h>
#include <qstringlist.h>

typedef float CSAMPLE; // defines the CSAMPLE type used for intermediate calculations
#endif

#include <qregexp.h>
#include <qthread.h>
#include <alsa/asoundlib.h>

/**
 * there are two independent defines S16_OUTPUT and DIRECT_OUTPUT
 *  1) S16_OUTPUT makes the class output short integers to ALSA
 *  2) DIRECT_OUTPUT uses mmapped output
 * 
 * NB by default, these are not defined and so the class uses floats via the
 * plug plugin. Defining S16_OUTPUT allows the bare hw to be used (via surround40).
 * XXX: Mmapped output doesn't seem to work with the plug plugin...
 */

#ifndef PLAYERTEST
class PlayerALSA : public Player, public QThread 
{
public:
    PlayerALSA(ConfigObject<ConfigValue> *config);
#else
class PlayerALSA : public QThread {
public:
    PlayerALSA();
#endif
    ~PlayerALSA();
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

    /** Main loop of player. Executed in a separate thread by QT */
    void run();

protected:
/** ALSA parameters */
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *hwparams;
    snd_pcm_sw_params_t *swparams;

    snd_pcm_uframes_t buffer_size;
    snd_pcm_uframes_t period_size;
    unsigned int period_no;
    bool isformatfloat;

    /** True if devices are open */
    bool isopen;
    int masterleft, masterright, headleft, headright;

#ifdef PLAYERTEST
    CSAMPLE *prepareBuffer(int nframes);
#endif
    int setPeriodSize(bool setMinimum);
    int set_hwparams();
    int set_swparams();
    int xrun_recovery(int err);

    bool twrite; // flag for thread

private:
    static const snd_pcm_access_t alsa_access = SND_PCM_ACCESS_RW_INTERLEAVED;
    static const int alsa_channels = 4;
    static const int default_latency = 200;
};
#endif
