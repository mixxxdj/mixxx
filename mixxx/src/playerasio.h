/***************************************************************************
                          playerasio.h  -  description
                             -------------------
    begin                : Sat Dec 6 2003
    copyright            : (C) 2003 by Beranger Enselme-Trichard
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


#ifndef PLAYERASIO_H
#define PLAYERASIO_H

#include "player.h"
#include <asiosys.h>
#include <asio.h>
#ifdef UNICODE
  #undef UNICODE
  #define WAS_UNICODE
#endif
#include <asiodrivers.h>
#ifdef WAS_UNICODE
  #define UNICODE
  #undef WAS_UNICODE
#endif


#define MAX_ASIO_DRIVERS = 32;

enum {
	// number of input and outputs supported by the host application
	kMaxInputChannels = 0,
	kMaxOutputChannels = 4
};


class PlayerAsio : public Player  {
public:
    PlayerAsio(ConfigObject<ConfigValue> *config, ControlObject *pControl);
    ~PlayerAsio();
    bool initialize();
    bool open();
    void close();
    void setDefaults();
    QStringList getInterfaces();
    QStringList getSampleRates();
    static QString getSoundApi() { return "ASIO"; }
    QString getSoundApiName() { return getSoundApi(); };
    /** Satisfy virtual declaration in EngineObject */
    void process(const CSAMPLE *, const CSAMPLE *, const int) { };
	void processCallback(long bufferIndex);

private:
    bool createBuffers();
	void Output_Float32_Int16(int channelNumber, float* inputBuffer, short* outputBuffer, bool swap); 
	void Output_Float32_Int32(int channelNumber, float* inputBuffer, long* outputBuffer, bool swap); 
	void Output_Float32_Float32(int channelNumber, float* inputBuffer, float* outputBuffer, bool swap); 


	// data...

    int m_reenter;

	// ASIOInit()
	ASIODriverInfo driverInfo;
	// ASIOGetChannels()
	long m_inputChannels;
	long m_outputChannels;
	// ASIOGetBufferSize()
	long m_minBufferSize;
	long m_maxBufferSize;
	long m_preferredBufferSize;
	long m_bufferGranularity;
	// ASIOOutputReady()
	bool m_postOutput;
	// ASIOGetLatencies ()
	long m_inputLatency;
	long m_outputLatency;
	// ASIOCreateBuffers ()
	ASIOCallbacks m_asioCallbacks;
	long m_inputBuffers;	// becomes number of actual created input buffers
	long m_outputBuffers;   	// becomes number of actual created output buffers
	ASIOBufferInfo m_bufferInfos[kMaxInputChannels + kMaxOutputChannels]; // buffer info's
	// ASIOGetChannelInfo()
	ASIOChannelInfo m_channelInfos[kMaxInputChannels + kMaxOutputChannels]; // channel info's
	// The above two arrays share the same indexing, as the data in them are linked together
    bool m_started;
    bool m_buffersCreated;
};


// ASIO callbacks
static void bufferSwitch(long index, ASIOBool processNow);
static ASIOTime *bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow);
static void sampleRateChanged(ASIOSampleRate sRate);
static long asioMessages(long selector, long value, void* message, double* opt);

extern AsioDrivers* asioDrivers;



#endif