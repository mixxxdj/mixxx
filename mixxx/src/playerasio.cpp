/***************************************************************************
                          playerasio.cpp  -  description
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

#include "playerasio.h"
#include "playerproxy.h"
#include "controlobject.h"
#include <assert.h>

#define MAX_DRIVER_NAME_LENGTH 32


PlayerAsio::PlayerAsio(ConfigObject<ConfigValue> *config, ControlObject *pControl) : Player(config,pControl) 
{
	memset(&driverInfo, 0, sizeof(driverInfo));
	m_inputChannels = 0;
	m_outputChannels = 0;
	m_minBufferSize = 0;
	m_maxBufferSize = 0;
	m_preferredBufferSize = 0;
	m_bufferGranularity = 0;
    m_postOutput = false;
	m_inputLatency = 0;
	m_outputLatency = 0;
	m_inputBuffers = 0;
	m_outputBuffers = 0;
	memset(&m_bufferInfos, 0, sizeof(m_bufferInfos)); 
	memset(&m_channelInfos, 0, sizeof(m_channelInfos)); 
    m_reenter = 0;
}

PlayerAsio::~PlayerAsio() {
    if(ASIOExit() == ASE_OK) {
        qDebug("ASIO exited");
	    asioDrivers->removeCurrentDriver();
    }
//    delete asioDrivers;
//    asioDrivers = 0;
}

bool PlayerAsio::initialize() {
    asioDrivers = new AsioDrivers();
	return true;
}

bool PlayerAsio::createBuffers() {
	int i;
	ASIOError result;

	ASIOBufferInfo *info = m_bufferInfos;

    // note: no input channels to take care of
	if (m_outputChannels > kMaxOutputChannels)
		m_outputBuffers = kMaxOutputChannels;
	else
		m_outputBuffers = m_outputChannels;

	// initialize the ASIO buffer structures
	for(i = 0; i < m_outputBuffers; i++, info++)
	{
		info->isInput = ASIOFalse;
		info->channelNum = i;
		info->buffers[0] = info->buffers[1] = 0;
	}

	m_asioCallbacks.asioMessage = asioMessages;
	m_asioCallbacks.bufferSwitch = bufferSwitch;
	m_asioCallbacks.bufferSwitchTimeInfo = bufferSwitchTimeInfo; 
	m_asioCallbacks.sampleRateDidChange = sampleRateChanged; 

	// create and activate buffers
	result = ASIOCreateBuffers(m_bufferInfos,
		m_outputBuffers,
		m_preferredBufferSize, &m_asioCallbacks);

	if (result == ASE_OK)
	{
		// now get all the buffer details, sample word length, name, word clock group and activation
		for (i = 0; i < m_outputBuffers; i++)
		{
			m_channelInfos[i].channel = m_bufferInfos[i].channelNum;
			m_channelInfos[i].isInput = m_bufferInfos[i].isInput;
			result = ASIOGetChannelInfo(&m_channelInfos[i]);
			if (result != ASE_OK)
				break;
		}

		if (result == ASE_OK)
		{
			// get the input and output latencies
			// Latencies often are only valid after ASIOCreateBuffers()
			// (input latency is the age of the first sample in the currently returned audio block)
			// (output latency is the time the first sample in the currently returned audio block requires to get to the output)
			result = ASIOGetLatencies(&m_inputLatency, &m_outputLatency);
		}
	}

    qDebug("ASIO Buffers Created");
	return result == ASE_OK;
}

bool PlayerAsio::open() {
	Player::open();
	// ASIO supports just one device driver open at a time
	// so we use the DeviceMasterLeft param as our driver name

	QString name; // name of the driver we want to open

	// get it from the config and convert it to a usable form for the function we want to call
	name = m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceMasterLeft"));     	
	char driverName[MAX_DRIVER_NAME_LENGTH];
	strcpy(driverName, name.latin1());

	// try and load the driver
    if(!asioDrivers->loadDriver(driverName))
		return false;

    if (ASIOInit(&driverInfo) != ASE_OK)
        return false;

	// collect the informational data of the driver
	// get the number of available channels
	if(ASIOGetChannels(&m_inputChannels, &m_outputChannels) != ASE_OK) 
		return false;

	if(ASIOGetBufferSize(&m_minBufferSize, &m_maxBufferSize, &m_preferredBufferSize, &m_bufferGranularity) != ASE_OK)
		return false;
    
    
	int sampleRate = m_pConfig->getValueString(ConfigKey("[Soundcard]", "Samplerate")).toInt();

    // Update SRATE in EngineObject
    setPlaySrate(sampleRate);
    m_pControlObjectSampleRate->queueFromThread((double)sampleRate);


	if(ASIOCanSampleRate(sampleRate) != ASE_OK)
		return false;

	if(ASIOSetSampleRate(sampleRate) != ASE_OK)
		return false;

	if(ASIOOutputReady() == ASE_OK)
		m_postOutput = true;
	else
		m_postOutput = false;

    if(createBuffers()) {
        m_started = true;
        if (ASIOStart() == ASE_OK) {
           m_iChannels = m_outputChannels;
           qDebug("ASIO Driver Started");
           return true;
        }
        else {
            m_started = false;
            return false;
        }
    }
	else
		return false;
}

void PlayerAsio::close() {
    if (!m_started || ASIOStop() == ASE_OK) {
        m_started = false;
        qDebug("ASIO stopped");
        if(!m_buffersCreated || ASIODisposeBuffers() == ASE_OK) {
            m_buffersCreated = false;
            qDebug("ASIO buffers disposed");
        }
    }
}

void PlayerAsio::setDefaults() {
}

QStringList PlayerAsio::getInterfaces() {
	QStringList result;
	LPASIODRVSTRUCT lpdrv = asioDrivers->lpdrvlist;
	while(lpdrv) {
		result.append(lpdrv->drvname);
		lpdrv = lpdrv->next;
	}
	return result;
}

QStringList PlayerAsio::getSampleRates() {
	QStringList result;
    result.append("44100");
	return result;
}

#define SwapLong(v) ((((v)>>24)&0xFF)|(((v)>>8)&0xFF00)|(((v)&0xFF00)<<8)|(((v)&0xFF)<<24)) ;   
#define SwapShort(v) ((((v)>>8)&0xFF)|(((v)&0xFF)<<8)) ;        

void PlayerAsio::Output_Float32_Float32(int channelNumber, float* inputBuffer, float* outputBuffer, bool swap) 
{
	long temp;
	for(int i=0; i < m_preferredBufferSize; i++)
    {
		temp = (long) inputBuffer[i*sizeof(float) + channelNumber];
		if (swap) temp = SwapLong(temp);
		outputBuffer[i] = (float)temp / 32768;
	}
}

const float MAX_INT16_FP = (float)0x7FFF;

void PlayerAsio::Output_Float32_Int16(int channelNumber, float* inputBuffer, short* outputBuffer, bool swap) 
{
	short temp;
	for(int i=0; i < m_preferredBufferSize; i++)
    {
		temp = (short)(inputBuffer[i*sizeof(float) + channelNumber]);
		if (swap) temp = SwapShort(temp);
		outputBuffer[i] = (short)temp;
	}
}

const float MAX_INT32_FP = 2147483520.0f;

void PlayerAsio::Output_Float32_Int32(int channelNumber, float* inputBuffer, long* outputBuffer, bool swap) 
{
	long temp;
	for(int i=0; i < m_preferredBufferSize; i++)
    {
		temp = (long) ((inputBuffer[i*sizeof(float) + channelNumber] / 32768) * MAX_INT32_FP);
		if (swap) temp = SwapLong(temp);
		outputBuffer[i] = temp;
	}
}



#ifdef MAC
const bool swap = true;
#else
const bool swap = false;
#endif

void PlayerAsio::processCallback(long bufferIndex) {
    float *inputBuffer = prepareBuffer(m_preferredBufferSize);
    ASIOBufferInfo *bufferInfo = m_bufferInfos;

	int i;

    m_reenter++;
    if (m_reenter > 1) {
        qDebug("ASIO : reentered callback ???!!!");
        return;
    }

    if (!m_started) {
        qDebug("ASIO : callback and not started ???!!!");
    }



	for (i = 0; i < m_outputBuffers; i++, bufferInfo++) {
		void* outputBuffer = bufferInfo->buffers[bufferIndex];
		ASIOSampleType bufferType = m_channelInfos[i].type;
        switch (bufferType) {
			case ASIOSTInt16LSB:
				Output_Float32_Int16(i, inputBuffer, (short*)outputBuffer, swap);
                break;
			case ASIOSTInt16MSB:
				Output_Float32_Int16(i, inputBuffer, (short*)outputBuffer, !swap);
				break;  
			case ASIOSTInt32LSB:
				Output_Float32_Int32(i, inputBuffer, (long *)outputBuffer, swap);
				break;
			case ASIOSTInt32MSB:
				Output_Float32_Int32(i, inputBuffer, (long *)outputBuffer, !swap);
				break;  
			case ASIOSTFloat32LSB:
				Output_Float32_Float32(i, inputBuffer, (float *)outputBuffer, swap);
				break;
			case ASIOSTFloat32MSB:
				Output_Float32_Float32(i, inputBuffer, (float *)outputBuffer, !swap);
				break;                                                  
			case ASIOSTInt24LSB:            // used for 20 bits as well
			case ASIOSTInt24MSB:            // used for 20 bits as well
			case ASIOSTFloat64LSB:          // IEEE 754 64 bit double float, as found on Intel x86 architecture
			case ASIOSTFloat64MSB:          // IEEE 754 64 bit double float, as found on Intel x86 architecture

			// these are used for 32 bit data buffer, with different alignment of the data inside
			// 32 bit PCI bus systems can more easily used with these

			case ASIOSTInt32LSB16:          // 32 bit data with 16 bit alignment
			case ASIOSTInt32LSB18:          // 32 bit data with 18 bit alignment
			case ASIOSTInt32LSB20:          // 32 bit data with 20 bit alignment
			case ASIOSTInt32LSB24:          // 32 bit data with 24 bit alignment
                                                                                                                                                                        
                                        
			case ASIOSTInt32MSB16:          // 32 bit data with 16 bit alignment
			case ASIOSTInt32MSB18:          // 32 bit data with 18 bit alignment
			case ASIOSTInt32MSB20:          // 32 bit data with 20 bit alignment
			case ASIOSTInt32MSB24:          // 32 bit data with 24 bit alignment
				// not implemented !!

				break;
		}       
	}
    m_reenter--;

}

static void bufferSwitch(long index, ASIOBool processNow) {
	// as this is a "back door" into the bufferSwitchTimeInfo a timeInfo needs to be created
	// though it will only set the timeInfo.samplePosition and timeInfo.systemTime fields and the according flags
	ASIOTime  timeInfo;
	memset (&timeInfo, 0, sizeof (timeInfo));

	// we don't synchronize with anything so we don't need to initialize timeInfo
	bufferSwitchTimeInfo (&timeInfo, index, processNow);
}

static ASIOTime *bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow) {
	// the actual processing callback !
	((PlayerAsio *)(PlayerProxy::getPlayer()))->processCallback(index);
	return 0;
}

static void sampleRateChanged(ASIOSampleRate sRate) {
	// do nothing: not supported yet...
}

static long asioMessages(long selector, long value, void* message, double* opt) {
	// currently the parameters "value", "message" and "opt" are not used.
	long ret = 0;
	switch(selector)
	{
		case kAsioSelectorSupported:
			
			if(//value == kAsioResetRequest
			value == kAsioEngineVersion
			//|| value == kAsioResyncRequest
			//|| value == kAsioLatenciesChanged
			// the following three were added for ASIO 2.0, you don't necessarily have to support them
			|| value == kAsioSupportsTimeInfo
			//|| value == kAsioSupportsTimeCode
			//|| value == kAsioSupportsInputMonitor
			)
				ret = 1L;
			break;
		case kAsioResetRequest:
			// not supported yet
			break;
		case kAsioResyncRequest:
			// not supported yet
			break;
		case kAsioLatenciesChanged:
			// not supported yet
			break;
		case kAsioEngineVersion:
			// return the supported ASIO version of the host application
			// If a host applications does not implement this selector, ASIO 1.0 is assumed
			// by the driver
			ret = 2L;
			break;
		case kAsioSupportsTimeInfo:
			// informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
			// is supported.
			// For compatibility with ASIO 1.0 drivers the host application should always support
			// the "old" bufferSwitch method, too.
			ret = 1;
			break;
		case kAsioSupportsTimeCode:
			// informs the driver wether application is interested in time code info.
			// If an application does not need to know about time code, the driver has less work
			// to do.
			ret = 0;
			break;
	}
	return ret;
}




