/**
 * writeaudiofile.h - Wrapper super class for various file format libraries
 * Copyright (C) 2007 - John Sully
 *
 * Licence: GPLv2 or later
 *
 */

#ifndef WRITEAUDIOFILE_H
#define WRITEAUDIOFILE_H

#include "controlobjectthreadmain.h"
#include "configobject.h"
#include "controlobject.h"
#include "defs.h"
#include <sndfile.h>

class WriteAudioFile
{
public:
    WriteAudioFile(ConfigObject<ConfigValue> *_config);
    virtual ~WriteAudioFile();
    virtual void open();
    virtual void write(const CSAMPLE *pIn, int iBufferSize);
    virtual void close();
private:
    SNDFILE *sf;
    SF_INFO sfInfo;
    ConfigObject<ConfigValue> *config;
    ControlObject *ctrlRec;
    bool ready; //if we can record this is set
};

//define Sub Classes
#endif
