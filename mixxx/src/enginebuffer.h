/***************************************************************************
                          enginebuffer.h  -  description
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

#ifndef ENGINEBUFFER_H
#define ENGINEBUFFER_H

#include <qapplication.h>

#include "defs.h"
#include "engineobject.h"
#include "monitor.h"

class MixxxApp;
class DlgPlaycontrol;
class ControlEngine;
class ControlObject;
class Reader;

/**
  *@author Tue and Ken Haste Andersen
*/

class EngineBuffer : public EngineObject
{
public:
    EngineBuffer(MixxxApp *_mixxx, DlgPlaycontrol *_playcontrol, const char *_group);
    ~EngineBuffer();
    /** Returns pointer to Reader object. Used in MixxxApp. */
    Reader *getReader();
    /** Reset buffer playpos and set file playpos. This must only be called while holding the
      * pause mutex */
    void setNewPlaypos(double);
    CSAMPLE *process(const CSAMPLE *, const int);

    CSAMPLE *update_visual();
    const char *getGroup();
    /** Get the relative playpos in a buffer sampled at Srate hz*/
    int getPlaypos(int Srate);
    /** Notify used to call seek when playpos slider changes */
    Monitor visualPlaypos;
    float visualRate;
private:
    void seek(double);

    MixxxApp *mixxx;
    QApplication *app;

    /** Pointer to reader */
    Reader *reader;    
    /** Buffer used in the process() */
    CSAMPLE *buffer;
    /** The current sample to play in the file. */
    double filepos_play;
    /** Sample in the buffer relative to filepos_play. */
    double bufferpos_play;
    /** Holds the rate as calculated in process. This is used in the
      * reader thread when calling soundbuffer->getchunk() */
    Monitor rate_exchange;
    /** Copy of rate_exchange, used to check if rate needs to be updated */
    double rate_old;
    /** Copy of length of file */
    int file_length_old;
    /** Copy of file sample rate*/
    int file_srate_old;

    /** Fileposition used in communication with file reader and main thread */
    //Monitor filepos_play_exchange;
    /** Mutex controlling weather the process function is in pause mode. This happens
      * during seek and loading of a new track */
    QMutex pause;
    /** Pointer to DlgPlaycontrol dialog. Used when updating file info window */
    DlgPlaycontrol *playcontrol;
   
    ControlEngine *playButton, *rateSlider, *wheel, *playposSlider;
    const char *group;

    CSAMPLE *read_buffer_prt;

    /** Rate factor between player and sound source sample rates */
    //FLOAT_TYPE BASERATE;
    /** Counter; when to update playpos slider */
    int playposUpdateCounter;

};
#endif
