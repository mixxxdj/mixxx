/**
 * writeaudiofile.h - Wrapper super class for various file format libraries
 * Copyright (C) 2007 - John Sully
 *
 * Licence: GPLv2 or later
 *
 */

#ifndef WRITEAUDIOFILE_H
#define WRITEAUDIOFILE_H

#include "controlobjectthread.h"
#include "configobject.h"
#include "controlobject.h"
#include "defs.h"
#include <sndfile.h>
#include "../engine/engineabstractrecord.h"
#include "encoder.h"


class WriteAudioFile : public Encoder
{
public:
    WriteAudioFile(ConfigObject<ConfigValue> *_config, EngineAbstractRecord *engine=0);
    ~WriteAudioFile();
    void open();
    void write(const CSAMPLE *pIn, const int iBufferSize);
    void close();

	/* We don't use WAVE and AIFF for streaming
	 * Moreover, WAVE and AIFF are not compressed
     * Hence, we leave the following method bodies empty
	 */
	int initEncoder(int bitrate){};
    void encodeBuffer(const CSAMPLE *samples, const int size) {};
	void updateMetaData(char* artist, char* title){};
	//overloaded method for MP3 and OGG recording
	void write(unsigned char *header, unsigned char *body,int headerLen, int bodyLen) {};
	
	//Call this method in conjunction with shoutcast streaming
	void sendPackages(){};
private:
    SNDFILE *sf;
    SF_INFO sfInfo;
    ConfigObject<ConfigValue> *config;
    ControlObjectThread *ctrlRec;
    bool ready; //if we can record this is set
};

//define Sub Classes
#endif
