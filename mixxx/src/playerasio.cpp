/***************************************************************************
                          PlayerAsio.cpp  -  description
                             -------------------
    begin                : Thu June 02 2005
    copyright            : (C) 2005 by Ingo Kossyk
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


// PlayerAsio.cpp: implementation of the PlayerAsio class.
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// ToDo:
// - Implement all conversion routines for the different drivers
// - Testing on different soundcards
// - Correctly (???) setting the latency in the preferences and locking it
// - Samplerate Changed needs to be implemented if changed in the Preferences
//////////////////////////////////////////////////////////////////////

#include "qstringlist.h"
#include "PlayerAsio.h"
#include "playerproxy.h"
#include "controlobject.h"
//////////////////////////////////////////////////////////////////////
// External Declarations
//////////////////////////////////////////////////////////////////////

__declspec(dllimport) AsioDrivers* asioDrivers;
ASIOCallbacks asioCallbacks;
DriverInfo asioDriverInfo;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const int MAX_DRIVER_NAME_LENGTH = 32;

PlayerAsio::PlayerAsio(ConfigObject<ConfigValue> * config) : Player(config)

{
    m_pTempBuffer = 0;
    m_reenter = 0;
    bufferLength = 0;
    driverIsLoaded = false;

    for (int i = 0 ; i < 32 ; ++i) {
        driverList[i] =static_cast<char *>(malloc(32));

        /* check memory */
        if(!driverList[i]) qDebug("ERROR: Could not allocate Memory!");
    }


    loadAsioDriver("dummy");

    drvNum = asioDrivers->getDriverNames(driverList,32);

}

PlayerAsio::~PlayerAsio()
{

    ASIOStop();
    ASIOExit();

}
//////////////////////////////////////////////////////////////////////
// Class functions
//////////////////////////////////////////////////////////////////////

/*******
 *
 * Conversion of different Ouputtypes
 *
 ********/
const double fScaler24 = (double)0xffff;

void PlayerAsio::float32toInt24inPlace(float * buffer, long frames, bool swap)
{
    double sc = fScaler24 + .5;
    long a;
    char * b = (char *)(buffer);
    char * aa = (char *)(&a);

    while(--frames >= 0)
    {
        a = static_cast<long>(static_cast<double>(*buffer++) *sc);


        if (swap)
        {
            *b++ = aa[3];
            *b++ = aa[2];
            *b++ = aa[1];
        }
        else
        {
            *b++ = aa[1];
            *b++ = aa[2];
            *b++ = aa[3];
        }
    }
}


void PlayerAsio::Output_Float32_Int24(int channelNumber, float * inputBuffer, float * outputBuffer, bool swap)
{
    for(int i=0; i < bufferLength; i++)
    {
        m_pTempBuffer[i] = inputBuffer[i*sizeof(float) + channelNumber];
    }

    float32toInt24inPlace(m_pTempBuffer,bufferLength,swap);
    memcpy(outputBuffer,m_pTempBuffer,bufferLength*3);

}

void PlayerAsio::Output_Float32_Int32(int channelNumber, float * inputBuffer, float * outputBuffer, bool swap)
{
    for(int i=0; i < bufferLength; i++)
    {
        m_pTempBuffer[i] = inputBuffer[i*sizeof(float) + channelNumber];
    }

    float32toInt32inPlace(static_cast<float *>(m_pTempBuffer),bufferLength,swap);
    memcpy(outputBuffer,m_pTempBuffer,bufferLength*4);

}

const double fScaler32 = (double)0xffff;

void PlayerAsio::float32toInt32inPlace(float * buffer, long frames,bool swap)
{
    double sc = fScaler32 + .49999;
    long a;
    char * b = (char *)(buffer);
    char * aa = (char *)(&a);

    while(--frames >= 0)
    {
        a = static_cast<long>(static_cast<double>(*buffer++) * sc);


        if(swap){
            *b++ = aa[3];
            *b++ = aa[2];
            *b++ = aa[1];
            *b++ = aa[0];
        }
        else
        {
            *b++ = aa[0];
            *b++ = aa[1];
            *b++ = aa[2];
            *b++ = aa[3];
        }
    }
}

void PlayerAsio::Output_Float32_Int16(int channelNumber, float * inputBuffer, short * outputBuffer, bool swap)
{
    short temp;
    for (int i=0; i < bufferLength; i++)
    {
        temp = static_cast<short>(inputBuffer[i*sizeof(float) + channelNumber]);
        outputBuffer[i] = static_cast<short>(temp);
        if (swap) SwapBuffer(&outputBuffer[i]);
    }
}

void PlayerAsio::SwapBuffer(short * buffer)
{
    short a;

    char * b = (char *)(buffer);
    char * aa = (char *)(&a);

    a = *buffer;

    *b++ = aa[1];
    *b++ = aa[0];

}
/*******
 *
 * Init
 *
 ********/

bool PlayerAsio::initialize() {
    return true;
}

bool PlayerAsio::initDriver()
{


    if (ASIOInit (&asioDriverInfo.driverInfo) == ASE_OK ){


        return true;
    }
    return false;


}


int PlayerAsio::getBufferSize(void)
{
    return asioDriverInfo.preferredSize;
}


int PlayerAsio::getChannelCount(void)
{
    return asioDriverInfo.outputChannels;
}


int PlayerAsio::getSampleRate(void)
{
    return asioDriverInfo.sampleRate;
}


QStringList PlayerAsio::getSampleRates() {
    QStringList result;
    result.append("44100");
    return result;
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

long PlayerAsio::init_asio_static_data (DriverInfo * asioDriverInfo)
{

    if(ASIOGetChannels(&asioDriverInfo->inputChannels, &asioDriverInfo->outputChannels) == ASE_OK)
    {


        // get the usable buffer sizes
        if(ASIOGetBufferSize(&asioDriverInfo->minSize, &asioDriverInfo->maxSize, &asioDriverInfo->preferredSize, &asioDriverInfo->granularity) == ASE_OK)
        {

            // get the currently selected sample rate
            if(ASIOGetSampleRate(&asioDriverInfo->sampleRate) == ASE_OK)
            {
                //Get SampleRate From Preferences
                int sampleRate = m_pConfig->getValueString(ConfigKey("[Soundcard]", "Samplerate")).toInt();
                //Calculate Latency
                int iLatencyMSec = (1000*asioDriverInfo->preferredSize*4)/(sampleRate);

                if (asioDriverInfo->sampleRate <= 0.0 || asioDriverInfo->sampleRate > 96000.0)
                {

                    if(ASIOSetSampleRate(sampleRate) == ASE_OK)
                    {
                        if(ASIOGetSampleRate(&asioDriverInfo->sampleRate) == ASE_OK)
                            qDebug("Asio State : SAMPLERATE CHANGED");
                        else
                            return -6;
                    }
                    else
                        return -5;
                } else if(asioDriverInfo->sampleRate != m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toInt())
                {

                    if(ASIOSetSampleRate(sampleRate) == ASE_OK)
                    {
                        if(ASIOGetSampleRate(&asioDriverInfo->sampleRate) == ASE_OK)
                            qDebug("Asio State : SAMPLERATE CHANGED");
                        else
                            return -6;
                    }
                    else
                        return -5;
                }


                // Driver does not store it's internal sample rate, so set it to a know one.
                // Usually you should check beforehand, that the selected sample rate is valid
                // with ASIOCanSampleRate().
                // Update SRATE and Latency ControlObjects
                m_pControlObjectSampleRate->queueFromThread(static_cast<double>(sampleRate));
                m_pControlObjectLatency->queueFromThread(static_cast<double>(iLatencyMSec));
                m_pConfig->set(ConfigKey("[Soundcard]","Latency"), iLatencyMSec);
                qDebug("Asio State: SAMPLERATE SET");

                // check wether the driver requires the ASIOOutputReady() optimization
                // (can be used by the driver to reduce output latency by one block)
                if(ASIOOutputReady() == ASE_OK)
                    asioDriverInfo->postOutput = true;
                else
                    asioDriverInfo->postOutput = false;


                return 0;
            }
            return -3;
        }
        return -2;
    }
    return -1;
}

ASIOError PlayerAsio::create_asio_buffers (DriverInfo * asioDriverInfo)
{

    long i=0;
    ASIOError result;

    bufferLength = asioDriverInfo->preferredSize;
    m_pTempBuffer = static_cast<float *>(malloc(bufferLength*4));

    // fill the bufferInfos from the start without a gap
    ASIOBufferInfo * info = asioDriverInfo->bufferInfos;

    asioDriverInfo->inputBuffers = 0;

    // prepare inputs (Though this is not necessaily required, no opened inputs will work, too
    if (asioDriverInfo->inputChannels > kMaxInputChannels)
        asioDriverInfo->inputBuffers = kMaxInputChannels;
    else
        asioDriverInfo->inputBuffers = asioDriverInfo->inputChannels;

    for(i = 0; i < asioDriverInfo->inputBuffers; i++, info++)
    {
        info->isInput = ASIOTrue;
        info->channelNum = i;
        info->buffers[0] = info->buffers[1] = 0;
    }
    // prepare outputs
    if (asioDriverInfo->outputChannels > kMaxOutputChannels)
        asioDriverInfo->outputBuffers = kMaxOutputChannels;
    else
        asioDriverInfo->outputBuffers = asioDriverInfo->outputChannels;

    qDebug(QString::number(asioDriverInfo->outputChannels));

    // prepare outputs
    if (asioDriverInfo->outputChannels > kMaxOutputChannels)
        asioDriverInfo->outputBuffers = kMaxOutputChannels;
    else
        asioDriverInfo->outputBuffers = asioDriverInfo->outputChannels;
    for(i = 0; i < asioDriverInfo->outputBuffers; i++, info++)
    {
        info->isInput = ASIOFalse;
        info->channelNum = i;
        info->buffers[0] = info->buffers[1] = 0;
    }

    // create and activate buffers
    result = ASIOCreateBuffers(asioDriverInfo->bufferInfos,
                               asioDriverInfo->outputBuffers+asioDriverInfo->inputBuffers,
                               asioDriverInfo->preferredSize, &asioCallbacks);

    if (result == ASE_OK)
    {
        // now get all the buffer details, sample word length, name, word clock group and activation
        for (i = 0; i < asioDriverInfo->inputBuffers + asioDriverInfo->outputBuffers; i++)
        {
            asioDriverInfo->channelInfos[i].channel = asioDriverInfo->bufferInfos[i].channelNum;
            asioDriverInfo->channelInfos[i].isInput = asioDriverInfo->bufferInfos[i].isInput;

            result = ASIOGetChannelInfo(&asioDriverInfo->channelInfos[i]);

            if (result != ASE_OK){
                qDebug("ERROR: Buffer init failed !");
                break;
            }
        }

        if (result == ASE_OK)
        {
            // get the input and output latencies
            // Latencies often are only valid after ASIOCreateBuffers()
            // (input latency is the age of the first sample in the currently returned audio block)
            // (output latency is the time the first sample in the currently returned audio block requires to get to the output)
            result = ASIOGetLatencies(&asioDriverInfo->inputLatency, &asioDriverInfo->outputLatency);
            //if (result == ASE_OK)
            //emit(initBuffers(asioDriverInfo->preferredSize));
        }
    }
    return result;
}

/*******
 *
 * Open / Close Calls
 *
 ********/
bool PlayerAsio::open()
{
    if (driverIsLoaded)
        close();

    QString name;

    name = m_pConfig->getValueString(ConfigKey("[Soundcard]","DeviceMasterLeft"));

    char driverName[MAX_DRIVER_NAME_LENGTH];
    strcpy(driverName, name.latin1());

    if (loadAsioDriver(driverName))
    {
        qDebug("Asio State: DRIVER LOADED");
        if(initDriver()){
            qDebug("Asio State: DRIVER INITIALIZED");
            init_asio_static_data (&asioDriverInfo);
            asioCallbacks.bufferSwitch = bufferSwitch;
            asioCallbacks.sampleRateDidChange = sampleRateChanged;
            asioCallbacks.asioMessage = asioMessages;
            asioCallbacks.bufferSwitchTimeInfo = bufferSwitchTimeInfo;
            create_asio_buffers(&asioDriverInfo);
            startAsio();
            return true;
        }
    }
    return false;
}

void PlayerAsio::startAsio()
{
    if (ASIOStart() == ASE_OK)
    {
        driverIsLoaded = true;
        qDebug("Asio streaming started");
    }
}

void PlayerAsio::close()
{
    if(ASIOStop() == ASE_OK)
    {
        driverIsLoaded = false;
        qDebug("Asio streaming stopped");
    }

    if(ASIOExit() == ASE_OK)
    {
        qDebug("Asio exited");
    }
}

void PlayerAsio::setDefaults()
{
    // Get list of interfaces
    QStringList interfaces = getInterfaces();

    // Set first interfaces to master left
    QStringList::iterator it = interfaces.begin();
    if (it!=interfaces.end())
    {
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterLeft"),ConfigValue((*it)));
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterRight"),ConfigValue((*it)));
    }
    else
    {
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterLeft"),ConfigValue("None"));
        m_pConfig->set(ConfigKey("[Soundcard]","DeviceMasterRight"),ConfigValue("None"));
    }

    // Set head left and right to none
    m_pConfig->set(ConfigKey("[Soundcard]","DeviceHeadLeft"),ConfigValue("None"));
    m_pConfig->set(ConfigKey("[Soundcard]","DeviceHeadRight"),ConfigValue("None"));

}

/*******
 *
 * Asio and mixxx Callbacks
 *
 ********/

//----------------------------------------------------------------------------------
void bufferSwitch(long index, ASIOBool processNow)
{   // the actual processing callback.

    ASIOTime timeInfo;
    memset (&timeInfo, 0, sizeof (timeInfo));


    bufferSwitchTimeInfo (&timeInfo, index, processNow);

}


//----------------------------------------------------------------------------------
void sampleRateChanged(ASIOSampleRate sRate)
{
    // do whatever you need to do if the sample rate changed
    // usually this only happens during external sync.
    // Audio processing is not stopped by the driver, actual sample rate
    // might not have even changed, maybe only the sample rate status of an
    // AES/EBU or S/PDIF digital input at the audio device.
    // You might have to update time/sample related conversion routines, etc.
}

//----------------------------------------------------------------------------------
long asioMessages(long selector, long value, void * message, double * opt)
{
    // currently the parameters "value", "message" and "opt" are not used.
    long ret = 0;
    switch(selector)
    {
    case kAsioSelectorSupported:
        if(value == kAsioResetRequest
           || value == kAsioEngineVersion
           || value == kAsioResyncRequest
           || value == kAsioLatenciesChanged
           // the following three were added for ASIO 2.0, you don't necessarily have to support them
           || value == kAsioSupportsTimeInfo
           || value == kAsioSupportsTimeCode
           || value == kAsioSupportsInputMonitor)
            ret = 1L;
        break;
    case kAsioResetRequest:
        // defer the task and perform the reset of the driver during the next "safe" situation
        // You cannot reset the driver right now, as this code is called from the driver.
        // Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
        // Afterwards you initialize the driver again.
        //asioDriverInfo.stopped;  // In this sample the processing will just stop
        ret = 1L;
        break;
    case kAsioResyncRequest:
        // This informs the application, that the driver encountered some non fatal data loss.
        // It is used for synchronization purposes of different media.
        // Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
        // Windows Multimedia system, which could loose data because the Mutex was hold too long
        // by another thread.
        // However a driver can issue it in other situations, too.
        ret = 1L;
        break;
    case kAsioLatenciesChanged:
        // This will inform the host application that the drivers were latencies changed.
        // Beware, it this does not mean that the buffer sizes have changed!
        // You might need to update internal delay data.
        ret = 1L;
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

void PlayerAsio::processCallback(long bufferIndex) {

    float * inputBuffer = prepareBuffer(bufferLength);
    // perform the processing
    for (int i = 0; i < asioDriverInfo.inputBuffers + asioDriverInfo.outputBuffers; i++)
    {
        if (asioDriverInfo.bufferInfos[i].isInput == false)
        {
            ASIOBufferInfo * bufferInfo = &asioDriverInfo.bufferInfos[i];

            m_reenter++;
            if (m_reenter > 1) {
                qDebug("ASIO : reentered callback ???!!!");
                return;
            }

            if (!driverIsLoaded) {
                qDebug("ASIO : callback and not started ???!!!");
            }


            bool swap = true;


            void * outputBuffer = bufferInfo->buffers[bufferIndex];
            ASIOSampleType bufferType = asioDriverInfo.channelInfos[i].type;
            switch (bufferType) {
            case ASIOSTInt16LSB:
                Output_Float32_Int16(i, inputBuffer, static_cast<short *>(outputBuffer), !swap);
                break;
            case ASIOSTInt16MSB:
                Output_Float32_Int16(i, inputBuffer, static_cast<short *>(outputBuffer), swap);
                break;
            case ASIOSTInt32LSB:
                Output_Float32_Int32(i, inputBuffer, static_cast<float *>(outputBuffer), !swap);
                break;
            case ASIOSTInt32MSB:
                Output_Float32_Int32(i, inputBuffer, static_cast<float *>(outputBuffer), swap);
                break;
            case ASIOSTFloat32LSB:
                //Output_Float32_Float32(i, inputBuffer, static_cast<float*>(outputBuffer), swap);
                break;
            case ASIOSTFloat32MSB:
                //Output_Float32_Float32(i, inputBuffer, static_cast<float*>(outputBuffer), !swap);
                memset(static_cast<float *>(outputBuffer),0,bufferLength*4);
                break;
            case ASIOSTInt24LSB:        // used for 20 bits as well
                Output_Float32_Int24(i, inputBuffer,static_cast<float *>(outputBuffer), !swap);
                //memset(static_cast<float*>(outputBuffer),0,bufferLength*4);
                break;
            case ASIOSTInt24MSB:        // used for 20 bits as well
            case ASIOSTFloat64LSB:      // IEEE 754 64 bit double float, as found on Intel x86 architecture
            case ASIOSTFloat64MSB:      // IEEE 754 64 bit double float, as found on Intel x86 architecture

                // these are used for 32 bit data buffer, with different alignment of the data inside
                // 32 bit PCI bus systems can more easily used with these

            case ASIOSTInt32LSB16:      // 32 bit data with 16 bit alignment
            case ASIOSTInt32LSB18:      // 32 bit data with 18 bit alignment
            case ASIOSTInt32LSB20:      // 32 bit data with 20 bit alignment
            case ASIOSTInt32LSB24:      // 32 bit data with 24 bit alignment


            case ASIOSTInt32MSB16:      // 32 bit data with 16 bit alignment
            case ASIOSTInt32MSB18:      // 32 bit data with 18 bit alignment
            case ASIOSTInt32MSB20:      // 32 bit data with 20 bit alignment
            case ASIOSTInt32MSB24:      // 32 bit data with 24 bit alignment
                // not implemented !!

                break;


            }

            m_reenter--;
        }
    }
    if (asioDriverInfo.postOutput)
        ASIOOutputReady();
}

ASIOTime * bufferSwitchTimeInfo(ASIOTime * timeInfo, long index, ASIOBool processNow)
{
    ((PlayerAsio *)(PlayerProxy::getPlayer()))->processCallback(index);

    return 0L;
}
