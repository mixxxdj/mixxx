/*
 * $Id: pa_mac_core.c 329 2003-05-07 12:22:30Z tuehaste $
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
 * To provide other rates we use the handy Apple AudioConverter which provides
 * sample rate conversion, mono-to-stereo conversion, and buffer size adaptation.
 *
 * There are four modes of operation:
 *    PA_MODE_OUTPUT_ONLY,
 *    PA_MODE_INPUT_ONLY,
 *    PA_MODE_IO_ONE_DEVICE,
 *    PA_MODE_IO_TWO_DEVICES
 *    
 * The processing pipeline for PA_MODE_IO_ONE_DEVICE is in one thread:
 *
 * PaOSX_CoreAudioIOCallback() input buffers -> RingBuffer -> input.AudioConverter ->
 *    PortAudio callback -> output.AudioConverter -> PaOSX_CoreAudioIOCallback() output buffers
 *
 * For two separate devices, we have to use two separate callbacks.
 * We pass data between them using a RingBuffer FIFO.
 * The processing pipeline for PA_MODE_IO_TWO_DEVICES is split into two threads:
 *
 * PaOSX_CoreAudioInputCallback() input buffers -> RingBuffer
 *
 * RingBuffer -> input.AudioConverter ->
 *    PortAudio callback -> output.AudioConverter -> PaOSX_CoreAudioIOCallback() output buffers
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
 02.03.2003 - Phil Burk - always use AudioConverters so that we can adapt when format changes.
              Synchronize with device when format changes.
 02.13.2003 - Phil Burk - scan for maxChannels because FormatMatch won't tell us.
 03.05.2003 - Phil Burk and Dominic Mazzoni - interleave and deinterleave multiple
              CoreAudio buffers. Needed for MOTU828 and some other N>2 channel devices.
              See code related to "streamInterleavingBuffer".
 03.06.2003 - Phil Burk and Ryan Francesconi - fixed numChannels query for MOTU828.
              Handle fact that MOTU828 gives you 8 channels even when you ask for 2!
 04.06.2003 - Phil Burk - Combine Dominic Mazzoni's technique of using Configuration to query maxChannels
              with old technique of scanning for mormat.
              Increase channel scan by 1 to handle mono USB microphones.
              Do not merge or split channels in AudioConverter to handle 2+2 channels
              of Quattro which has a format of 2 channels.
 04.07.2003 - Phil Burk - use AudioGetCurrentHostTime instead of getrusage() which can lock threads.
 04.10.2003 - Phil Burk - fixed pointer bug with input deinterleaving loop.
              Detect and ignore NULL inputData and outputData in CodeAudio callback.
              Overlap creation and deletion of AudioConverters to prevent thread death when device rate changes.
 04.16.2003 - Phil Burk - Fixed input channel scrambling when numChannels != 2^N. Caused by alignment
              error when filling RingBuffer with 2^N zero bytes.
*/

#include <CoreServices/CoreServices.h>
#include <CoreAudio/CoreAudio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/DefaultAudioOutput.h>
#include <AudioToolbox/AudioConverter.h>
#include <CoreAudio/HostTime.h>

#include "portaudio.h"
#include "pa_host.h"
#include "pa_trace.h"
#include "ringbuffer.h"

/************************************************* Configuration ********/
#define PA_ENABLE_LOAD_MEASUREMENT  (1)

/************************************************* Constants ********/
#define SET_DEVICE_BUFFER_SIZE   (1)

/* To trace program, enable TRACE_REALTIME_EVENTS in pa_trace.h */
#define PA_TRACE_RUN             (0)
#define PA_TRACE_START_STOP      (0)

#define PA_MIN_LATENCY_MSEC      (20) /* FIXME */
#define MIN_TIMEOUT_MSEC         (3000)

#define PRINT(x) { printf x; fflush(stdout); }
#define PRINT_ERR( msg, err ) PRINT(( msg ": error = 0x%0lX = '%s'\n", (err), ErrorToString(err)) )
#define DBUG(x)    /* PRINT(x) */
#define DBUGBACK(x) /* if( sMaxBackgroundErrorMessages-- > 0 ) PRINT(x) */
#define DBUGX(x)

// define value of isInput passed to CoreAudio routines
#define IS_INPUT    (true)
#define IS_OUTPUT   (false)

typedef enum PaDeviceMode
{
    PA_MODE_OUTPUT_ONLY,
    PA_MODE_INPUT_ONLY,
    PA_MODE_IO_ONE_DEVICE,
    PA_MODE_IO_TWO_DEVICES
} PaDeviceMode;

#define PA_USING_OUTPUT   (pahsc->mode != PA_MODE_INPUT_ONLY)
#define PA_USING_INPUT    (pahsc->mode != PA_MODE_OUTPUT_ONLY)

/**************************************************************
 * Information needed by PortAudio specific to a CoreAudio device.
 */
typedef struct PaHostInOut_s
{
    AudioDeviceID      audioDeviceID; /* CoreAudio specific ID */
    int                bytesPerUserNativeBuffer; /* User buffer size in native host format. Depends on numChannels. */
    AudioConverterRef  converter;
    void              *converterBuffer;
    int                numChannels;
    /** Used for interleaving or de-interleaving multiple streams for devices like MOTU828. */
    int                streamInterleavingBufferLen; /**< size in bytes */
    Float32           *streamInterleavingBuffer; 
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
    PaDeviceMode       mode;
    RingBuffer         ringBuffer;
    char              *ringBufferData;
    Boolean            formatListenerCalled;
    /* For measuring CPU utilization. */
    UInt64             entryTime;
    double             inverseHostTicksPerBuffer; /* 1/Ticks of real-time audio per user buffer. */
} PaHostSoundControl;

/**************************************************************
 * Structure for internal extended device info query.
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
static int sNumCoreDevices = 0;
static AudioDeviceID *sCoreDeviceIDs;   // Array of Core AudioDeviceIDs
static PaHostDeviceInfo *sDeviceInfos = NULL;
static int sDefaultInputDeviceID = paNoDevice;
static int sDefaultOutputDeviceID = paNoDevice;
static int sSavedHostError = 0;

static const double supportedSampleRateRange[] = { 8000.0, 96000.0 }; /* FIXME - go to double HW rate. */
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

static OSStatus PAOSX_DevicePropertyListener (AudioDeviceID					inDevice,
								UInt32							inChannel,
								Boolean							isInput,
								AudioDevicePropertyID			inPropertyID,
								void*							inClientData);
                                
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
    pahsc->entryTime = AudioGetCurrentHostTime();
}

/******************************************************************************
** Measure fractional CPU load based on real-time it took to calculate
** buffers worth of output.
*/
static void Pa_EndUsageCalculation( internalPortAudioStream   *past )
{
    UInt64   exitTime;
    UInt64   ticksElapsed;
    double   newUsage;
    
#define LOWPASS_COEFFICIENT_0   (0.95)
#define LOWPASS_COEFFICIENT_1   (0.99999 - LOWPASS_COEFFICIENT_0)

    PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
    if( pahsc == NULL ) return;
    
    exitTime = AudioGetCurrentHostTime();
    
    ticksElapsed = exitTime - pahsc->entryTime;

    /* Use inverse because it is faster than the divide. */
	newUsage =  ticksElapsed * pahsc->inverseHostTicksPerBuffer;
	/* Low pass filter result. */
    past->past_Usage = (LOWPASS_COEFFICIENT_0 * past->past_Usage) +
                           (LOWPASS_COEFFICIENT_1 * newUsage);
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
    DBUG(("PaOSX_QueryDevices: scan for input ======================\n"));
    PaOSX_ScanDevices( IS_INPUT );
    sNumInputDevices = sNumPaDevices;
    // Now scan all the output devices.
    DBUG(("PaOSX_QueryDevices: scan for output ======================\n"));
    PaOSX_ScanDevices( IS_OUTPUT );
    sNumOutputDevices = sNumPaDevices - sNumInputDevices;

    // Figure out which of the devices that we scanned is the default device.
    sDefaultInputDeviceID = PaOSX_QueryDefaultInputDevice();
    sDefaultOutputDeviceID = PaOSX_QueryDefaultOutputDevice();

    return paNoError;
}


/*************************************************************************/
/* Query a device for its sample rate.
 * @return positive rate or 0.0 on error.
 */
static Float64 PaOSX_GetDeviceSampleRate( AudioDeviceID deviceID, Boolean isInput )
{
    OSStatus  err = noErr;
    AudioStreamBasicDescription formatDesc;
    UInt32    dataSize;
    dataSize = sizeof(formatDesc);
    err = AudioDeviceGetProperty( deviceID, 0, isInput,
        kAudioDevicePropertyStreamFormat, &dataSize, &formatDesc);
    if( err != noErr ) return 0.0;
    else return formatDesc.mSampleRate;
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
** Scan all of the Core Audio devices to see which support selected
** input or output mode.
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
        DBUG(("PaOSX_ScanDevices: paDevId = %d, coreDevId = %d, result = %d\n", sNumPaDevices, coreDeviceIndex, result ));
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
** Determine the maximum number of channels based on the configuration.
** @return maxChannels or negative error.
*/
static int PaOSX_GetMaxChannels_Config( AudioDeviceID devID, Boolean isInput )
{
    OSStatus         err;
    UInt32           outSize;
    Boolean          outWritable;
    AudioBufferList *list;
    int              numChannels;
    int              i;

    // Determine maximum number of channels supported.
    // dmazzoni: new method

    outSize = 0;
    err = AudioDeviceGetPropertyInfo(devID, 0, isInput,
                                     kAudioDevicePropertyStreamConfiguration,
                                     &outSize, &outWritable);
    if ( err != noErr )
    {
        PRINT_ERR("PaOSX_GetMaxChannels_Config: Could not get stream configuration info", err);
        sSavedHostError = err;
        return paHostError;
    }

    list = (AudioBufferList *)PaHost_AllocateFastMemory( outSize );
    err = AudioDeviceGetProperty(devID, 0, isInput,
                                 kAudioDevicePropertyStreamConfiguration,
                                 &outSize, list);
    if ( err != noErr )
    {
        PRINT_ERR("PaOSX_GetMaxChannels_Config: Could not get stream configuration", err);
        sSavedHostError = err;
        return paHostError;
    }

    numChannels = 0;
    for( i=0; i<list->mNumberBuffers; i++ )
    {
        int bufChannels = list->mBuffers[i].mNumberChannels;
        DBUG(("PaOSX_GetMaxChannels_Config: buffer %d has %d channels.\n", i, bufChannels ));
        numChannels += bufChannels;
    }
    
    PaHost_FreeFastMemory( list, outSize );

    return numChannels;
}

/*************************************************************************
** Determine the maximum number of channels a device will support based on scanning the format.
** @return maxChannels or negative error.
*/
static int PaOSX_GetMaxChannels_Format( AudioDeviceID devID, Boolean isInput )
{
    OSStatus         err;
    UInt32           outSize;
    AudioStreamBasicDescription formatDesc;
    int              maxChannels;
    int              numChannels;
    Boolean          gotMax;

	// Scan to find highest matching format.
    // Unfortunately some devices won't just return maxChannels for the match.
    // For example, some 8 channel devices return 2 when given 256 as input.
    gotMax = false;
    maxChannels = 0;
    numChannels = 0;
    while( !gotMax )
    {
    
        memset( &formatDesc, 0, sizeof(formatDesc));
        numChannels = numChannels + 1;
        DBUG(("PaOSX_GetMaxChannels: try numChannels = %d = %d + 1\n",
            numChannels, numChannels ));
        formatDesc.mChannelsPerFrame = numChannels;
        outSize = sizeof(formatDesc);

        err = AudioDeviceGetProperty( devID, 0,
            isInput, kAudioDevicePropertyStreamFormatMatch, &outSize, &formatDesc);

        DBUG(("PaOSX_GetMaxChannels: err 0x%0x, formatDesc.mChannelsPerFrame= %d\n",
            err, formatDesc.mChannelsPerFrame ));
        if( err != noErr )
        {
            if (numChannels > (maxChannels + 4)) // Try several possibilities above current max
			{
                gotMax = true;
            }
        }
        else
        {
            // This value worked so we have a new candidate for maxChannels.
            if (formatDesc.mChannelsPerFrame > numChannels)
            {
            	maxChannels =  formatDesc.mChannelsPerFrame;
            }
            else if(formatDesc.mChannelsPerFrame < numChannels)
            {
                if (numChannels > (maxChannels + 4)) // Try several possibilities above current max
                {
                    gotMax = true;
                }
            }
            else
            {
            	maxChannels = numChannels;            	
            }
        }
    }
    return maxChannels;
}



/*************************************************************************
** Determine the maximum number of channels a device will support.
** It is not clear at this point which the better technique so
** we do both and use the biggest result.
**
** @return maxChannels or negative error.
*/
static int PaOSX_GetMaxChannels( AudioDeviceID devID, Boolean isInput )
{
    int maxChannelsFormat;
    int maxChannelsConfig;
    maxChannelsFormat = PaOSX_GetMaxChannels_Format( devID, isInput );
    maxChannelsConfig = PaOSX_GetMaxChannels_Config( devID, isInput );
    return (maxChannelsFormat > maxChannelsConfig) ? maxChannelsFormat : maxChannelsConfig;
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
    int              maxChannels;

    deviceInfo->structVersion = 1;
    deviceInfo->maxInputChannels = 0;
    deviceInfo->maxOutputChannels = 0;

    deviceInfo->sampleRates = supportedSampleRateRange; // because we use sample rate converter to get continuous rates
    deviceInfo->numSampleRates = -1;

    devID = sCoreDeviceIDs[ coreDeviceIndex ];
    hostDeviceInfo->audioDeviceID = devID;
    DBUG(("PaOSX_QueryDeviceInfo: coreDeviceIndex = %d, devID = %d, isInput = %d\n",
        coreDeviceIndex, (int) devID, isInput ));
        
    // Get data format info from the device.
    outSize = sizeof(formatDesc);
    err = AudioDeviceGetProperty(devID, 0, isInput, kAudioDevicePropertyStreamFormat, &outSize, &formatDesc);
    // This just may not be an appropriate device for input or output so leave quietly.
    if( (err != noErr)  || (formatDesc.mChannelsPerFrame == 0) ) goto error;
    
    DBUG(("PaOSX_QueryDeviceInfo: mFormatID = 0x%x\n", (unsigned int) formatDesc.mFormatID));
    DBUG(("PaOSX_QueryDeviceInfo: mFormatFlags = 0x%x\n",(unsigned int)  formatDesc.mFormatFlags));

    // Right now the Core Audio headers only define one formatID: LinearPCM
    // Apparently LinearPCM must be Float32 for now.
    if( (formatDesc.mFormatID == kAudioFormatLinearPCM) &&
        ((formatDesc.mFormatFlags & kLinearPCMFormatFlagIsFloat) != 0) )
    {
        deviceInfo->nativeSampleFormats = paFloat32;
    }
    else
    {
        PRINT(("PaOSX_QueryDeviceInfo: ERROR - not LinearPCM & Float32!!!\n"));
        return paSampleFormatNotSupported;
    }
    
    maxChannels = PaOSX_GetMaxChannels( devID, isInput );
    if( maxChannels <= 0 ) goto error;
    if( isInput )
    {
        deviceInfo->maxInputChannels = maxChannels;
    }
    else
    {
        deviceInfo->maxOutputChannels = maxChannels;
    }
    
    // Get the device name
    deviceInfo->name = PaOSX_DeviceNameFromID( devID, isInput );
    DBUG(("PaOSX_QueryDeviceInfo: name = %s\n", deviceInfo->name ));
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
        *outData = zeroPad; /* Give it zero data to keep it happy. */
        *outDataSize = sizeof(zeroPad);
    }
	return noErr;
}

/*****************************************************************************
** Get audio input, if any, from passed in buffer, or from converter or from FIFO,
** then run PA callback and output data.
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
            AddTraceMessage("PaOSX_LoadAndProcess: zero rest of wave buffer ", i );
            memset( outputBuffer, 0, pahsc->output.bytesPerUserNativeBuffer );
        }
    }
    else
    {
        /* Do we need data from the converted input? */
        if( PA_USING_INPUT )
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
        
        /* Measure CPU load. */
#if PA_ENABLE_LOAD_MEASUREMENT
        Pa_StartUsageCalculation( past );
#endif

        /* Fill part of audio converter buffer by converting input to user format,
        * calling user callback, then converting output to native format. */
        if( PaConvert_Process( past, inputBuffer, outputBuffer ))
        {
            past->past_StopSoon = 1;
        }
        
#if PA_ENABLE_LOAD_MEASUREMENT
        Pa_EndUsageCalculation( past );
#endif

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
** If data available, write it to the Ring Buffer so we can
** pull it from the other side.
*/
static OSStatus PaOSX_WriteInputRingBuffer( internalPortAudioStream   *past,
        const AudioBufferList*  inInputData  )
{
    int   numBytes = 0;
    int   currentInterleavedChannelIndex;
    int   numFramesInInputBuffer;
    int   numInterleavedChannels;
    int   numChannelsRemaining;
	int   i;
    long  writeRoom;
    char *inputNativeBufferfPtr = NULL;
    PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;

    /* Do we need to deinterleave the buffers first? */
    if( past->past_NumInputChannels != inInputData->mBuffers[0].mNumberChannels )
    {
		numFramesInInputBuffer = inInputData->mBuffers[0].mDataByteSize /
            (sizeof(float) * inInputData->mBuffers[0].mNumberChannels);
		
		numBytes = numFramesInInputBuffer * sizeof(float) * past->past_NumInputChannels;
	
		/* Allocate temporary buffer if needed. */
		if ( (pahsc->input.streamInterleavingBuffer != NULL) &&
			 (pahsc->input.streamInterleavingBufferLen < numBytes) )
		{
			PaHost_FreeFastMemory( pahsc->input.streamInterleavingBuffer, pahsc->input.streamInterleavingBufferLen );
			pahsc->input.streamInterleavingBuffer = NULL;
		}
		if ( pahsc->input.streamInterleavingBuffer == NULL )
		{
			pahsc->input.streamInterleavingBufferLen = numBytes;
			pahsc->input.streamInterleavingBuffer = (float *)PaHost_AllocateFastMemory( pahsc->input.streamInterleavingBufferLen );
		}
	
		/* Perform interleaving by writing to temp buffer. */
		currentInterleavedChannelIndex = 0;
		numInterleavedChannels = past->past_NumInputChannels;
		numChannelsRemaining = numInterleavedChannels;
		
		for( i=0; i<inInputData->mNumberBuffers; i++ )
		{
			int j;
			int numBufChannels = inInputData->mBuffers[i].mNumberChannels;
			/* Don't use more than we need or more than we have. */
			int numChannelsUsedInThisBuffer = (numChannelsRemaining < numBufChannels ) ?
				  numChannelsRemaining : numBufChannels;
			for( j=0; j<numChannelsUsedInThisBuffer; j++ )
			{
				int k;
                float *dest = &pahsc->input.streamInterleavingBuffer[ currentInterleavedChannelIndex ];
                float *src = &((float *)inInputData->mBuffers[i].mData)[ j ];
				/* Move one channel from CoreAudio buffer to interleaved buffer. */
				for( k=0; k<numFramesInInputBuffer; k++ )
				{
					*dest = *src;
                    src += numBufChannels;
                    dest += numInterleavedChannels;
				}
				currentInterleavedChannelIndex++;
			}
			numChannelsRemaining -= numChannelsUsedInThisBuffer;
			if( numChannelsRemaining <= 0 ) break;
		}

        inputNativeBufferfPtr = (char *)pahsc->input.streamInterleavingBuffer;
    }
    else
    {
        inputNativeBufferfPtr = (char*)inInputData->mBuffers[0].mData;
        numBytes = inInputData->mBuffers[0].mDataByteSize;
    }

    writeRoom = RingBuffer_GetWriteAvailable( &pahsc->ringBuffer );
    
    if( numBytes <= writeRoom )
    {
        RingBuffer_Write(  &pahsc->ringBuffer, inputNativeBufferfPtr, numBytes );
        DBUGBACK(("PaOSX_WriteInputRingBuffer: wrote %ld bytes to FIFO.\n", numBytes));
    } // FIXME else drop samples on floor, remember overflow???            

    return noErr;
}

/**********************************************************************
** Use any available input buffers by writing to RingBuffer.
** Process input if PA_MODE_INPUT_ONLY.
*/
static OSStatus PaOSX_HandleInput( internalPortAudioStream   *past,
        const AudioBufferList*  inInputData )
{
    OSStatus            err = noErr;
    PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;

    if( inInputData == NULL )
    {
        DBUG(("PaOSX_HandleInput: inInputData == NULL\n"));
        return noErr;
    }
    
    if(  inInputData->mNumberBuffers > 0  )
    {    
        /* Write to FIFO here if we are only using this callback. */
        if( (pahsc->mode == PA_MODE_INPUT_ONLY) || (pahsc->mode == PA_MODE_IO_ONE_DEVICE) )
        {
            err = PaOSX_WriteInputRingBuffer( past, inInputData  );
            if( err != noErr ) goto error;
        }
    }
    
    if( pahsc->mode == PA_MODE_INPUT_ONLY )
    {
        /* Generate user buffers as long as we have a half full input FIFO. */
        long halfSize = pahsc->ringBuffer.bufferSize / 2;
        while( (RingBuffer_GetReadAvailable( &pahsc->ringBuffer ) >= halfSize) &&
            (past->past_StopSoon == 0) )
        {
            err = PaOSX_LoadAndProcess ( past, NULL, NULL );
            if( err != noErr ) goto error;
        }
    }
        
error:
    return err;
}

/**********************************************************************
** Fill any available output buffers.
*/
static OSStatus PaOSX_HandleOutput( internalPortAudioStream   *past,
            AudioBufferList*  outOutputData )
{
    OSStatus            err = noErr;
    void               *outputNativeBufferfPtr = NULL;
    PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
    UInt32              numBytes = 0;
    int                 numChannelsRemaining;
    Boolean             deinterleavingNeeded;
    int                 numFramesInOutputBuffer;
    
    if( outOutputData == NULL )
    {
        DBUG(("PaOSX_HandleOutput: outOutputData == NULL\n"));
        return noErr;
    }

    deinterleavingNeeded = past->past_NumOutputChannels != outOutputData->mBuffers[0].mNumberChannels;
    
	numFramesInOutputBuffer = outOutputData->mBuffers[0].mDataByteSize /
        (sizeof(float) * outOutputData->mBuffers[0].mNumberChannels);
    
    if( pahsc->mode != PA_MODE_INPUT_ONLY )
    {
        /* If we are using output, then we need an empty output buffer. */
        if( outOutputData->mNumberBuffers > 0 )
        {
        	
            /* If we have multiple CoreAudio buffers, then we will need to deinterleave after conversion. */
            if( deinterleavingNeeded )
            {
            	numBytes = numFramesInOutputBuffer * sizeof(float) * past->past_NumOutputChannels;
                
                /* Free old buffer if we are allocating new one. */
                if ( (pahsc->output.streamInterleavingBuffer != NULL) &&
                     (pahsc->output.streamInterleavingBufferLen < numBytes) )
                {
                    PaHost_FreeFastMemory( pahsc->output.streamInterleavingBuffer, pahsc->output.streamInterleavingBufferLen );
                    pahsc->output.streamInterleavingBuffer = NULL;
                }
                /* Allocate interleaving buffer if needed. */
                if ( pahsc->output.streamInterleavingBuffer == NULL )
                {
                    pahsc->output.streamInterleavingBufferLen = numBytes;
                    pahsc->output.streamInterleavingBuffer = (float *)PaHost_AllocateFastMemory( pahsc->output.streamInterleavingBufferLen );
                }

                outputNativeBufferfPtr =  (void*)pahsc->output.streamInterleavingBuffer;
            }
            else
            {
                numBytes = outOutputData->mBuffers[0].mDataByteSize;
                outputNativeBufferfPtr =  (void*)outOutputData->mBuffers[0].mData;
            }
    
            /* Pull data from PA user through converter. */
            err = AudioConverterFillBuffer(
                pahsc->output.converter,
                PaOSX_OutputConverterCallbackProc,
                past,
                &numBytes,
                outputNativeBufferfPtr);
            if( err != noErr )
            {
                PRINT_ERR("PaOSX_HandleOutput: AudioConverterFillBuffer failed", err);
                goto error;
            }
            
            /* Deinterleave data from PortAudio and write to multiple CoreAudio buffers. */
            if( deinterleavingNeeded )
            {
                int numInterleavedChannels = past->past_NumOutputChannels;
                int i, currentInterleavedChannelIndex = 0;
                numChannelsRemaining = numInterleavedChannels;
                
                for( i=0; i<outOutputData->mNumberBuffers; i++ )
                {
                    int numBufChannels = outOutputData->mBuffers[i].mNumberChannels;
                    int j;            
                    /* Don't use more than we need or more than we have. */
                	int numChannelsUsedInThisBuffer = (numChannelsRemaining < numBufChannels ) ?
                	      numChannelsRemaining : numBufChannels;
                	
                    for( j=0; j<numChannelsUsedInThisBuffer; j++ )
                    {
                        int k;
                        float *dest = &((float *)outOutputData->mBuffers[i].mData)[ j ];
                        float *src = &pahsc->output.streamInterleavingBuffer[ currentInterleavedChannelIndex ];
                        /* Move one channel from interleaved buffer to CoreAudio buffer. */
                        for( k=0; k<numFramesInOutputBuffer; k++ )
                        {
                            *dest = *src;
                            dest += numBufChannels;
                            src += numInterleavedChannels;
                        }
                        currentInterleavedChannelIndex++;
                    }
                    
                    numChannelsRemaining -= numChannelsUsedInThisBuffer;
                    if( numChannelsRemaining <= 0 ) break;
                }
            }
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
    OSStatus      err = noErr;
    internalPortAudioStream *past = (internalPortAudioStream *) contextPtr;
    PaHostSoundControl *pahsc;
    pahsc = (PaHostSoundControl *) past->past_DeviceData;
   
    /* If there is a FIFO for input then write to it. */
    if( (pahsc->ringBufferData != NULL) && (inInputData != NULL) )
    {
        err = PaOSX_WriteInputRingBuffer( past, inInputData  );
        if( err != noErr ) goto error;
    }
error:
    return err;
}

/******************************************************************
 * This is the primary callback for CoreAudio.
 * It can handle input and/or output for a single device.
 * It takes input from CoreAudio, converts it and passes it to the
 * PortAudio user callback. Then takes the PA results and passes it
 * back to CoreAudio.
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
        else if( inInputTime->mFlags & kAudioTimeStampSampleTimeValid) 
        {
            past->past_FrameCount = inInputTime->mSampleTime;
        }
        
        past->past_NumCallbacks += 1;
        
        /* Process full input buffer. */
        err = PaOSX_HandleInput( past, inInputData );
        if( err != 0 ) goto error;
        
        /* Fill up empty output buffers. */
        err = PaOSX_HandleOutput( past, outOutputData );
        if( err != 0 ) goto error;
    }

    if( err != 0 ) DBUG(("PaOSX_CoreAudioIOCallback: returns %ld.\n", err ));
    
error:
    return err;
}

/*******************************************************************/
/** Attempt to set device sample rate.
 * This is not critical because we use an AudioConverter but we may
 * get better fidelity if we can avoid resampling.
 *
 * Only set format once because some devices take time to settle.
 * Return flag indicating whether format changed so we know whether to wait
 * for DevicePropertyListener to get called.
 *
 * @return negative error, zero if no change, or one if changed successfully.
 */
static PaError PaOSX_SetFormat( AudioDeviceID devID, Boolean isInput,
        double desiredRate, int desiredNumChannels )
{
    AudioStreamBasicDescription formatDesc;
    PaError  result = 0;
    OSStatus err;
    UInt32   dataSize;
    Float64  originalRate;
    int      originalChannels;
    
    /* Get current device format. This is critical because if we pass
     * zeros for unspecified fields then the iMic device gets switched to a 16 bit
     * integer format!!! I don't know if this is a Mac bug or not. But it only
     * started happening when I upgraded from OS X V10.1 to V10.2 (Jaguar).
     */
    dataSize = sizeof(formatDesc);
    err = AudioDeviceGetProperty( devID, 0, isInput,
        kAudioDevicePropertyStreamFormat, &dataSize, &formatDesc);
    if( err != noErr )
    {
        PRINT_ERR("PaOSX_SetFormat: Could not get format.", err);
        sSavedHostError = err;
        return paHostError;
    }
    
    originalRate = formatDesc.mSampleRate;
    originalChannels = formatDesc.mChannelsPerFrame;
        
    // Is it already set to the correct format?
    if( (originalRate != desiredRate) || (originalChannels != desiredNumChannels) )
    {
        DBUG(("PaOSX_SetFormat: try to change sample rate to %f.\n", desiredRate ));
        DBUG(("PaOSX_SetFormat: try to set number of channels to %d\n", desiredNumChannels));

        formatDesc.mSampleRate = desiredRate;
        formatDesc.mChannelsPerFrame = desiredNumChannels;
        formatDesc.mBytesPerFrame = formatDesc.mChannelsPerFrame * sizeof(float);
        formatDesc.mBytesPerPacket = formatDesc.mBytesPerFrame * formatDesc.mFramesPerPacket;

        err = AudioDeviceSetProperty( devID, 0, 0,
            isInput, kAudioDevicePropertyStreamFormat, sizeof(formatDesc), &formatDesc);
        if (err != noErr)
        {
            /* Could not set to desired rate so query for closest match. */
            dataSize = sizeof(formatDesc);
            err = AudioDeviceGetProperty( devID, 0,
                isInput, kAudioDevicePropertyStreamFormatMatch, &dataSize, &formatDesc);
                
            DBUG(("PaOSX_SetFormat: closest rate is %f.\n", formatDesc.mSampleRate ));
            DBUG(("PaOSX_SetFormat: closest numChannels is %d.\n", (int)formatDesc.mChannelsPerFrame ));
            // Set to closest if different from original.
            if( (err == noErr) &&
                ((originalRate != formatDesc.mSampleRate) ||
                 (originalChannels != formatDesc.mChannelsPerFrame)) )
            {
                err = AudioDeviceSetProperty( devID, 0, 0,
                    isInput, kAudioDevicePropertyStreamFormat, sizeof(formatDesc), &formatDesc);
                if( err == noErr ) result = 1;
            }
        }
        else result = 1;
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
            DBUG(("mute for channel %d = %ld\n", iChannel, uidata32));
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
static OSStatus PAOSX_DevicePropertyListener (AudioDeviceID					inDevice,
								UInt32							inChannel,
								Boolean							isInput,
								AudioDevicePropertyID			inPropertyID,
								void*							inClientData)
{
    PaHostSoundControl       *pahsc;
    internalPortAudioStream  *past;
    UInt32                    dataSize;
    OSStatus                  err = noErr;
	AudioStreamBasicDescription userStreamFormat, hardwareStreamFormat;
    PaHostInOut              *hostInOut;
	AudioStreamBasicDescription *destFormatPtr, *srcFormatPtr;

    past = (internalPortAudioStream *) inClientData;
    pahsc = (PaHostSoundControl *) past->past_DeviceData;

    DBUG(("PAOSX_DevicePropertyListener: called with propertyID = 0x%0X\n", (unsigned int) inPropertyID ));

    if(inPropertyID == kAudioDevicePropertyStreamFormat)
    {    
        /* Get target device format */
        dataSize = sizeof(hardwareStreamFormat);
        err = AudioDeviceGetProperty(inDevice, 0, isInput,
            kAudioDevicePropertyStreamFormat, &dataSize, &hardwareStreamFormat);
        if( err != noErr )
        {
            PRINT_ERR("PAOSX_DevicePropertyListener: Could not get device format", err);
            sSavedHostError = err;
            goto error;
        }

        DBUG(("PAOSX_DevicePropertyListener: HW mChannelsPerFrame = %d\n", (int)hardwareStreamFormat.mChannelsPerFrame ));
        
        /* Set source user format. */
        userStreamFormat = hardwareStreamFormat;
        userStreamFormat.mSampleRate = past->past_SampleRate;	// sample rate of the user synthesis code
        userStreamFormat.mChannelsPerFrame = (isInput) ? past->past_NumInputChannels : past->past_NumOutputChannels;	//	the number of channels in each frame
        DBUG(("PAOSX_DevicePropertyListener: User mChannelsPerFrame = %d\n", (int)userStreamFormat.mChannelsPerFrame ));
    
        userStreamFormat.mBytesPerFrame = userStreamFormat.mChannelsPerFrame * sizeof(float);
        userStreamFormat.mBytesPerPacket = userStreamFormat.mBytesPerFrame * userStreamFormat.mFramesPerPacket;
    
    	/* Don't use AudioConverter for merging or splitting channels. */
        hardwareStreamFormat.mChannelsPerFrame = userStreamFormat.mChannelsPerFrame;
        hardwareStreamFormat.mBytesPerFrame = userStreamFormat.mBytesPerFrame;
        hardwareStreamFormat.mBytesPerPacket = userStreamFormat.mBytesPerPacket;
    	        
        if( isInput )
        {
            hostInOut = &pahsc->input;
            srcFormatPtr = &hardwareStreamFormat;
            destFormatPtr = &userStreamFormat;
        }
        else
        {
            hostInOut = &pahsc->output;
            srcFormatPtr = &userStreamFormat;
            destFormatPtr = &hardwareStreamFormat;
        }
        DBUG(("PAOSX_DevicePropertyListener: source rate = %f\n", srcFormatPtr->mSampleRate ));
        DBUG(("PAOSX_DevicePropertyListener: dest rate = %f\n", destFormatPtr->mSampleRate ));
        
        // Don't delete old converter until we create new one so we don't pull
        // the rug out from under other audio threads.
        AudioConverterRef oldConverter = hostInOut->converter;
        
        // Make converter to change sample rate.
        err = AudioConverterNew (
            srcFormatPtr, 
            destFormatPtr, 
            &hostInOut->converter );
        if( err != noErr )
        {
            PRINT_ERR("Could not create format converter", err);
            sSavedHostError = err;
            goto error;
        }
        
        if( oldConverter != NULL )
        {
            verify_noerr( AudioConverterDispose( oldConverter ) );
        }
    }
    
error:
    pahsc->formatListenerCalled = true;
    return err;
}

/* Allocate FIFO between Device callback and Converter callback so that device can push data
* and converter can pull data.
*/
static PaError PaOSX_CreateInputRingBuffer( internalPortAudioStream   *past )
{
    PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
    OSStatus  err = noErr;
    UInt32    dataSize;
    double    sampleRateRatio;
    long      numBytes;
    UInt32    framesPerHostBuffer;
    UInt32    bytesForDevice;
    UInt32    bytesForUser;
    UInt32    bytesPerFrame;
    AudioStreamBasicDescription formatDesc;
    
    dataSize = sizeof(formatDesc);
    err = AudioDeviceGetProperty( pahsc->input.audioDeviceID, 0, IS_INPUT,
        kAudioDevicePropertyStreamFormat, &dataSize, &formatDesc);
    if( err != noErr )
    {
        PRINT_ERR("PaOSX_CreateInputRingBuffer: Could not get input format.\n", err);
        sSavedHostError = err;
        return paHostError;
    }

    // If device is delivering audio faster than being consumed then buffer must be bigger.
    sampleRateRatio = formatDesc.mSampleRate / past->past_SampleRate;
    
    // Get size of CoreAudio IO buffers.
    dataSize = sizeof(framesPerHostBuffer);
    err = AudioDeviceGetProperty( pahsc->input.audioDeviceID, 0, IS_INPUT,
                                kAudioDevicePropertyBufferFrameSize, &dataSize,
                                &framesPerHostBuffer);
    if( err != noErr )
    {
        PRINT_ERR("PaOSX_CreateInputRingBuffer: Could not get input buffer size.\n", err);
        sSavedHostError = err;
        return paHostError;
    }
    
    bytesPerFrame = past->past_NumInputChannels * sizeof(Float32);
    
    bytesForDevice = framesPerHostBuffer * bytesPerFrame * 2;
    bytesForUser = past->past_FramesPerUserBuffer * bytesPerFrame * 3 * sampleRateRatio;
        
    // Ring buffer should be large enough to consume audio input from device,
    // and to deliver a complete user buffer.
    numBytes = (bytesForDevice > bytesForUser) ? bytesForDevice : bytesForUser;
            
    numBytes = RoundUpToNextPowerOf2( numBytes );

    DBUG(("PaOSX_CreateInputRingBuffer: FIFO numBytes = %ld\n", numBytes));
    pahsc->ringBufferData = PaHost_AllocateFastMemory( numBytes );
    if( pahsc->ringBufferData == NULL )
    {
        return paInsufficientMemory;
    }
    RingBuffer_Init( &pahsc->ringBuffer, numBytes, pahsc->ringBufferData );
    // Make it look almost full at beginning. We must advance by an integral number of frames
    // so that the channels don't get scrambled when numChannels is not a power of 2.
    {
        int numZeroFrames = numBytes / bytesPerFrame;
        int numZeroBytes = numZeroFrames * bytesPerFrame;
        RingBuffer_AdvanceWriteIndex( &pahsc->ringBuffer, numZeroBytes );
    }
    
    return paNoError;
}

/******************************************************************
 * Try to set the I/O bufferSize of the device.
 * Scale the size by the ratio of the sample rates so that the converter will have
 * enough data to operate on.
 */
static OSStatus PaOSX_SetDeviceBufferSize( AudioDeviceID devID, Boolean isInput, int framesPerUserBuffer, Float64 sampleRateRatio )
{
    UInt32    dataSize;
    UInt32    ioBufferSize;
    int       scaler;    
    
	scaler = (int) sampleRateRatio;
    if( sampleRateRatio > (Float64) scaler ) scaler += 1;
    DBUG(("PaOSX_SetDeviceBufferSize: buffer size scaler = %d\n", scaler ));
    ioBufferSize = framesPerUserBuffer * scaler;
    
    // Limit buffer size to reasonable value.
    if( ioBufferSize < 128 ) ioBufferSize = 128;

    dataSize = sizeof(ioBufferSize);
    return AudioDeviceSetProperty( devID, 0, 0, isInput,
                            kAudioDevicePropertyBufferFrameSize, dataSize,
                            &ioBufferSize);
}


/*******************************************************************/
static PaError PaOSX_OpenCommonDevice( internalPortAudioStream   *past,
        PaHostInOut *inOut, Boolean isInput )
{
    PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
    PaError          result = paNoError;
    OSStatus         err = noErr;
    Float64          deviceRate;

    PaOSX_FixVolumeScalars( inOut->audioDeviceID, isInput,
        inOut->numChannels, 0.1, 0.9 );

    // The HW device format changes are asynchronous.
    // So we don't know when or if the PAOSX_DevicePropertyListener() will
    // get called. To be safe, call the listener now to forcibly create the converter.
    if( inOut->converter == NULL )
    {
        err = PAOSX_DevicePropertyListener (inOut->audioDeviceID,
                                0, isInput, kAudioDevicePropertyStreamFormat, past);
        if (err != kAudioHardwareNoError)
        {
            PRINT_ERR("PaOSX_OpenCommonDevice: PAOSX_DevicePropertyListener failed.\n", err);
            sSavedHostError = err;
            return paHostError;
        }
    }

    // Add listener for when format changed by other apps.
    DBUG(("PaOSX_OpenCommonDevice: call AudioDeviceAddPropertyListener()\n" ));
	err = AudioDeviceAddPropertyListener( inOut->audioDeviceID, 0, isInput,
        kAudioDevicePropertyStreamFormat,
        (AudioDevicePropertyListenerProc) PAOSX_DevicePropertyListener, past );
    if (err != noErr)
    {
        return -1; // FIXME
    }

    // Only change format if current HW format is different.
    // Don't bother to check result because we are going to use an AudioConverter anyway.
    pahsc->formatListenerCalled = false;
    result = PaOSX_SetFormat( inOut->audioDeviceID, isInput, past->past_SampleRate, inOut->numChannels );
    // Synchronize with device because format changes put some devices into unusable mode.
    if( result > 0 )
    {
        const int sleepDurMsec = 50;
        int spinCount = MIN_TIMEOUT_MSEC / sleepDurMsec;
        while( !pahsc->formatListenerCalled && (spinCount > 0) )
        {
            Pa_Sleep( sleepDurMsec ); // FIXME - use a semaphore or signal
            spinCount--;
        }
        if( !pahsc->formatListenerCalled )
        {
            PRINT(("PaOSX_OpenCommonDevice: timed out waiting for device format to settle.\n"));
        }
        result = 0;
    }
    
#if SET_DEVICE_BUFFER_SIZE
    // Try to set the I/O bufferSize of the device.
    {
        Float64   ratio;
        deviceRate = PaOSX_GetDeviceSampleRate( inOut->audioDeviceID, isInput );
        if( deviceRate <= 0.0 ) deviceRate =  past->past_SampleRate;
        ratio = deviceRate / past->past_SampleRate ;
        err = PaOSX_SetDeviceBufferSize( inOut->audioDeviceID, isInput,
            past->past_FramesPerUserBuffer, ratio );
        if( err != noErr )
        {
            DBUG(("PaOSX_OpenCommonDevice: Could not set I/O buffer size.\n"));
        }
    }
#endif

    /* Allocate an input buffer because we need it between the user callback and the converter. */
    inOut->converterBuffer = PaHost_AllocateFastMemory( inOut->bytesPerUserNativeBuffer );
    if( inOut->converterBuffer == NULL )
    {
        return paInsufficientMemory;
    }

    return result;
}

/*******************************************************************/
static PaError PaOSX_OpenInputDevice( internalPortAudioStream   *past )
{
    PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
    const PaHostDeviceInfo *hostDeviceInfo;
    PaError          result = paNoError;
        
    DBUG(("PaOSX_OpenInputDevice: -------------\n"));
    
    if( (past->past_InputDeviceID < LOWEST_INPUT_DEVID) ||
        (past->past_InputDeviceID > HIGHEST_INPUT_DEVID) )
    {
        return paInvalidDeviceId;
    }
    hostDeviceInfo = &sDeviceInfos[past->past_InputDeviceID];

    if( past->past_NumInputChannels > hostDeviceInfo->paInfo.maxInputChannels )
    {
        return paInvalidChannelCount; /* Too many channels! */
    }
    pahsc->input.numChannels = past->past_NumInputChannels;

    // setup PA conversion procedure
    result = PaConvert_SetupInput( past, paFloat32 );

    result = PaOSX_OpenCommonDevice( past, &pahsc->input, IS_INPUT );
    if( result != paNoError ) goto error;
    
    // Allocate a ring buffer so we can push in data from device, and pull through AudioConverter.
    result = PaOSX_CreateInputRingBuffer( past );
    if( result != paNoError ) goto error;
            
error:
    return result;
}

/*******************************************************************/
static PaError PaOSX_OpenOutputDevice( internalPortAudioStream *past )
{
    PaHostSoundControl *pahsc;
    const PaHostDeviceInfo *hostDeviceInfo;
    PaError          result = paNoError;

    DBUG(("PaOSX_OpenOutputDevice: -------------\n"));
    pahsc = (PaHostSoundControl *) past->past_DeviceData;
    
    // Validate DeviceID
    DBUG(("PaOSX_OpenOutputDevice: deviceID = 0x%x\n", past->past_OutputDeviceID));
    if( (past->past_OutputDeviceID < LOWEST_OUTPUT_DEVID) ||
        (past->past_OutputDeviceID > HIGHEST_OUTPUT_DEVID) )
    {
        return paInvalidDeviceId;
    }
    hostDeviceInfo = &sDeviceInfos[past->past_OutputDeviceID];
    
    // Validate number of channels.
    if( past->past_NumOutputChannels > hostDeviceInfo->paInfo.maxOutputChannels )
    {
        return paInvalidChannelCount; /* Too many channels! */
    }
    pahsc->output.numChannels = past->past_NumOutputChannels;
    
    // setup conversion procedure
    result = PaConvert_SetupOutput( past, paFloat32 );
	if( result != paNoError ) goto error;
        
    result = PaOSX_OpenCommonDevice( past, &pahsc->output, IS_OUTPUT );
    if( result != paNoError ) goto error;
    
error:
    return result;
}

/*******************************************************************
* Determine how many User Buffers we can put into our CoreAudio stream buffer.
* Uses:
*    past->past_FramesPerUserBuffer, etc.
* Sets:
*    past->past_NumUserBuffers
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

    // calculate buffer sizes in bytes
    pahsc->input.bytesPerUserNativeBuffer = past->past_FramesPerUserBuffer *
        Pa_GetSampleSize(paFloat32) * past->past_NumInputChannels;
    pahsc->output.bytesPerUserNativeBuffer = past->past_FramesPerUserBuffer *
        Pa_GetSampleSize(paFloat32) * past->past_NumOutputChannels;

    DBUG(("PaOSX_CalcNumHostBuffers: past_NumUserBuffers = %ld\n", past->past_NumUserBuffers ));
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

    assert( past->past_Magic == PA_MAGIC );
    
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

    useOutput = (past->past_OutputDeviceID != paNoDevice) && (past->past_NumOutputChannels > 0);
    useInput = (past->past_InputDeviceID != paNoDevice) && (past->past_NumInputChannels > 0);
    
    // Set device IDs and determine IO Device mode.
    if( useOutput )
    {
        pahsc->output.audioDeviceID = sDeviceInfos[past->past_OutputDeviceID].audioDeviceID;
        pahsc->primaryDeviceID = pahsc->output.audioDeviceID;
        if( useInput )
        {
            pahsc->input.audioDeviceID = sDeviceInfos[past->past_InputDeviceID].audioDeviceID;
            pahsc->mode = ( pahsc->input.audioDeviceID != pahsc->primaryDeviceID ) ?
                PA_MODE_IO_TWO_DEVICES : PA_MODE_IO_ONE_DEVICE;
        }
        else
        {
            pahsc->mode = PA_MODE_OUTPUT_ONLY;
        }
    }
    else
    {
        /* Just input, not output. */
        pahsc->input.audioDeviceID = sDeviceInfos[past->past_InputDeviceID].audioDeviceID;
        pahsc->primaryDeviceID = pahsc->input.audioDeviceID;
        pahsc->mode = PA_MODE_INPUT_ONLY;
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
    
#if PA_ENABLE_LOAD_MEASUREMENT    
    pahsc->inverseHostTicksPerBuffer = past->past_SampleRate /
        (AudioGetHostClockFrequency() * past->past_FramesPerUserBuffer);
        DBUG(("inverseHostTicksPerBuffer based on buffer size of %d frames.\n", past->past_FramesPerUserBuffer ));
#else
    PRINT(("WARNING - Pa_GetCPULoad() mesaurement disabled in pa_mac_core.c.\n"));
#endif

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
    if( pahsc->mode == PA_MODE_IO_TWO_DEVICES )
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
    
/* If full duplex and using two separate devices then stop second input device. */
    if( pahsc->mode == PA_MODE_IO_TWO_DEVICES )
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
    // Stop Listener callbacks ASAP before dismantling stream.
    if( PA_USING_INPUT )
    {
        AudioDeviceRemovePropertyListener( pahsc->input.audioDeviceID, 0, IS_INPUT,
            kAudioDevicePropertyStreamFormat,
            (AudioDevicePropertyListenerProc) PAOSX_DevicePropertyListener );
    }

    if( PA_USING_OUTPUT )
    {
        AudioDeviceRemovePropertyListener( pahsc->output.audioDeviceID, 0, IS_OUTPUT,
            kAudioDevicePropertyStreamFormat,
            (AudioDevicePropertyListenerProc) PAOSX_DevicePropertyListener );
    }

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
	if ( pahsc->input.streamInterleavingBuffer != NULL )
    {
        PaHost_FreeFastMemory( pahsc->input.streamInterleavingBuffer, pahsc->input.streamInterleavingBufferLen );
    }
	if ( pahsc->output.streamInterleavingBuffer != NULL )
    {
        PaHost_FreeFastMemory( pahsc->output.streamInterleavingBuffer, pahsc->output.streamInterleavingBufferLen );
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
** on minimum latency. Because CoreAudio manages latency, this just selects
** a reasonably small number of buffers.
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


