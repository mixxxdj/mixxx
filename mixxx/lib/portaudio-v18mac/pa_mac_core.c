/*
 * $Id: pa_mac_core.c 244 2003-01-27 10:12:56Z tuehaste $
 * pa_mac_core.c
 * Implementation of PortAudio for Mac OS X Core Audio
 *
 * PortAudio Portable Real-Time Audio Library
 * Latest Version at: http://www.portaudio.com
 *
 * Authors: Ross Bencina and Phil Burk
 * Copyright (c) 1999-2002 Ross Bencina and Phil Burk
 *
 * Theory of Operation
 *
 * This code uses the HAL (Hardware Access Layer) of the Apple CoreAudio library.
 * This is the layer closes to the hardware.
 * The HAL layer only supports the native HW supported sample rates.
 * So if the chip only supports 44100 Hz, then the HAL only supports 44100.
 * To provide other rates we use the handy Apple AUConverter which provides
 * sample rate conversion, mono-to-stereo conversion, and buffer size adaptation.
 *
 * License
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * CHANGE HISTORY:
 
 3.29.2001 - Phil Burk - First pass... converted from Window MME code with help from Darren.
 3.30.2001 - Darren Gibbs - Added more support for dynamically querying device info.
 12.7.2001 - Gord Peters - Tweaks to compile on PA V17 and OS X 10.1
 2.7.2002 - Darren and Phil - fixed isInput so GetProperty works better, 
             fixed device queries for numChannels and sampleRates,
            one CoreAudio device now maps to separate input and output PaDevices,
            audio input works if using same CoreAudio device (some HW devices make separate CoreAudio devices).
 2.22.2002 - Stephane Letz - Explicit cast needed for compilation with Code Warrior 7
 3.19.2002 - Phil Burk - Added paInt16, paInt8, format using new "pa_common/pa_convert.c" file.
            Return error if opened in mono mode cuz not supported. [Supported 10.12.2002]
            Add support for Pa_GetCPULoad();
            Fixed timestamp in callback and Pa_StreamTime() (Thanks n++k for the advice!)
            Check for invalid sample rates and return an error.
            Check for getenv("PA_MIN_LATENCY_MSEC") to set latency externally.
            Better error checking for invalid channel counts and invalid devices.
 3.29.2002 - Phil Burk - Fixed Pa_GetCPULoad() for small buffers.
 3.31.2002 - Phil Burk - Use getrusage() instead of gettimeofday() for CPU Load calculation.
 10.12.2002 - Phil Burk - Use AudioConverter to allow wide range of sample rates, and mono.
              Use FIFO (from pablio/rinbuffer.h) so that we can pull data through converter.
              Added PaOSX_FixVolumeScalar() to make iMic audible.
 10.17.2002 - Phil Burk - Support full duplex between two different devices.
              Name internal functions PaOSX_*
              Dumped useless PA_MIN_LATENCY_MSEC environment variable.
              Use kAudioDevicePropertyStreamFormatMatch to determine max channels.

TODO:
O- debug problem when changing sample rates on iMic
O- add support for paInt32 format
O- Why does iMic have grunge for the first second or two then clears up?
O- request notification when formats change or device unplugged
O- Why does patest_wire.c on iMic chop up sound when SR=34567Hz?
*/

#include <CoreServices/CoreServices.h>
#include <CoreAudio/CoreAudio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/DefaultAudioOutput.h>
#include <AudioToolbox/AudioConverter.h>

#include "portaudio.h"
#include "pa_host.h"
#include "pa_trace.h"
#include "ringbuffer.h"

/************************************************* Constants ********/

/* To trace program, enable TRACE_REALTIME_EVENTS in pa_trace.h */
#define PA_TRACE_RUN             (0)
#define PA_TRACE_START_STOP      (1)

#define PA_MIN_LATENCY_MSEC      (1)
#define MIN_TIMEOUT_MSEC         (1000)

#define PRINT(x) { printf x; fflush(stdout); }
#define PRINT_ERR( msg, err ) PRINT(( msg ": error = 0x%0lX = '%s'\n", (err), ErrorToString(err)) )
#define DBUG(x)    /* PRINT(x) */
#define DBUGBACK(x) /* if( sMaxBackgroundErrorMessages-- > 0 ) PRINT(x) */
#define DBUGX(x)

// define value of isInput passed to CoreAudio routines
#define IS_INPUT    (true)
#define IS_OUTPUT   (false)

typedef struct PaHostInOut
{
    AudioDeviceID      audioDeviceID; // CoreAudio specific ID
    int                bytesPerUserNativeBuffer; /* User buffer size in native host format. Depends on numChannels. */
    AudioConverterRef  converter;
    void              *converterBuffer;
} PaHostInOut;

/**************************************************************
 * Structure for internal host specific stream data.
 * This is allocated on a per stream basis.
 */
typedef struct PaHostSoundControl
{
    PaHostInOut        input;
    PaHostInOut        output;
    AudioDeviceID      primaryDeviceID;
    Boolean            usingSecondDevice;
    int                framesPerHostBuffer;
    /* For sample rate, format conversion, or when using two devices. */
    RingBuffer         ringBuffer;
    char              *ringBufferData;
    /* For measuring CPU utilization. */
    struct rusage      entryRusage;
    double             inverseMicrosPerHostBuffer; /* 1/Microseconds of real-time audio per user buffer. */
} PaHostSoundControl;

/**************************************************************
 * Structure for internal extended device info.
 * There will be one or two PortAudio devices for each Core Audio device:
 *   one input and or one output.
 */
typedef struct PaHostDeviceInfo
{
    PaDeviceInfo      paInfo;
    AudioDeviceID     audioDeviceID;
}
PaHostDeviceInfo;

/************************************************* Shared Data ********/
/* FIXME - put Mutex around this shared data. */
static int sNumPaDevices = 0;   /* Total number of PaDeviceInfos */
static int sNumInputDevices = 0; /* Total number of input PaDeviceInfos */
static int sNumOutputDevices = 0;
static PaHostDeviceInfo *sDeviceInfos = NULL;
static int sDefaultInputDeviceID = paNoDevice;
static int sDefaultOutputDeviceID = paNoDevice;
static int sSavedHostError = 0;
static int sNumCoreDevices = 0;
static AudioDeviceID *sCoreDeviceIDs;   // Array of Core AudioDeviceIDs

static const double supportedSampleRateRange[] = { 8000.0, 96000.0 };
static const char sMapperSuffixInput[] = " - Input";
static const char sMapperSuffixOutput[] = " - Output";

/* Debug support. */
//static int sMaxBackgroundErrorMessages = 100;
//static int sCoverageCounter = 1; // used to check code coverage during validation

/* We index the input devices first, then the output devices. */
#define LOWEST_INPUT_DEVID     (0)
#define HIGHEST_INPUT_DEVID    (sNumInputDevices - 1)
#define LOWEST_OUTPUT_DEVID    (sNumInputDevices)
#define HIGHEST_OUTPUT_DEVID   (sNumPaDevices - 1)

/************************************************* Macros ********/

/************************************************* Prototypes **********/

static PaError PaOSX_QueryDevices( void );
static int PaOSX_ScanDevices( Boolean isInput );
static int PaOSX_QueryDeviceInfo( PaHostDeviceInfo *hostDeviceInfo, int coreDeviceIndex, Boolean isInput );
static PaDeviceID PaOSX_QueryDefaultInputDevice( void );
static PaDeviceID PaOSX_QueryDefaultOutputDevice( void );
static void PaOSX_CalcHostBufferSize( internalPortAudioStream *past );

/**********************************************************************/
/* OS X errors are 4 character ID that can be printed.
 * Note that uses a static pad so result must be printed immediately.
 */
static OSStatus statusText[2] = { 0, 0 };
static const char *ErrorToString( OSStatus err )
{
    const char *str;

    switch (err)
    {
    case kAudioHardwareUnspecifiedError:
        str = "kAudioHardwareUnspecifiedError";
        break;
    case kAudioHardwareNotRunningError:
        str = "kAudioHardwareNotRunningError";
        break;
    case kAudioHardwareUnknownPropertyError:
        str = "kAudioHardwareUnknownPropertyError";
        break;
    case kAudioDeviceUnsupportedFormatError:
        str = "kAudioDeviceUnsupportedFormatError";
        break;
    case kAudioHardwareBadPropertySizeError:
        str = "kAudioHardwareBadPropertySizeError";
        break;
    case kAudioHardwareIllegalOperationError:
        str = "kAudioHardwareIllegalOperationError";
        break;
    default:
        str = "Unknown CoreAudio Error!";
        statusText[0] = err;
    	str = (const char *)statusText;
        break;
    }

    return str;
}

/**********************************************************************/
static unsigned long RoundUpToNextPowerOf2( unsigned long n )
{
    long numBits = 0;
    if( ((n-1) & n) == 0) return n; /* Already Power of two. */
    while( n > 0 )
    {
        n= n>>1;
        numBits++;
    }
    return (1<<numBits);
}

/********************************* BEGIN CPU UTILIZATION MEASUREMENT ****/
static void Pa_StartUsageCalculation( internalPortAudioStream   *past )
{
    PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
    if( pahsc == NULL ) return;
    /* Query user CPU timer for usage analysis and to prevent overuse of CPU. */
    getrusage( RUSAGE_SELF, &pahsc->entryRusage );
}

static long SubtractTime_AminusB( struct timeval *timeA, struct timeval *timeB )
{
    long secs = timeA->tv_sec - timeB->tv_sec;
    long usecs = secs * 1000000;
    usecs += (timeA->tv_usec - timeB->tv_usec);
    return usecs;
}

/******************************************************************************
** Measure fractional CPU load based on real-time it took to calculate
** buffers worth of output.
*/
static void Pa_EndUsageCalculation( internalPortAudioStream   *past )
{
    struct rusage currentRusage;
    long  usecsElapsed;
    double newUsage;

#define LOWPASS_COEFFICIENT_0   (0.95)
#define LOWPASS_COEFFICIENT_1   (0.99999 - LOWPASS_COEFFICIENT_0)

    PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
    if( pahsc == NULL ) return;
    
    if( getrusage( RUSAGE_SELF, &currentRusage ) == 0 )
    {
        usecsElapsed = SubtractTime_AminusB( &currentRusage.ru_utime, &pahsc->entryRusage.ru_utime );
        
        /* Use inverse because it is faster than the divide. */
        newUsage =  usecsElapsed * pahsc->inverseMicrosPerHostBuffer;

        past->past_Usage = (LOWPASS_COEFFICIENT_0 * past->past_Usage) +
                           (LOWPASS_COEFFICIENT_1 * newUsage);
    }
}
/****************************************** END CPU UTILIZATION *******/

/************************************************************************/
static PaDeviceID PaOSX_QueryDefaultInputDevice( void )
{
    OSStatus      err = noErr;
    UInt32        count;
    int           i;
    AudioDeviceID tempDeviceID = kAudioDeviceUnknown;
    PaDeviceID    defaultDeviceID = paNoDevice;

    // get the default output device for the HAL
    // it is required to pass the size of the data to be returned
    count = sizeof(tempDeviceID);
    err = AudioHardwareGetProperty( kAudioHardwarePropertyDefaultInputDevice,  &count, (void *) &tempDeviceID);
    if (err != noErr) goto error;
    
    // scan input devices to see which one matches this device
    defaultDeviceID = paNoDevice;
    for( i=LOWEST_INPUT_DEVID; i<=HIGHEST_INPUT_DEVID; i++ )
    {
        DBUG(("PaOSX_QueryDefaultInputDevice: i = %d, aDevId = %ld\n", i, sDeviceInfos[i].audioDeviceID ));
        if( sDeviceInfos[i].audioDeviceID == tempDeviceID )
        {
            defaultDeviceID = i;
            break;
        }
    }
error:
    return defaultDeviceID;
}

/************************************************************************/
static PaDeviceID PaOSX_QueryDefaultOutputDevice( void )
{
    OSStatus      err = noErr;
    UInt32        count;
    int           i;
    AudioDeviceID tempDeviceID = kAudioDeviceUnknown;
    PaDeviceID    defaultDeviceID = paNoDevice;

    // get the default output device for the HAL
    // it is required to pass the size of the data to be returned
    count = sizeof(tempDeviceID);
    err = AudioHardwareGetProperty( kAudioHardwarePropertyDefaultOutputDevice,  &count, (void *) &tempDeviceID);
    if (err != noErr) goto error;
    
    // scan output devices to see which one matches this device
    defaultDeviceID = paNoDevice;
    for( i=LOWEST_OUTPUT_DEVID; i<=HIGHEST_OUTPUT_DEVID; i++ )
    {
        DBUG(("PaOSX_QueryDefaultOutputDevice: i = %d, aDevId = %ld\n", i, sDeviceInfos[i].audioDeviceID ));
        if( sDeviceInfos[i].audioDeviceID == tempDeviceID )
        {
            defaultDeviceID = i;
            break;
        }
    }
error:
    return defaultDeviceID;
}

/******************************************************************/
static PaError PaOSX_QueryDevices( void )
{
    OSStatus err = noErr;
    UInt32   outSize;
    Boolean  outWritable;
    int      numBytes;

    // find out how many Core Audio devices there are, if any
    outSize = sizeof(outWritable);
    err = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices, &outSize, &outWritable);
    if (err != noErr)
    {
        PRINT_ERR("Couldn't get info about list of audio devices", err);
        sSavedHostError = err;
        return paHostError;
    }
        
    // calculate the number of device available
    sNumCoreDevices = outSize / sizeof(AudioDeviceID);

    // Bail if there aren't any devices
    if (sNumCoreDevices < 1)
    {
        PRINT(("No Devices Available"));
        return paHostError;
    }
    
    // make space for the devices we are about to get
    sCoreDeviceIDs =  (AudioDeviceID *)malloc(outSize);

    // get an array of AudioDeviceIDs
    err = AudioHardwareGetProperty(kAudioHardwarePropertyDevices, &outSize, (void *)sCoreDeviceIDs);
    if (err != noErr)
    {
        PRINT_ERR("Couldn't get list of audio device IDs", err);
        sSavedHostError = err;
        return paHostError;
    }

    // Allocate structures to hold device info pointers.
    // There will be a maximum of two Pa devices per Core Audio device, input and/or output.
    numBytes = sNumCoreDevices * 2 * sizeof(PaHostDeviceInfo);
    sDeviceInfos = (PaHostDeviceInfo *) PaHost_AllocateFastMemory( numBytes );
    if( sDeviceInfos == NULL ) return paInsufficientMemory;

    // Scan all the Core Audio devices to see which support input and allocate a
    // PaHostDeviceInfo structure for each one.
    PaOSX_ScanDevices( IS_INPUT );
    sNumInputDevices = sNumPaDevices;
    // Now scan all the output devices.
    PaOSX_ScanDevices( IS_OUTPUT );
    sNumOutputDevices = sNumPaDevices - sNumInputDevices;

    // Figure out which of the devices that we scanned is the default device.
    sDefaultInputDeviceID = PaOSX_QueryDefaultInputDevice();
    sDefaultOutputDeviceID = PaOSX_QueryDefaultOutputDevice();

    return paNoError;
}

/*************************************************************************/
/* Allocate a string containing the device name. */
static char *PaOSX_DeviceNameFromID(AudioDeviceID deviceID, Boolean isInput )
{
    OSStatus err = noErr;
    UInt32  outSize;
    Boolean  outWritable;
    char     *deviceName = nil;
    
    // query size of name
    err =  AudioDeviceGetPropertyInfo(deviceID, 0, isInput, kAudioDevicePropertyDeviceName, &outSize, &outWritable);
    if (err == noErr)
    {
        deviceName =  (char*)malloc( outSize + 1);
        if( deviceName )
        {
            err = AudioDeviceGetProperty(deviceID, 0, isInput, kAudioDevicePropertyDeviceName, &outSize, deviceName);
            if (err != noErr)
                PRINT_ERR("Couldn't get audio device name", err);
        }
    }

    return deviceName;
}

/*************************************************************************
** Scan all of the Core Audio devices to see which support input or output.
** Changes sNumDevices, and fills in sDeviceInfos.
*/
static int PaOSX_ScanDevices( Boolean isInput )
{
    int coreDeviceIndex;
    int result;
    PaHostDeviceInfo  *hostDeviceInfo;
    int numAdded = 0;

    for(  coreDeviceIndex=0; coreDeviceIndex<sNumCoreDevices; coreDeviceIndex++ )
    {
        // try to fill in next PaHostDeviceInfo
        hostDeviceInfo = &sDeviceInfos[sNumPaDevices];
        result = PaOSX_QueryDeviceInfo( hostDeviceInfo, coreDeviceIndex, isInput );
        DBUGX(("PaOSX_ScanDevices: paDevId = %d, coreDevId = %d\n", sNumPaDevices, coreDeviceIndex ));
        if( result > 0 )
        {
            sNumPaDevices += 1;  // bump global counter if we got one
            numAdded += 1;
        }
        else if( result < 0 ) return result;
    }
    return numAdded;
}


/*************************************************************************
** Try to fill in the device info for this device.
** Return 1 if a good device that PA can use.
** Return 0 if not appropriate
** or return negative error.
**
*/
static int PaOSX_QueryDeviceInfo( PaHostDeviceInfo *hostDeviceInfo, int coreDeviceIndex, Boolean isInput )
{
    OSStatus         err;
    UInt32           outSize;
    AudioStreamBasicDescription formatDesc;
    AudioDeviceID    devID;
    PaDeviceInfo    *deviceInfo = &hostDeviceInfo->paInfo;

    deviceInfo->structVersion = 1;
    deviceInfo->maxInputChannels = 0;
    deviceInfo->maxOutputChannels = 0;

    deviceInfo->sampleRates = supportedSampleRateRange; // because we use sample rate converter to get continuous rates
    deviceInfo->numSampleRates = -1;

    devID = sCoreDeviceIDs[ coreDeviceIndex ];
    hostDeviceInfo->audioDeviceID = devID;
    DBUG(("PaOSX_QueryDeviceInfo: coreDeviceIndex = %d, devID = %d, isInput = %d\n",
        coreDeviceIndex, devID, isInput ));
    // Get data format info from the device.
    outSize = sizeof(formatDesc);
    err = AudioDeviceGetProperty(devID, 0, isInput, kAudioDevicePropertyStreamFormat, &outSize, &formatDesc);
    // This just may not be an appropriate device for input or output so leave quietly.
    if( (err != noErr)  || (formatDesc.mChannelsPerFrame == 0) ) goto error;

    // Right now the Core Audio headers only define one formatID: LinearPCM
    // Apparently LinearPCM must be Float32 for now.
    if( (formatDesc.mFormatID == kAudioFormatLinearPCM) &&
        (formatDesc.mFormatFlags & kLinearPCMFormatFlagIsFloat) )
    {
        deviceInfo->nativeSampleFormats = paFloat32;
    }
    else
    {
        return paSampleFormatNotSupported;
    }

    // Determine maximum number of channels supported.
    memset( &formatDesc, 0, sizeof(formatDesc));
    formatDesc.mChannelsPerFrame = 256; // FIXME - what about device with > 256 channels
    outSize = sizeof(formatDesc);
    err = AudioDeviceGetProperty( devID, 0,
        isInput, kAudioDevicePropertyStreamFormatMatch, &outSize, &formatDesc);
    if( err != noErr )
    {
        PRINT_ERR("PaOSX_QueryDeviceInfo: Could not get device format match", err);
        sSavedHostError = err;
        return paHostError;
    }

    if( isInput )
    {
        deviceInfo->maxInputChannels = formatDesc.mChannelsPerFrame;
    }
    else
    {
        deviceInfo->maxOutputChannels = formatDesc.mChannelsPerFrame;
    }

    // Get the device name
    deviceInfo->name = PaOSX_DeviceNameFromID( devID, isInput );
    return 1;

error:
    return 0;
}

/**********************************************************************/
static PaError PaOSX_MaybeQueryDevices( void )
{
    if( sNumPaDevices == 0 )
    {
        return PaOSX_QueryDevices();
    }
    return 0;
}

static char zeroPad[256] = { 0 };

/**********************************************************************
** This is the proc that supplies the data to the AudioConverterFillBuffer call.
** We can pass back arbitrarily sized blocks so if the FIFO region is split
** just pass back the first half.
*/
static OSStatus PaOSX_InputConverterCallbackProc (AudioConverterRef			inAudioConverter,
								UInt32*						outDataSize,
								void**						outData,
								void*						inUserData)
{
    internalPortAudioStream   *past = (internalPortAudioStream *) inUserData;
    PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
    void *dataPtr1;
    long size1;
    void *dataPtr2;
    long size2;
           
    /* Pass contiguous region from FIFO directly to converter. */
    RingBuffer_GetReadRegions( &pahsc->ringBuffer, *outDataSize,
            &dataPtr1, &size1, &dataPtr2, &size2 );

    if( size1 > 0 )
    {
        *outData = dataPtr1;
        *outDataSize = size1;
        RingBuffer_AdvanceReadIndex( &pahsc->ringBuffer, size1 );
        DBUGX(("PaOSX_InputConverterCallbackProc: read %ld bytes from FIFO.\n", size1 ));
    }
    else
    {
        DBUGBACK(("PaOSX_InputConverterCallbackProc: got no data!\n"));
        *outData = zeroPad; // Give it zero data to keep it happy.
        *outDataSize = sizeof(zeroPad);
    }
	return noErr;
}

/*****************************************************************************
** Get audio input, if any, from passed in buffer, or from converter or from FIFO
** and run PA callback.
*/
static OSStatus PaOSX_LoadAndProcess( internalPortAudioStream   *past,
    void *inputBuffer, void *outputBuffer )
{
    OSStatus err = noErr;
    PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
    
    if( past->past_StopSoon )
    {
        if( outputBuffer )
        {
            /* Clear remainder of audio buffer if we are waiting for stop. */
            AddTraceMessage("PaOSX_HandleInputOutput: zero rest of wave buffer ", i );
            memset( outputBuffer, 0, pahsc->output.bytesPerUserNativeBuffer );
        }
    }
    else
    {
        /* Do we need data from the converted input? */
        if( pahsc->input.converter != NULL )
        {
            UInt32 size = pahsc->input.bytesPerUserNativeBuffer;
            err = AudioConverterFillBuffer(
                pahsc->input.converter,
                PaOSX_InputConverterCallbackProc,
                past,
                &size,
                pahsc->input.converterBuffer);
            if( err != noErr ) return err;
            inputBuffer = pahsc->input.converterBuffer;
        }
        /* Or should just get the data directly from the FIFO? */
        else if( pahsc->ringBufferData != NULL )
        {
            if( RingBuffer_GetReadAvailable( &pahsc->ringBuffer ) >= pahsc->input.bytesPerUserNativeBuffer)
            {
                RingBuffer_Read(  &pahsc->ringBuffer, pahsc->input.converterBuffer, pahsc->input.bytesPerUserNativeBuffer );
                inputBuffer = pahsc->input.converterBuffer;
            }
        }
        
        /* Fill part of audio converter buffer by converting input to user format,
        * calling user callback, then converting output to native format. */
        if( PaConvert_Process( past, inputBuffer, outputBuffer ))
        {
            past->past_StopSoon = 1;
        }
    }
    return err;
}

/*****************************************************************************
** This is the proc that supplies the data to the AudioConverterFillBuffer call
*/
static OSStatus PaOSX_OutputConverterCallbackProc (AudioConverterRef			inAudioConverter,
								UInt32*						outDataSize,
								void**						outData,
								void*						inUserData)
{
    internalPortAudioStream   *past = (internalPortAudioStream *) inUserData;
    PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
    
	*outData = pahsc->output.converterBuffer;
	*outDataSize = pahsc->output.bytesPerUserNativeBuffer;
    
	return PaOSX_LoadAndProcess ( past, pahsc->input.converterBuffer, pahsc->output.converterBuffer );
}

/**********************************************************************
** Fill any available output buffers and use any available
** input buffers by calling user callback.
** Will set past->past_StopSoon if user callback indicates that it is finished.
*/
static OSStatus PaOSX_HandleInputOutput( internalPortAudioStream   *past,
        const AudioBufferList*  inInputData,
        AudioBufferList*  outOutputData )
{
    OSStatus            err = noErr;
    char               *inputNativeBufferfPtr = NULL;
    char               *outputNativeBufferfPtr = NULL;
    int                 i;
    PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;

    /* If we are using output, then we need an empty output buffer. */
    if( outOutputData->mNumberBuffers > 0 )
    {
        outputNativeBufferfPtr =  (char*)outOutputData->mBuffers[0].mData;
    }

    if(  inInputData->mNumberBuffers > 0  )
    {
        inputNativeBufferfPtr = (char*)inInputData->mBuffers[0].mData;
    
        /* If there is a FIFO for input then write to it. */
        if( (pahsc->ringBufferData != NULL) && !pahsc->usingSecondDevice )
        {
            long writeRoom = RingBuffer_GetWriteAvailable( &pahsc->ringBuffer );
            long numBytes = inInputData->mBuffers[0].mDataByteSize;
            if( numBytes <= writeRoom )
            {
                RingBuffer_Write(  &pahsc->ringBuffer, inputNativeBufferfPtr, numBytes );
                DBUGBACK(("PaOSX_HandleInputOutput: wrote %ld bytes to FIFO.\n", inInputData->mBuffers[0].mDataByteSize));
            } // FIXME else ???            
        }
    }
    
    if( pahsc->output.converter != NULL )
    {
        /* Using output and input converter. */
        UInt32 size = outOutputData->mBuffers[0].mDataByteSize;
        err = AudioConverterFillBuffer(
            pahsc->output.converter,
            PaOSX_OutputConverterCallbackProc,
            past,
            &size,
            outputNativeBufferfPtr);
        if( err != noErr )
        {
            PRINT_ERR("PaOSX_HandleInputOutput: AudioConverterFillBuffer failed", err);
            goto error;
        }
    }
    else if( (pahsc->input.converter != NULL) && !pahsc->usingSecondDevice)
    {
        /* Using just an input converter. */
        /* Generate user buffers as long as we have a half full input FIFO. */
        long gotHalf = pahsc->ringBuffer.bufferSize / 2;
        while( (RingBuffer_GetReadAvailable( &pahsc->ringBuffer ) >= gotHalf) &&
            (past->past_StopSoon == 0) )
        {
            err = PaOSX_LoadAndProcess ( past, NULL, outputNativeBufferfPtr );
            if( err != noErr ) goto error;
            if( outputNativeBufferfPtr) outputNativeBufferfPtr += pahsc->output.bytesPerUserNativeBuffer;
        }
    }
    else
    {
        /* No AUConverters used. */
        /* Each host buffer contains multiple user buffers so do them all now. */
        for( i=0; i<past->past_NumUserBuffers; i++ )
        {
            err = PaOSX_LoadAndProcess ( past, inputNativeBufferfPtr, outputNativeBufferfPtr );
            if( err != noErr ) goto error;
            if( inputNativeBufferfPtr ) inputNativeBufferfPtr += pahsc->input.bytesPerUserNativeBuffer;
            if( outputNativeBufferfPtr) outputNativeBufferfPtr += pahsc->output.bytesPerUserNativeBuffer;
        }
    }
    
error:
    return err;
}

/******************************************************************
 * This callback is used when two separate devices are used for input and output.
 * This often happens when using USB devices which present as two devices: input and output.
 * It just writes its data to a FIFO so that it can be read by the main callback
 * proc PaOSX_CoreAudioIOCallback().
 */
static OSStatus PaOSX_CoreAudioInputCallback (AudioDeviceID  inDevice, const AudioTimeStamp*  inNow,
                    const AudioBufferList*  inInputData, const AudioTimeStamp*  inInputTime,
                    AudioBufferList*  outOutputData, const AudioTimeStamp* inOutputTime,
                    void* contextPtr)
{
    internalPortAudioStream *past = (internalPortAudioStream *) contextPtr;
    PaHostSoundControl *pahsc;
    pahsc = (PaHostSoundControl *) past->past_DeviceData;
   
    /* If there is a FIFO for input then write to it. */
    if( pahsc->ringBufferData != NULL )
    {
        long writeRoom = RingBuffer_GetWriteAvailable( &pahsc->ringBuffer );
        long numBytes = inInputData->mBuffers[0].mDataByteSize;
        if( numBytes <= writeRoom )
        {
            RingBuffer_Write(  &pahsc->ringBuffer, inInputData->mBuffers[0].mData, inInputData->mBuffers[0].mDataByteSize );
        }
        else
        {
            DBUGBACK(("PaOSX_CoreAudioInputCallback: FIFO too full to write!\n"));
        }            
    }
    
    return noErr;
}

/******************************************************************
 * This is the primary callback for CoreAudio.
 * It can handle input and/or output for a single device.
 * It takes input from CoreAudio, converts it and passes it to the
 * PortAudio callback. Then takes the PA results and passes it back to CoreAudio.
 */
static OSStatus PaOSX_CoreAudioIOCallback (AudioDeviceID  inDevice, const AudioTimeStamp*  inNow,
                    const AudioBufferList*  inInputData, const AudioTimeStamp*  inInputTime,
                    AudioBufferList*  outOutputData, const AudioTimeStamp* inOutputTime,
                    void* contextPtr)
{
    OSStatus      err = noErr;
    internalPortAudioStream *past;
    PaHostSoundControl *pahsc;
    past = (internalPortAudioStream *) contextPtr;
    pahsc = (PaHostSoundControl *) past->past_DeviceData;

    /* Has someone asked us to abort by calling Pa_AbortStream()? */
    if( past->past_StopNow )
    {
        past->past_IsActive = 0; /* Will cause thread to return. */
    }
    /* Has someone asked us to stop by calling Pa_StopStream()
     * OR has a user callback returned '1' to indicate finished.
     */
    else if( past->past_StopSoon )
    {
        // FIXME - Pretend all done. Should wait for audio to play out but CoreAudio latency very low.
        past->past_IsActive = 0; /* Will cause thread to return. */
    }
    else
    {
        /* use time stamp from CoreAudio if valid */
        if( inOutputTime->mFlags & kAudioTimeStampSampleTimeValid) 
        {
            past->past_FrameCount = inOutputTime->mSampleTime;
        }
        
        /* Measure CPU load. */
        Pa_StartUsageCalculation( past );
        past->past_NumCallbacks += 1;
        
        /* Process full input buffer and fill up empty output buffers. */
        err = PaOSX_HandleInputOutput( past, inInputData, outOutputData );
        
        Pa_EndUsageCalculation( past );
    }

    // FIXME PaOSX_UpdateStreamTime( pahsc );
    if( err != 0 ) DBUG(("PaOSX_CoreAudioIOCallback: returns %ld.\n", err ));

    return err;
}

/*******************************************************************/
/* Attempt to set device sample rate. */
static PaError PaOSX_SetSampleRate( AudioDeviceID devID, Boolean isInput, double sampleRate )
{
    AudioStreamBasicDescription formatDesc;
    PaError  result = paNoError;
    OSStatus err;
    UInt32   dataSize;
     
    // try to set to desired rate
    memset( &formatDesc, 0, sizeof(AudioStreamBasicDescription) );
    formatDesc.mSampleRate = sampleRate;
    err = AudioDeviceSetProperty( devID, 0, 0,
        isInput, kAudioDevicePropertyStreamFormat, sizeof(formatDesc), &formatDesc);
    if (err != noErr)
    {
        result = paInvalidSampleRate;
        
        /* Could not set to desired rate so query for closest match. */
        DBUG(("PaOSX_SetSampleRate: couldn't set to %f. Try to find match.\n", sampleRate ));
        dataSize = sizeof(formatDesc);
        AudioDeviceGetProperty( devID, 0,
            isInput, kAudioDevicePropertyStreamFormatMatch, &dataSize, &formatDesc);
        formatDesc.mSampleRate = sampleRate;
        err = AudioDeviceGetProperty( devID, 0,
            isInput, kAudioDevicePropertyStreamFormatMatch, &dataSize, &formatDesc);
        if (err == noErr)
        {
            /* Set to that matching rate. */
            sampleRate = formatDesc.mSampleRate;
            DBUG(("PaOSX_SetSampleRate: match succeeded, set to %f instead\n", sampleRate ));
            memset( &formatDesc, 0, sizeof(AudioStreamBasicDescription) );
            formatDesc.mSampleRate = sampleRate;
            AudioDeviceSetProperty( devID, 0, 0,
                isInput, kAudioDevicePropertyStreamFormat, sizeof(formatDesc), &formatDesc);
        }
    }
    return result;
}

/*******************************************************************
 * Check volume level of device. If below threshold, then set to newLevel.
 * Using volume instead of decibels because decibel range varies by device.
 */
static void PaOSX_FixVolumeScalars( AudioDeviceID devID, Boolean isInput,
    int numChannels, double threshold, double newLevel )
{
    OSStatus err = noErr;
    UInt32    dataSize;
    int       iChannel;

/* The master channel is 0. Left and right are channels 1 and 2. */
/* Fix volume. */
    for( iChannel = 0; iChannel<=numChannels; iChannel++ )
    {
        Float32   fdata32;
        dataSize = sizeof( fdata32 );
        err = AudioDeviceGetProperty( devID, iChannel, isInput, 
            kAudioDevicePropertyVolumeScalar, &dataSize, &fdata32 );
        if( err == noErr )
        {
            DBUG(("kAudioDevicePropertyVolumeScalar for channel %d = %f\n", iChannel, fdata32));
            if( fdata32 <= (Float32) threshold )
            {
                dataSize = sizeof( fdata32 );
                fdata32 = (Float32) newLevel;
                err = AudioDeviceSetProperty( devID, 0, iChannel, isInput, 
                    kAudioDevicePropertyVolumeScalar, dataSize, &fdata32 );
                if( err != noErr )
                {
                    PRINT(("Warning: audio volume is very low and could not be turned up.\n"));
                }
                else
                {
                    PRINT(("Volume for audio channel %d was <= %4.2f so set to %4.2f by PortAudio!\n",
                        iChannel, threshold, newLevel ));
                }
            }
        }
    }
/* Unmute if muted. */
    for( iChannel = 0; iChannel<=numChannels; iChannel++ )
    {
        UInt32    uidata32;
        dataSize = sizeof( uidata32 );
        err = AudioDeviceGetProperty( devID, iChannel, isInput, 
            kAudioDevicePropertyMute, &dataSize, &uidata32 );
        if( err == noErr )
        {
            DBUG(("uidata32 for channel %d = %ld\n", iChannel, uidata32));
            if( uidata32 == 1 ) // muted?
            {
                dataSize = sizeof( uidata32 );
                uidata32 = 0; // unmute
                err = AudioDeviceSetProperty( devID, 0, iChannel, isInput, 
                    kAudioDevicePropertyMute, dataSize, &uidata32 );
                if( err != noErr )
                {
                    PRINT(("Warning: audio is muted and could not be unmuted!\n"));
                }
                else
                {
                    PRINT(("Audio channel %d was unmuted by PortAudio!\n", iChannel ));
                }
            }
        }
    }

}

#if 0
static void PaOSX_DumpDeviceInfo( AudioDeviceID devID, Boolean isInput )
{
    OSStatus err = noErr;
    UInt32    dataSize;
    UInt32    uidata32;
    Float32   fdata32;
    AudioValueRange audioRange;
    
    dataSize = sizeof( uidata32 );
    err = AudioDeviceGetProperty( devID, 0, isInput, 
        kAudioDevicePropertyLatency, &dataSize, &uidata32 );
    if( err != noErr )
    {
        PRINT_ERR("Error reading kAudioDevicePropertyLatency", err);
        return;
    }
    PRINT(("kAudioDevicePropertyLatency = %d\n", (int)uidata32 ));
    
    dataSize = sizeof( fdata32 );
    err = AudioDeviceGetProperty( devID, 1, isInput, 
        kAudioDevicePropertyVolumeScalar, &dataSize, &fdata32 );
    if( err != noErr )
    {
        PRINT_ERR("Error reading kAudioDevicePropertyVolumeScalar", err);
        return;
    }
    PRINT(("kAudioDevicePropertyVolumeScalar = %f\n", fdata32 ));
    
    dataSize = sizeof( uidata32 );
    err = AudioDeviceGetProperty( devID, 0, isInput, 
        kAudioDevicePropertyBufferSize, &dataSize, &uidata32 );
    if( err != noErr )
    {
        PRINT_ERR("Error reading buffer size", err);
        return;
    }
    PRINT(("kAudioDevicePropertyBufferSize = %d bytes\n", (int)uidata32 ));

    dataSize = sizeof( audioRange );
    err = AudioDeviceGetProperty( devID, 0, isInput, 
        kAudioDevicePropertyBufferSizeRange, &dataSize, &audioRange );
    if( err != noErr )
    {
        PRINT_ERR("Error reading buffer size range", err);
        return;
    }
    PRINT(("kAudioDevicePropertyBufferSizeRange = %g to %g bytes\n", audioRange.mMinimum, audioRange.mMaximum ));
    
    dataSize = sizeof( uidata32 );
    err = AudioDeviceGetProperty( devID, 0, isInput, 
        kAudioDevicePropertyBufferFrameSize, &dataSize, &uidata32 );
    if( err != noErr )
    {
        PRINT_ERR("Error reading buffer size", err);
        return;
    }
    PRINT(("kAudioDevicePropertyBufferFrameSize = %d frames\n", (int)uidata32 ));
    
    dataSize = sizeof( audioRange );
    err = AudioDeviceGetProperty( devID, 0, isInput, 
        kAudioDevicePropertyBufferFrameSizeRange, &dataSize, &audioRange );
    if( err != noErr )
    {
        PRINT_ERR("Error reading buffer size range", err);
        return;
    }
    PRINT(("kAudioDevicePropertyBufferFrameSizeRange = %g to %g frames\n", audioRange.mMinimum, audioRange.mMaximum ));

    return;
}
#endif

/*******************************************************************/
static PaError PaOSX_OpenInputDevice( internalPortAudioStream   *past )
{
    PaHostSoundControl *pahsc;
    const PaHostDeviceInfo *hostDeviceInfo;
    PaError          result = paNoError;
    UInt32           dataSize;
    OSStatus         err = noErr;
    int              needConverter = 0;
    double           deviceRate = past->past_SampleRate;

    DBUG(("PaOSX_OpenInputDevice: -------------\n"));

    pahsc = (PaHostSoundControl *) past->past_DeviceData;
    
    if( (past->past_InputDeviceID < LOWEST_INPUT_DEVID) ||
        (past->past_InputDeviceID > HIGHEST_INPUT_DEVID) )
    {
        return paInvalidDeviceId;
    }
    hostDeviceInfo = &sDeviceInfos[past->past_InputDeviceID];

    PaOSX_FixVolumeScalars( pahsc->input.audioDeviceID, IS_INPUT,
        hostDeviceInfo->paInfo.maxInputChannels, 0.1, 0.9 );

    /* Try to set sample rate. */
    result = PaOSX_SetSampleRate( pahsc->input.audioDeviceID, IS_INPUT, past->past_SampleRate );
	if( result != paNoError )
    {
        DBUG(("PaOSX_OpenInputDevice: Need converter for sample rate = %f\n", past->past_SampleRate ));
        needConverter = 1;
        result = paNoError;
    }
    else
    {
        DBUG(("PaOSX_OpenInputDevice: successfully set sample rate to %f\n", past->past_SampleRate ));
    }

    /* Try to set number of channels. */ 
    if( past->past_NumInputChannels > hostDeviceInfo->paInfo.maxInputChannels )
    {
        return paInvalidChannelCount; /* Too many channels! */
    }
    else if( past->past_NumInputChannels < hostDeviceInfo->paInfo.maxInputChannels )
    {
    
        AudioStreamBasicDescription formatDesc;
        OSStatus err;
        memset( &formatDesc, 0, sizeof(AudioStreamBasicDescription) );
        formatDesc.mChannelsPerFrame = past->past_NumInputChannels;
        err = AudioDeviceSetProperty( pahsc->input.audioDeviceID, 0, 0,
            IS_INPUT, kAudioDevicePropertyStreamFormat, sizeof(formatDesc), &formatDesc);
        if (err != noErr)
        {
            needConverter = 1;
        }
    }

    /* Try to set the I/O bufferSize of the device. */
    dataSize = sizeof(pahsc->framesPerHostBuffer);
    err = AudioDeviceSetProperty( pahsc->input.audioDeviceID, 0, 0, IS_INPUT,
                                kAudioDevicePropertyBufferFrameSize, dataSize,
                                &pahsc->framesPerHostBuffer);
    if( err != noErr )
    {
        DBUG(("PaOSX_OpenInputDevice: Need converter for buffer size = %d\n", pahsc->framesPerHostBuffer));
        needConverter = 1;
    }
    
    // setup PA conversion procedure
    result = PaConvert_SetupInput( past, paFloat32 );
    
    if( needConverter )
    {
        AudioStreamBasicDescription sourceStreamFormat, destStreamFormat;
        
        /* Get source device format */
        dataSize = sizeof(sourceStreamFormat);
        err = AudioDeviceGetProperty(pahsc->input.audioDeviceID, 0, IS_INPUT,
            kAudioDevicePropertyStreamFormat, &dataSize, &sourceStreamFormat);
        if( err != noErr )
        {
            PRINT_ERR("PaOSX_OpenInputDevice: Could not get input device format", err);
            sSavedHostError = err;
            return paHostError;
        }
        deviceRate = sourceStreamFormat.mSampleRate;
        DBUG(("PaOSX_OpenInputDevice: current device sample rate = %f\n", deviceRate ));
        
        /* Set target user format. */
        destStreamFormat = sourceStreamFormat;
        destStreamFormat.mSampleRate = past->past_SampleRate;	// sample rate of the user synthesis code
        destStreamFormat.mChannelsPerFrame = past->past_NumInputChannels;	//	the number of channels in each frame
                
        err = AudioConverterNew (
            &sourceStreamFormat, 
            &destStreamFormat, 
            &pahsc->input.converter);
        if( err != noErr )
        {
            PRINT_ERR("Could not create input format converter", err);
            sSavedHostError = err;
            return paHostError;
        }
    }
    
    /* Allocate FIFO between Device callback and Converter callback so that device can push data
    * and converter can pull data.
    */
    if( needConverter || pahsc->usingSecondDevice )
    {
        double sampleRateRatio;
        long minSize, numBytes;
        
        /* Allocate an input buffer because we need it between the user callback and the converter. */
        pahsc->input.converterBuffer = PaHost_AllocateFastMemory( pahsc->input.bytesPerUserNativeBuffer );
        if( pahsc->input.converterBuffer == NULL )
        {
            return paInsufficientMemory;
        }

        sampleRateRatio = deviceRate / past->past_SampleRate;
        minSize = pahsc->input.bytesPerUserNativeBuffer * 4 * sampleRateRatio;
        numBytes = RoundUpToNextPowerOf2( minSize );
        DBUG(("PaOSX_OpenInputDevice: FIFO numBytes = %ld\n", numBytes));
        pahsc->ringBufferData = PaHost_AllocateFastMemory( numBytes );
        if( pahsc->ringBufferData == NULL )
        {
            return paInsufficientMemory;
        }
        RingBuffer_Init( &pahsc->ringBuffer, numBytes, pahsc->ringBufferData );
        // make it look full at beginning
        RingBuffer_AdvanceWriteIndex( &pahsc->ringBuffer, numBytes );
    }
    
    return result;
}

/*******************************************************************/
static PaError PaOSX_OpenOutputDevice( internalPortAudioStream *past )
{
    PaHostSoundControl *pahsc;
    const PaHostDeviceInfo *hostDeviceInfo;
    PaError          result = paNoError;
    UInt32           dataSize;
    OSStatus         err = noErr;
    int              needConverter = 0;
    
    DBUG(("PaOSX_OpenOutputDevice: -------------\n"));

    pahsc = (PaHostSoundControl *) past->past_DeviceData;
    
    DBUG(("PaHost_OpenStream: deviceID = 0x%x\n", past->past_OutputDeviceID));
    if( (past->past_OutputDeviceID < LOWEST_OUTPUT_DEVID) ||
        (past->past_OutputDeviceID > HIGHEST_OUTPUT_DEVID) )
    {
        return paInvalidDeviceId;
    }
    
    hostDeviceInfo = &sDeviceInfos[past->past_OutputDeviceID];
    
    //PaOSX_DumpDeviceInfo( pahsc->output.audioDeviceID, IS_OUTPUT );

    PaOSX_FixVolumeScalars( pahsc->output.audioDeviceID, IS_OUTPUT,
        hostDeviceInfo->paInfo.maxOutputChannels, 0.1, 0.9 );
    
    /* Try to set sample rate. */
    result = PaOSX_SetSampleRate( pahsc->output.audioDeviceID, IS_OUTPUT, past->past_SampleRate );
	if( result != paNoError )
    {
        DBUG(("PaOSX_OpenOutputDevice: Need converter for sample rate = %f\n", past->past_SampleRate ));
        needConverter = 1;
        result = paNoError;
    }
    else
    {
        DBUG(("PaOSX_OpenOutputDevice: successfully set sample rate to %f\n", past->past_SampleRate ));
    }

    if( past->past_NumOutputChannels > hostDeviceInfo->paInfo.maxOutputChannels )
    {
        return paInvalidChannelCount; /* Too many channels! */
    }
    else
    {
    /* Attempt to set number of channels. */ 
        AudioStreamBasicDescription formatDesc;
        OSStatus err;
        memset( &formatDesc, 0, sizeof(AudioStreamBasicDescription) );
        formatDesc.mChannelsPerFrame = past->past_NumOutputChannels;
        err = AudioDeviceSetProperty( pahsc->output.audioDeviceID, 0, 0,
            IS_OUTPUT, kAudioDevicePropertyStreamFormat, sizeof(formatDesc), &formatDesc);
        if (err != kAudioHardwareNoError)
        {
            DBUG(("PaOSX_OpenOutputDevice: Need converter for num channels.\n"));
            needConverter = 1;
        }
    }

    /* Change the I/O bufferSize of the device. */
    dataSize = sizeof(pahsc->framesPerHostBuffer);
    err = AudioDeviceSetProperty( pahsc->output.audioDeviceID, 0, 0, IS_OUTPUT,
                                  kAudioDevicePropertyBufferFrameSize, dataSize,
                                  &pahsc->framesPerHostBuffer);
    if( err != noErr )
    {
        DBUG(("PaOSX_OpenOutputDevice: Need converter for buffer size = %d\n", pahsc->framesPerHostBuffer));
        needConverter = 1;
    }
        
    // setup conversion procedure
    result = PaConvert_SetupOutput( past, paFloat32 );
    
    if( needConverter )
    {
        AudioStreamBasicDescription sourceStreamFormat, destStreamFormat;
        DBUG(("PaOSX_OpenOutputDevice: using AUConverter!\n"));
        /* Get target device format */
        dataSize = sizeof(destStreamFormat);
        err = AudioDeviceGetProperty(pahsc->output.audioDeviceID, 0, IS_OUTPUT,
            kAudioDevicePropertyStreamFormat, &dataSize, &destStreamFormat);
        if( err != noErr )
        {
            PRINT_ERR("PaOSX_OpenOutputDevice: Could not get output device format", err);
            sSavedHostError = err;
            return paHostError;
        }

        /* Set source user format. */
        sourceStreamFormat = destStreamFormat;
        sourceStreamFormat.mSampleRate = past->past_SampleRate;	// sample rate of the user synthesis code
        sourceStreamFormat.mChannelsPerFrame = past->past_NumOutputChannels;	//	the number of channels in each frame
                
        /* Allocate an output buffer because we need it between the user callback and the converter. */
        pahsc->output.converterBuffer = PaHost_AllocateFastMemory( pahsc->output.bytesPerUserNativeBuffer );
        err = AudioConverterNew (
            &sourceStreamFormat, 
            &destStreamFormat, 
            &pahsc->output.converter);
        if( err != noErr )
        {
            PRINT_ERR("Could not create output format converter", err);
            sSavedHostError = err;
            return paHostError;
        }
    }
    
    return result;
}

/*******************************************************************
* Determine how many User Buffers we can put into our CoreAudio stream buffer.
* Uses:
*    past->past_FramesPerUserBuffer, etc.
* Sets:
*    past->past_NumUserBuffers
*    pahsc->framesPerHostBuffer
*    pahsc->input.bytesPerUserNativeBuffer
*    pahsc->output.bytesPerUserNativeBuffer
*/
static void PaOSX_CalcHostBufferSize( internalPortAudioStream *past )
{
    PaHostSoundControl *pahsc = ( PaHostSoundControl *)past->past_DeviceData;

    // Determine number of user buffers based strictly on minimum reasonable buffer size.
    // We ignore the Pa_OpenStream numBuffer parameter because CoreAudio has a big
    // mix buffer and handles latency automatically.
    past->past_NumUserBuffers = Pa_GetMinNumBuffers( past->past_FramesPerUserBuffer, past->past_SampleRate );

    // Calculate size of CoreAudio buffer.
    pahsc->framesPerHostBuffer = past->past_FramesPerUserBuffer * past->past_NumUserBuffers;

    // calculate buffer sizes in bytes
    pahsc->input.bytesPerUserNativeBuffer = past->past_FramesPerUserBuffer *
        Pa_GetSampleSize(paFloat32) * past->past_NumInputChannels;
    pahsc->output.bytesPerUserNativeBuffer = past->past_FramesPerUserBuffer *
        Pa_GetSampleSize(paFloat32) * past->past_NumOutputChannels;

    DBUG(("PaOSX_CalcNumHostBuffers: past_NumUserBuffers = %ld\n", past->past_NumUserBuffers ));
    DBUG(("PaOSX_CalcNumHostBuffers: framesPerHostBuffer = %d\n", pahsc->framesPerHostBuffer ));
    DBUG(("PaOSX_CalcNumHostBuffers: input.bytesPerUserNativeBuffer = %d\n", pahsc->input.bytesPerUserNativeBuffer ));
    DBUG(("PaOSX_CalcNumHostBuffers: output.bytesPerUserNativeBuffer = %d\n", pahsc->output.bytesPerUserNativeBuffer ));
}

/*****************************************************************************/
/************** Internal Host API ********************************************/
/*****************************************************************************/
PaError PaHost_OpenStream( internalPortAudioStream   *past )
{
    PaError             result = paNoError;
    PaHostSoundControl *pahsc;
    Boolean             useInput;  
    Boolean             useOutput;          

    /* Allocate and initialize host data. */
    pahsc = (PaHostSoundControl *) malloc(sizeof(PaHostSoundControl));
    if( pahsc == NULL )
    {
        result = paInsufficientMemory;
        goto error;
    }
    memset( pahsc, 0, sizeof(PaHostSoundControl) );
    past->past_DeviceData = (void *) pahsc;
    pahsc->primaryDeviceID = kAudioDeviceUnknown;
    pahsc->input.audioDeviceID = kAudioDeviceUnknown;
    pahsc->output.audioDeviceID = kAudioDeviceUnknown;
    
    PaOSX_CalcHostBufferSize( past );
    
    /* Setup constants for CPU load measurement. */
    pahsc->inverseMicrosPerHostBuffer = past->past_SampleRate / (1000000.0 * 	pahsc->framesPerHostBuffer);

    useOutput = (past->past_OutputDeviceID != paNoDevice) && (past->past_NumOutputChannels > 0);
    useInput = (past->past_InputDeviceID != paNoDevice) && (past->past_NumInputChannels > 0);
    
    // Set device IDs
    if( useOutput )
    {
        pahsc->output.audioDeviceID = sDeviceInfos[past->past_OutputDeviceID].audioDeviceID;
        pahsc->primaryDeviceID = pahsc->output.audioDeviceID;
        if( useInput )
        {
            pahsc->input.audioDeviceID = sDeviceInfos[past->past_InputDeviceID].audioDeviceID;
            if( pahsc->input.audioDeviceID != pahsc->primaryDeviceID )
            {
                pahsc->usingSecondDevice = TRUE; // Use two separate devices!
            }
        }
    }
    else
    {
        /* Just input, not output. */
        pahsc->input.audioDeviceID = sDeviceInfos[past->past_InputDeviceID].audioDeviceID;
        pahsc->primaryDeviceID = pahsc->input.audioDeviceID;
    }
	DBUG(("outputDeviceID = %ld\n", pahsc->output.audioDeviceID ));
	DBUG(("inputDeviceID = %ld\n", pahsc->input.audioDeviceID ));
	DBUG(("primaryDeviceID = %ld\n", pahsc->primaryDeviceID ));
    
    /* ------------------ OUTPUT */
    if( useOutput )
    {
        result = PaOSX_OpenOutputDevice( past );
        if( result < 0 ) goto error;
    }

    /* ------------------ INPUT */
    if( useInput )
    {
        result = PaOSX_OpenInputDevice( past );
        if( result < 0 ) goto error;
    }

    return result;

error:
    PaHost_CloseStream( past );
    return result;
}

/*************************************************************************/
PaError PaHost_StartOutput( internalPortAudioStream *past )
{
    return 0;
}

/*************************************************************************/
PaError PaHost_StartInput( internalPortAudioStream *past )
{
    return 0;
}

/*************************************************************************/
PaError PaHost_StartEngine( internalPortAudioStream *past )
{
    OSStatus            err = noErr;
    PaError             result = paNoError;
    PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;

    past->past_StopSoon = 0;
    past->past_StopNow = 0;
    past->past_IsActive = 1;
    
/* If full duplex and using two separate devices then start input device. */
    if( pahsc->usingSecondDevice )
    {
        // Associate an IO proc with the device and pass a pointer to the audio data context
        err = AudioDeviceAddIOProc(pahsc->input.audioDeviceID, (AudioDeviceIOProc)PaOSX_CoreAudioInputCallback, past);
        if (err != noErr)
        {            
            PRINT_ERR("PaHost_StartEngine: AudioDeviceAddIOProc secondary failed", err );
            goto error;
        }
        
        // start playing sound through the device
        err = AudioDeviceStart(pahsc->input.audioDeviceID, (AudioDeviceIOProc)PaOSX_CoreAudioInputCallback);
        if (err != noErr)
        {            
            PRINT_ERR("PaHost_StartEngine: AudioDeviceStart secondary failed", err );
            PRINT(("The program may succeed if you run it again!\n"));
            goto error;
        }
    }
        
    // Associate an IO proc with the device and pass a pointer to the audio data context
    err = AudioDeviceAddIOProc(pahsc->primaryDeviceID, (AudioDeviceIOProc)PaOSX_CoreAudioIOCallback, past);
    if (err != noErr)
    {            
        PRINT_ERR("PaHost_StartEngine: AudioDeviceAddIOProc primary failed", err );
        goto error;
    }

    // start playing sound through the device
    err = AudioDeviceStart(pahsc->primaryDeviceID, (AudioDeviceIOProc)PaOSX_CoreAudioIOCallback);
    if (err != noErr)
    {            
        PRINT_ERR("PaHost_StartEngine: AudioDeviceStart primary failed", err );
        PRINT(("The program may succeed if you run it again!\n"));
        goto error;
    }
    
    return result;

error:
    sSavedHostError = err;
    return paHostError;
}

/*************************************************************************/
PaError PaHost_StopEngine( internalPortAudioStream *past, int abort )
{
    OSStatus  err = noErr;
    PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
    if( pahsc == NULL ) return paNoError;
    (void) abort;

    /* Tell background thread to stop generating more data and to let current data play out. */
    past->past_StopSoon = 1;
    /* If aborting, tell background thread to stop NOW! */
    if( abort ) past->past_StopNow = 1;
    past->past_IsActive = 0;

#if PA_TRACE_START_STOP
    AddTraceMessage( "PaHost_StopOutput: pahsc_HWaveOut ", (int) pahsc->pahsc_HWaveOut );
#endif

    // FIXME - we should ask proc to stop instead of stopping abruptly
    err = AudioDeviceStop(pahsc->primaryDeviceID, (AudioDeviceIOProc)PaOSX_CoreAudioIOCallback);
    if (err != noErr)
    {
        goto error;
    }

    err = AudioDeviceRemoveIOProc(pahsc->primaryDeviceID, (AudioDeviceIOProc)PaOSX_CoreAudioIOCallback);
    if (err != noErr) goto error;
    
/* If full duplex and using two separate devices then start input device. */
    if( pahsc->usingSecondDevice )
    {
        err = AudioDeviceStop(pahsc->input.audioDeviceID, (AudioDeviceIOProc)PaOSX_CoreAudioInputCallback);
        if (err != noErr) goto error;
        err = AudioDeviceRemoveIOProc(pahsc->input.audioDeviceID, (AudioDeviceIOProc)PaOSX_CoreAudioInputCallback);
        if (err != noErr) goto error;
    }

    return paNoError;

error:
    sSavedHostError = err;
    return paHostError;
}

/*************************************************************************/
PaError PaHost_StopInput( internalPortAudioStream *past, int abort )
{
    return paNoError;
}

/*************************************************************************/
PaError PaHost_StopOutput( internalPortAudioStream *past, int abort )
{
    return paNoError;
}

/*******************************************************************/
PaError PaHost_CloseStream( internalPortAudioStream   *past )
{
    PaHostSoundControl *pahsc;

    if( past == NULL ) return paBadStreamPtr;
    pahsc = (PaHostSoundControl *) past->past_DeviceData;
    if( pahsc == NULL ) return paNoError;
        
    //PaOSX_DumpDeviceInfo( sDeviceInfos[past->past_OutputDeviceID].audioDeviceID, IS_OUTPUT );

#if PA_TRACE_START_STOP
    AddTraceMessage( "PaHost_CloseStream: pahsc_HWaveOut ", (int) pahsc->pahsc_HWaveOut );
#endif

    if( pahsc->output.converterBuffer != NULL )
    {
        PaHost_FreeFastMemory( pahsc->output.converterBuffer, pahsc->output.bytesPerUserNativeBuffer );
    }
    if( pahsc->input.converterBuffer != NULL )
    {
        PaHost_FreeFastMemory( pahsc->input.converterBuffer, pahsc->input.bytesPerUserNativeBuffer );
    }
    if( pahsc->ringBufferData != NULL )
    {
        PaHost_FreeFastMemory( pahsc->ringBufferData, pahsc->ringBuffer.bufferSize );
    }
    if( pahsc->output.converter != NULL )
    {
        verify_noerr(AudioConverterDispose (pahsc->output.converter));
    }
    if( pahsc->input.converter != NULL )
    {
        verify_noerr(AudioConverterDispose (pahsc->input.converter));
    }
    
    free( pahsc );
    past->past_DeviceData = NULL;

    return paNoError;
}

/**********************************************************************
** Initialize Host dependant part of API.
*/
PaError PaHost_Init( void )
{
    return PaOSX_MaybeQueryDevices();
}

/*************************************************************************
** Cleanup device info.
*/
PaError PaHost_Term( void )
{
    int i;

    if( sDeviceInfos != NULL )
    {
        for( i=0; i<sNumPaDevices; i++ )
        {
            if( sDeviceInfos[i].paInfo.name != NULL )
            {
                free( (char*)sDeviceInfos[i].paInfo.name );
            }
        }
        free( sDeviceInfos );
        sDeviceInfos = NULL;
    }

    sNumPaDevices = 0;
    return paNoError;
}

/*************************************************************************
 * Allocate memory that can be accessed in real-time.
 * This may need to be held in physical memory so that it is not
 * paged to virtual memory.
 * This call MUST be balanced with a call to PaHost_FreeFastMemory().
 */
void *PaHost_AllocateFastMemory( long numBytes )
{
    void *addr = malloc( numBytes ); /* FIXME - do we need physical memory, not virtual memory? */
    if( addr != NULL ) memset( addr, 0, numBytes );
    return addr;
}

/*************************************************************************
 * Free memory that could be accessed in real-time.
 * This call MUST be balanced with a call to PaHost_AllocateFastMemory().
 */
void PaHost_FreeFastMemory( void *addr, long numBytes )
{
    if( addr != NULL ) free( addr );
}


/***********************************************************************/
PaError PaHost_StreamActive( internalPortAudioStream   *past )
{
    PaHostSoundControl *pahsc;
    if( past == NULL ) return paBadStreamPtr;
    pahsc = (PaHostSoundControl *) past->past_DeviceData;
    if( pahsc == NULL ) return paInternalError;
    return (PaError) past->past_IsActive;
}

/*******************************************************************/
PaError PaHost_GetTotalBufferFrames( internalPortAudioStream   *past )
{
    PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
    return pahsc->framesPerHostBuffer;
}

/*****************************************************************************/
/************** External User API ********************************************/
/*****************************************************************************/

/**********************************************************************
** Query devices and use result.
*/
PaDeviceID Pa_GetDefaultInputDeviceID( void )
{
    PaError result = PaOSX_MaybeQueryDevices();
	if( result < 0 ) return result;
	return sDefaultInputDeviceID;
}

PaDeviceID Pa_GetDefaultOutputDeviceID( void )
{
    PaError result = PaOSX_MaybeQueryDevices();
	if( result < 0 ) return result;
	return sDefaultOutputDeviceID;
}


/*************************************************************************
** Determine minimum number of buffers required for this host based
** on minimum latency. Because CoreAudio manages latency, this just sets
** a reasonable small buffer size.
*/
int Pa_GetMinNumBuffers( int framesPerBuffer, double framesPerSecond )
{
    int minBuffers;
    double denominator;
    int minLatencyMsec = PA_MIN_LATENCY_MSEC;
    denominator =  1000.0 * framesPerBuffer;
    minBuffers = (int) (((minLatencyMsec * framesPerSecond) + denominator - 1) / denominator );
    if( minBuffers < 1 ) minBuffers = 1;
    return minBuffers;
}

/*************************************************************************/
void Pa_Sleep( long msec )
{
    usleep( msec * 1000 );
}

/*************************************************************************/
PaTimestamp Pa_StreamTime( PortAudioStream *stream )
{
    AudioTimeStamp timeStamp;
    PaTimestamp streamTime;
    PaHostSoundControl *pahsc;
    internalPortAudioStream   *past = (internalPortAudioStream *) stream;
    if( past == NULL ) return paBadStreamPtr;
    pahsc = (PaHostSoundControl *) past->past_DeviceData;
  
    AudioDeviceGetCurrentTime(pahsc->primaryDeviceID, &timeStamp);
  
    streamTime = ( timeStamp.mFlags & kAudioTimeStampSampleTimeValid) ?
            timeStamp.mSampleTime : past->past_FrameCount;

    return streamTime;
}

/************************************************************************************/
long Pa_GetHostError()
{
    return sSavedHostError;
}

/*************************************************************************/
int Pa_CountDevices()
{
    if( sNumPaDevices <= 0 ) Pa_Initialize();
    return sNumPaDevices;
}

/*************************************************************************
** PaDeviceInfo structures have already been created
** so just return the pointer.
**
*/
const PaDeviceInfo* Pa_GetDeviceInfo( PaDeviceID id )
{
    if( id < 0 || id >= sNumPaDevices )
        return NULL;

    return &sDeviceInfos[id].paInfo;
}

