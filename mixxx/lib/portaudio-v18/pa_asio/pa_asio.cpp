/*
 * $Id: pa_asio.cpp 479 2003-08-13 11:03:25Z tuehaste $
 * Portable Audio I/O Library for ASIO Drivers
 *
 * Author: Stephane Letz
 * Based on the Open Source API proposed by Ross Bencina
 * Copyright (c) 2000-2001 Stephane Letz, Phil Burk
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
 */

/* Modification History

        08-03-01 First version : Stephane Letz
        08-06-01 Tweaks for PC, use C++, buffer allocation, Float32 to Int32 conversion : Phil Burk
        08-20-01 More conversion, PA_StreamTime, Pa_GetHostError : Stephane Letz
        08-21-01 PaUInt8 bug correction, implementation of ASIOSTFloat32LSB and ASIOSTFloat32MSB native formats : Stephane Letz
        08-24-01 MAX_INT32_FP hack, another Uint8 fix : Stephane and Phil
        08-27-01 Implementation of hostBufferSize < userBufferSize case, better management of the ouput buffer when
                 the stream is stopped : Stephane Letz
        08-28-01 Check the stream pointer for null in bufferSwitchTimeInfo, correct bug in bufferSwitchTimeInfo when 
                 the stream is stopped : Stephane Letz
        10-12-01 Correct the PaHost_CalcNumHostBuffers function: computes FramesPerHostBuffer to be the lowest that
                 respect requested FramesPerUserBuffer and userBuffersPerHostBuffer : Stephane Letz
        10-26-01 Management of hostBufferSize and userBufferSize of any size : Stephane Letz
        10-27-01 Improve calculus of hostBufferSize to be multiple or divisor of userBufferSize if possible : Stephane and Phil
        10-29-01 Change MAX_INT32_FP to (2147483520.0f) to prevent roundup to 0x80000000 : Phil Burk
        10-31-01 Clear the ouput buffer and user buffers in PaHost_StartOutput, correct bug in GetFirstMultiple : Stephane Letz 
        11-06-01 Rename functions : Stephane Letz 
        11-08-01 New Pa_ASIO_Adaptor_Init function to init Callback adpatation variables, cleanup of Pa_ASIO_Callback_Input: Stephane Letz 
        11-29-01 Break apart device loading to debug random failure in Pa_ASIO_QueryDeviceInfo ; Phil Burk
        01-03-02 Desallocate all resources in PaHost_Term for cases where Pa_CloseStream is not called properly :  Stephane Letz
        02-01-02 Cleanup, test of multiple-stream opening : Stephane Letz
        19-02-02 New Pa_ASIO_loadDriver that calls CoInitialize on each thread on Windows : Stephane Letz
        09-04-02 Correct error code management in PaHost_Term, removes various compiler warning : Stephane Letz
        12-04-02 Add Mac includes for <Devices.h> and <Timer.h> : Phil Burk
        13-04-02 Removes another compiler warning : Stephane Letz
        30-04-02 Pa_ASIO_QueryDeviceInfo bug correction, memory allocation checking, better error handling : D Viens, P Burk, S Letz
        01-12-02 Fix Pa_GetDefaultInputDeviceID and Pa_GetDefaultOuputDeviceID result when no driver are available : S Letz
        05-12-02 More debug messages : S Letz
        01-23-03 Increased max channels to 128. Fixed comparison of (OutputChannels > kMaxInputChannels) : P Burk
        02-17-03 Better termination handling : PaHost_CloseStream is called in PaHost_term is the the stream was not explicitely closed by the application : S Letz
        04-02-03 More robust ASIO driver buffer size initialization : some buggy drivers (like the Hoontech DSP24) give incorrect [min, preferred, max] values
       	   		 They should work with the preferred size value, thus if Pa_ASIO_CreateBuffers fails with the hostBufferSize computed in PaHost_CalcNumHostBuffers, 
       	   		 we try again with the preferred size. Fix an old (never detected?) bug in the buffer adapdation code : S Letz
       	30-06-03 The audio callback was not protected against reentrancy : some drivers (like the Hoontech DSP24) seems to cause this behaviour 
       			 that corrupted the buffer adapdation state and finally caused crashes. The reentrancy state is now checked in bufferSwitchTimeInfo : S Letz 
       	   		 
         TO DO :
        
        - Check Pa_StopSteam and Pa_AbortStream
        - Optimization for Input only or Ouput only (really necessary ??)
*/


#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "portaudio.h"
#include "pa_host.h"
#include "pa_trace.h"

#include "asiosys.h"
#include "asio.h"
#include "asiodrivers.h"


#if MAC
#include <Devices.h>
#include <Timer.h>
#include <Math64.h>
#else
#include <math.h>
#include <windows.h>
#include <mmsystem.h>
#endif

enum {
        // number of input and outputs supported by the host application
        // you can change these to higher or lower values
        kMaxInputChannels = 128,
        kMaxOutputChannels = 128
};

/* ASIO specific device information. */
typedef struct internalPortAudioDevice
{       
        PaDeviceInfo    pad_Info;
} internalPortAudioDevice;


/*  ASIO driver internal data storage */
typedef struct PaHostSoundControl
{
        // ASIOInit()
        ASIODriverInfo  pahsc_driverInfo;

        // ASIOGetChannels()
        int32           pahsc_NumInputChannels;         
        int32           pahsc_NumOutputChannels;        

        // ASIOGetBufferSize() - sizes in frames per buffer
        int32           pahsc_minSize;
        int32           pahsc_maxSize;
        int32           pahsc_preferredSize;
        int32           pahsc_granularity;

        // ASIOGetSampleRate()
        ASIOSampleRate pahsc_sampleRate;

        // ASIOOutputReady()
        bool           pahsc_postOutput;

        // ASIOGetLatencies ()
        int32          pahsc_inputLatency;
        int32          pahsc_outputLatency;

        // ASIOCreateBuffers ()
        ASIOBufferInfo bufferInfos[kMaxInputChannels + kMaxOutputChannels]; // buffer info's

        // ASIOGetChannelInfo()
        ASIOChannelInfo pahsc_channelInfos[kMaxInputChannels + kMaxOutputChannels]; // channel info's
        // The above two arrays share the same indexing, as the data in them are linked together

        // Information from ASIOGetSamplePosition()
        // data is converted to double floats for easier use, however 64 bit integer can be used, too
        double         nanoSeconds;
        double         samples;
        double         tcSamples;       // time code samples

        // bufferSwitchTimeInfo()
        ASIOTime       tInfo;           // time info state
        unsigned long  sysRefTime;      // system reference time, when bufferSwitch() was called

        // Signal the end of processing in this example
        bool           stopped;
        
        ASIOCallbacks   pahsc_asioCallbacks;
        
         
        int32   pahsc_userInputBufferFrameOffset;   // Position in Input user buffer
        int32   pahsc_userOutputBufferFrameOffset;  // Position in Output user buffer
        int32   pahsc_hostOutputBufferFrameOffset;  // Position in Output ASIO buffer
        
        int32  past_FramesPerHostBuffer;        // Number of frames in ASIO buffer
        
        int32  pahsc_InputBufferOffset;         // Number of null frames for input buffer alignement
        int32  pahsc_OutputBufferOffset;        // Number of null frames for ouput buffer alignement
        
#if MAC
        UInt64   pahsc_EntryCount;
        UInt64   pahsc_LastExitCount;
#elif WINDOWS
        LARGE_INTEGER      pahsc_EntryCount;
        LARGE_INTEGER      pahsc_LastExitCount;
#endif  
        
        PaTimestamp   pahsc_NumFramesDone;
        
        internalPortAudioStream   *past;
        
        int32 reenterCount; // Counter of audio callback reentrancy
        int32 reenterError; // Counter of audio callback reentrancy detection
        
} PaHostSoundControl;


//----------------------------------------------------------
#define PRINT(x) { printf x; fflush(stdout); }
#define ERR_RPT(x) PRINT(x)

#define DBUG(x)   /* PRINT(x)  */
#define DBUGX(x)  /* PRINT(x)  */

/* We are trying to be compatible with CARBON but this has not been thoroughly tested. */
#define CARBON_COMPATIBLE  (0)
#define PA_MAX_DEVICE_INFO (32)

#define MIN_INT8     (-0x80)
#define MAX_INT8     (0x7F)

#define MIN_INT8_FP  ((float)-0x80)
#define MAX_INT8_FP  ((float)0x7F)

#define MIN_INT16_FP ((float)-0x8000)
#define MAX_INT16_FP ((float)0x7FFF)

#define MIN_INT16    (-0x8000)
#define MAX_INT16    (0x7FFF)

#define MAX_INT32_FP (2147483520.0f)  /* 0x0x7FFFFF80 - seems safe */

/************************************************************************************/
/****************** Data ************************************************************/
/************************************************************************************/
static int                 sNumDevices = 0;
static internalPortAudioDevice sDevices[PA_MAX_DEVICE_INFO] = { 0 };
static int32               sPaHostError = 0;
static int                 sDefaultOutputDeviceID = 0;
static int                 sDefaultInputDeviceID = 0;

PaHostSoundControl asioDriverInfo = {0};

#ifdef MAC 
static bool swap = true;
#elif WINDOWS   
static bool swap = false;
#endif

// Prototypes
static void bufferSwitch(long index, ASIOBool processNow);
static ASIOTime *bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow);
static void sampleRateChanged(ASIOSampleRate sRate);
static long asioMessages(long selector, long value, void* message, double* opt);
static void Pa_StartUsageCalculation( internalPortAudioStream   *past );
static void Pa_EndUsageCalculation( internalPortAudioStream   *past );

static void Pa_ASIO_Convert_Inter_Input(
        ASIOBufferInfo* nativeBuffer, 
        void* inputBuffer,
        long NumInputChannels, 
        long NumOuputChannels,
        long framePerBuffer,
        long hostFrameOffset,
        long userFrameOffset,
        ASIOSampleType nativeFormat, 
        PaSampleFormat paFormat, 
        PaStreamFlags flags,
        long index);

static void Pa_ASIO_Convert_Inter_Output(
        ASIOBufferInfo* nativeBuffer, 
        void* outputBuffer,
        long NumInputChannels, 
        long NumOuputChannels,
        long framePerBuffer,
        long hostFrameOffset,
        long userFrameOffset,
        ASIOSampleType nativeFormat, 
        PaSampleFormat paFormat, 
        PaStreamFlags flags,
        long index);

static void Pa_ASIO_Clear_Output(ASIOBufferInfo* nativeBuffer, 
        ASIOSampleType nativeFormat,
        long NumInputChannels, 
        long NumOuputChannels,
        long index, 
        long hostFrameOffset, 
        long frames);

static void Pa_ASIO_Callback_Input(long index);
static void Pa_ASIO_Callback_Output(long index, long framePerBuffer);
static void Pa_ASIO_Callback_End();
static void Pa_ASIO_Clear_User_Buffers();

// Some external references
extern AsioDrivers* asioDrivers ;
bool loadAsioDriver(char *name);
unsigned long get_sys_reference_time();


/************************************************************************************/
/****************** Macro  ************************************************************/
/************************************************************************************/

#define SwapLong(v) ((((v)>>24)&0xFF)|(((v)>>8)&0xFF00)|(((v)&0xFF00)<<8)|(((v)&0xFF)<<24)) ;   
#define SwapShort(v) ((((v)>>8)&0xFF)|(((v)&0xFF)<<8)) ;        

#define ClipShort(v) (((v)<MIN_INT16)?MIN_INT16:(((v)>MAX_INT16)?MAX_INT16:(v)))
#define ClipChar(v) (((v)<MIN_INT8)?MIN_INT8:(((v)>MAX_INT8)?MAX_INT8:(v)))
#define ClipFloat(v) (((v)<-1.0f)?-1.0f:(((v)>1.0f)?1.0f:(v)))

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef max
#define max(a,b) ((a)>=(b)?(a):(b))
#endif


static bool Pa_ASIO_loadAsioDriver(char *name)
{
	#ifdef	WINDOWS
		CoInitialize(0);
	#endif
	return loadAsioDriver(name);
}
 


// Utilities for alignement buffer size computation
static int PGCD (int a, int b) {return (b == 0) ? a : PGCD (b,a%b);}
static int PPCM (int a, int b) {return (a*b) / PGCD (a,b);}

// Takes the size of host buffer and user buffer : returns the number of frames needed for buffer adaptation
static int Pa_ASIO_CalcFrameShift (int M, int N)
{
        int res = 0;
        for (int i = M; i < PPCM (M,N) ; i+=M) { res = max (res, i%N); }
        return res;
}

// We have the following relation :
// Pa_ASIO_CalcFrameShift (M,N) + M = Pa_ASIO_CalcFrameShift (N,M) + N
                        
/* ASIO sample type to PortAudio sample type conversion */
static PaSampleFormat Pa_ASIO_Convert_SampleFormat(ASIOSampleType type) 
{
        switch (type) {
        
                case ASIOSTInt16MSB:
                case ASIOSTInt16LSB:
                case ASIOSTInt32MSB16:
                case ASIOSTInt32LSB16:
                        return paInt16;
                
                case ASIOSTFloat32MSB:
                case ASIOSTFloat32LSB:
                case ASIOSTFloat64MSB:
                case ASIOSTFloat64LSB:
                        return paFloat32;
                
                case ASIOSTInt32MSB:
                case ASIOSTInt32LSB:
                case ASIOSTInt32MSB18:          
                case ASIOSTInt32MSB20:          
                case ASIOSTInt32MSB24:          
                case ASIOSTInt32LSB18:          
                case ASIOSTInt32LSB20:          
                case ASIOSTInt32LSB24:          
                        return paInt32;
                        
                case ASIOSTInt24MSB:
                case ASIOSTInt24LSB:
                        return paInt24;
                        
                default:
                        return paCustomFormat;
        }
}



//--------------------------------------------------------------------------------------------------------------------
static void PaHost_CalcBufferOffset(internalPortAudioStream   *past)
{
	 if (asioDriverInfo.past_FramesPerHostBuffer > past->past_FramesPerUserBuffer){
            // Computes the MINIMUM value of null frames shift for the output buffer alignement
            asioDriverInfo.pahsc_OutputBufferOffset = Pa_ASIO_CalcFrameShift (asioDriverInfo.past_FramesPerHostBuffer,past->past_FramesPerUserBuffer);
            asioDriverInfo.pahsc_InputBufferOffset = 0;
            DBUG(("PaHost_CalcBufferOffset : Minimum BufferOffset for Output = %d\n", asioDriverInfo.pahsc_OutputBufferOffset));
    }else{
    
            //Computes the MINIMUM value of null frames shift for the input buffer alignement
            asioDriverInfo.pahsc_InputBufferOffset = Pa_ASIO_CalcFrameShift (asioDriverInfo.past_FramesPerHostBuffer,past->past_FramesPerUserBuffer);
            asioDriverInfo.pahsc_OutputBufferOffset = 0;
            DBUG(("PaHost_CalcBufferOffset : Minimum BufferOffset for Input = %d\n", asioDriverInfo.pahsc_InputBufferOffset));
    }
}

//--------------------------------------------------------------------------------------------------------------------
/* Allocate ASIO buffers, initialise channels */
static ASIOError Pa_ASIO_CreateBuffers (PaHostSoundControl *asioDriverInfo, long InputChannels,
                                                                          long OutputChannels, long framesPerBuffer)
{
        ASIOError  err;
        int i;
        
        ASIOBufferInfo *info = asioDriverInfo->bufferInfos;
        
        // Check parameters
        if ((InputChannels > kMaxInputChannels) || (OutputChannels > kMaxOutputChannels)) return ASE_InvalidParameter;
        
        for(i = 0; i < InputChannels; i++, info++){
                info->isInput = ASIOTrue;
                info->channelNum = i;
                info->buffers[0] = info->buffers[1] = 0;
        }
        
        for(i = 0; i < OutputChannels; i++, info++){
                info->isInput = ASIOFalse;
                info->channelNum = i;
                info->buffers[0] = info->buffers[1] = 0;
        }
        
        // Set up the asioCallback structure and create the ASIO data buffer
        asioDriverInfo->pahsc_asioCallbacks.bufferSwitch = &bufferSwitch;
        asioDriverInfo->pahsc_asioCallbacks.sampleRateDidChange = &sampleRateChanged;
        asioDriverInfo->pahsc_asioCallbacks.asioMessage = &asioMessages;
        asioDriverInfo->pahsc_asioCallbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfo;
        
        DBUG(("Pa_ASIO_CreateBuffers : ASIOCreateBuffers with inputChannels = %ld \n", InputChannels));
        DBUG(("Pa_ASIO_CreateBuffers : ASIOCreateBuffers with OutputChannels = %ld \n", OutputChannels));
        DBUG(("Pa_ASIO_CreateBuffers : ASIOCreateBuffers with size = %ld \n", framesPerBuffer));
     
        err =  ASIOCreateBuffers( asioDriverInfo->bufferInfos, InputChannels+OutputChannels,
                                  framesPerBuffer, &asioDriverInfo->pahsc_asioCallbacks);
        if (err != ASE_OK) return err;
        
        // Initialise buffers
        for (i = 0; i < InputChannels + OutputChannels; i++)
        {
                asioDriverInfo->pahsc_channelInfos[i].channel = asioDriverInfo->bufferInfos[i].channelNum;
                asioDriverInfo->pahsc_channelInfos[i].isInput = asioDriverInfo->bufferInfos[i].isInput;
                err = ASIOGetChannelInfo(&asioDriverInfo->pahsc_channelInfos[i]);
                if (err != ASE_OK) break;
        }

        err = ASIOGetLatencies(&asioDriverInfo->pahsc_inputLatency, &asioDriverInfo->pahsc_outputLatency);
        
        DBUG(("Pa_ASIO_CreateBuffers : InputLatency = %ld latency = %ld msec \n", 
                asioDriverInfo->pahsc_inputLatency,  
                (long)((asioDriverInfo->pahsc_inputLatency*1000)/ asioDriverInfo->past->past_SampleRate)));
        DBUG(("Pa_ASIO_CreateBuffers : OuputLatency = %ld latency = %ld msec \n", 
                asioDriverInfo->pahsc_outputLatency,
                (long)((asioDriverInfo->pahsc_outputLatency*1000)/ asioDriverInfo->past->past_SampleRate)));
        
        return err;
}


/*
 Query ASIO driver info :
 
 First we get all available ASIO drivers located in the ASIO folder,
 then try to load each one. For each loaded driver, get all needed informations.
*/
static PaError Pa_ASIO_QueryDeviceInfo( internalPortAudioDevice * ipad )
{

#define NUM_STANDARDSAMPLINGRATES   3   /* 11.025, 22.05, 44.1 */
#define NUM_CUSTOMSAMPLINGRATES     9   /* must be the same number of elements as in the array below */
#define MAX_NUMSAMPLINGRATES  (NUM_STANDARDSAMPLINGRATES+NUM_CUSTOMSAMPLINGRATES)

        ASIOSampleRate possibleSampleRates[] 
                = {8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0, 44100.0, 48000.0, 88200.0, 96000.0};
                
        ASIOChannelInfo channelInfos;
        long InputChannels,OutputChannels;
        double *sampleRates;
        char* names[PA_MAX_DEVICE_INFO] ;
        PaDeviceInfo *dev;
        int           i;
        int           numDrivers;
		ASIOError     asioError;
		
	   /* Allocate names */
        for (i = 0 ; i < PA_MAX_DEVICE_INFO ; i++) {
        	names[i] = (char*)PaHost_AllocateFastMemory(32);
        	/* check memory */
        	if(!names[i]) return paInsufficientMemory;
        }
        
        /* MUST BE CHECKED : to force fragments loading on Mac */
        Pa_ASIO_loadAsioDriver("dummy");
        
        /* Get names of all available ASIO drivers */
        asioDrivers->getDriverNames(names,PA_MAX_DEVICE_INFO);
        
        /* Check all available ASIO drivers */
#if MAC
        numDrivers = asioDrivers->getNumFragments();
#elif WINDOWS
        numDrivers = asioDrivers->asioGetNumDev();
#endif

        DBUG(("PaASIO_QueryDeviceInfo: number of installed drivers = %d\n", numDrivers ));

        for (int driver = 0 ; driver < numDrivers ; driver++)
        {

            #if WINDOWS
                    asioDriverInfo.pahsc_driverInfo.asioVersion = 2; // FIXME - is this right? PLB
                    asioDriverInfo.pahsc_driverInfo.sysRef = GetDesktopWindow(); // FIXME - is this right? PLB
            #endif
            
            DBUG(("---------------------------------------\n"));
            
            DBUG(("PaASIO_QueryDeviceInfo: Driver name = %s\n", names[driver]));
  
            /* If the driver can be loaded : */
            if ( !Pa_ASIO_loadAsioDriver(names[driver]) ){
                     DBUG(("PaASIO_QueryDeviceInfo could not loadAsioDriver %s\n", names[driver]));
            } else {
            
                    DBUG(("PaASIO_QueryDeviceInfo: loadAsioDriver OK\n"));
                    
                    if((asioError = ASIOInit(&asioDriverInfo.pahsc_driverInfo)) != ASE_OK){
                    
                         DBUG(("PaASIO_QueryDeviceInfo: ASIOInit returned %d for %s\n", asioError, names[driver]));
                         
                    }else {
                    
                    	 DBUG(("PaASIO_QueryDeviceInfo: ASIOInit OK \n"));
                    	 
                    	 if(ASIOGetChannels(&InputChannels, &OutputChannels) != ASE_OK){
                    	 
                            DBUG(("PaASIO_QueryDeviceInfo could not ASIOGetChannels for %s\n", names[driver]));
                            
                         }else {
                            
                            DBUG(("PaASIO_QueryDeviceInfo: ASIOGetChannels OK \n"));
                                
                            /* Gets the name */
                            dev = &(ipad[sNumDevices].pad_Info);
                            dev->name = names[driver];
                            names[driver] = 0;
                            
                            /* Gets Input and Output channels number */
                            dev->maxInputChannels = InputChannels;
                            dev->maxOutputChannels = OutputChannels;
                            
                            DBUG(("PaASIO_QueryDeviceInfo: InputChannels = %d\n", InputChannels ));
                            DBUG(("PaASIO_QueryDeviceInfo: OutputChannels = %d\n", OutputChannels ));
                            
                            /* Make room in case device supports all rates. */
                            sampleRates = (double*)PaHost_AllocateFastMemory(MAX_NUMSAMPLINGRATES * sizeof(double));
                            /* check memory */
                            if (!sampleRates) {
                                ASIOExit();
                                return paInsufficientMemory;
                            }
                            dev->sampleRates = sampleRates;
                            dev->numSampleRates = 0;
                            
                            /* Loop through the possible sampling rates and check each to see if the device supports it. */
                            for (int index = 0; index < MAX_NUMSAMPLINGRATES; index++) {
                                    if (ASIOCanSampleRate(possibleSampleRates[index]) != ASE_NoClock) {
                                            DBUG(("PaASIO_QueryDeviceInfo: possible sample rate = %d\n", (long)possibleSampleRates[index]));
                                            dev->numSampleRates += 1;
                                            *sampleRates = possibleSampleRates[index];
                                            sampleRates++;
                                    }
                            }
                            
                            /* We assume that all channels have the same SampleType, so check the first */
                            channelInfos.channel = 0;
                            channelInfos.isInput = 1;
                           
                            if ((asioError = ASIOGetChannelInfo(&channelInfos)) == ASE_NotPresent) {
                            	DBUG(("PaASIO_QueryDeviceInfo: ASIOGetChannelInfo returned %d \n",asioError)); 
                            }
                            
                            dev->nativeSampleFormats = Pa_ASIO_Convert_SampleFormat(channelInfos.type);
                            
                            /* unload the driver */
                            if ((asioError = ASIOExit()) != ASE_OK) {
                            	DBUG(("PaASIO_QueryDeviceInfo: ASIOExit returned %d \n",asioError)); 
                            }
                            
                            sNumDevices++;
                        }
		                   
                    }
               }
        }
                    
        /* free only unused names */
        for (i = 0 ; i < PA_MAX_DEVICE_INFO ; i++) if (names[i]) PaHost_FreeFastMemory(names[i],32);
        
        return paNoError;
}

//----------------------------------------------------------------------------------
// TAKEN FROM THE ASIO SDK: 
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
// TAKEN FROM THE ASIO SDK: 
long asioMessages(long selector, long value, void* message, double* opt)
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
                        
                case kAsioBufferSizeChange:
                        //printf("kAsioBufferSizeChange \n");
                        break;
                        
                case kAsioResetRequest:
                        // defer the task and perform the reset of the driver during the next "safe" situation
                        // You cannot reset the driver right now, as this code is called from the driver.
                        // Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
                        // Afterwards you initialize the driver again.
                        asioDriverInfo.stopped;  // In this sample the processing will just stop
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
                        //printf("kAsioLatenciesChanged \n");
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


//----------------------------------------------------------------------------------
// conversion from 64 bit ASIOSample/ASIOTimeStamp to double float
#if NATIVE_INT64
        #define ASIO64toDouble(a)  (a)
#else
        const double twoRaisedTo32 = 4294967296.;
        #define ASIO64toDouble(a)  ((a).lo + (a).hi * twoRaisedTo32)
#endif


static ASIOTime *bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow)
{       
        // the actual processing callback.
        // Beware that this is normally in a seperate thread, hence be sure that you take care
        // about thread synchronization. This is omitted here for simplicity.
        
        // store the timeInfo for later use
        asioDriverInfo.tInfo = *timeInfo;

        // get the time stamp of the buffer, not necessary if no
        // synchronization to other media is required
        
        if (timeInfo->timeInfo.flags & kSystemTimeValid)
                asioDriverInfo.nanoSeconds = ASIO64toDouble(timeInfo->timeInfo.systemTime);
        else
                asioDriverInfo.nanoSeconds = 0;

        if (timeInfo->timeInfo.flags & kSamplePositionValid)
                asioDriverInfo.samples = ASIO64toDouble(timeInfo->timeInfo.samplePosition);
        else
                asioDriverInfo.samples = 0;

        if (timeInfo->timeCode.flags & kTcValid)
                asioDriverInfo.tcSamples = ASIO64toDouble(timeInfo->timeCode.timeCodeSamples);
        else
                asioDriverInfo.tcSamples = 0;

        // get the system reference time
        asioDriverInfo.sysRefTime = get_sys_reference_time();

#if 0
        // a few debug messages for the Windows device driver developer
        // tells you the time when driver got its interrupt and the delay until the app receives
        // the event notification.
        static double last_samples = 0;
        char tmp[128];
        sprintf (tmp, "diff: %d / %d ms / %d ms / %d samples                 \n", asioDriverInfo.sysRefTime - (long)(asioDriverInfo.nanoSeconds / 1000000.0), asioDriverInfo.sysRefTime, (long)(asioDriverInfo.nanoSeconds / 1000000.0), (long)(asioDriverInfo.samples - last_samples));
        OutputDebugString (tmp);
        last_samples = asioDriverInfo.samples;
#endif

        // To avoid the callback accessing a desallocated stream
        if (asioDriverInfo.past == NULL) return 0L;
        
        // Keep sample position
	    asioDriverInfo.pahsc_NumFramesDone = timeInfo->timeInfo.samplePosition.lo;
        
        // Reentrancy control
        if( ++asioDriverInfo.reenterCount) {
        	asioDriverInfo.reenterError++;
        	DBUG(("bufferSwitchTimeInfo : reentrancy detection = %d\n", asioDriverInfo.reenterError));
       		return 0L;
        }
		
		do {

           	/*  Has a user callback returned '1' to indicate finished at the last ASIO callback? */
	        if( asioDriverInfo.past->past_StopSoon ) {
	        
	                Pa_ASIO_Clear_Output(asioDriverInfo.bufferInfos, 
	                        asioDriverInfo.pahsc_channelInfos[0].type,
	                        asioDriverInfo.pahsc_NumInputChannels ,
	                        asioDriverInfo.pahsc_NumOutputChannels,
	                        index, 
	                        0, 
	                        asioDriverInfo.past_FramesPerHostBuffer);
	                
	                asioDriverInfo.past->past_IsActive = 0; 
	                
	                // Finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
	                if (asioDriverInfo.pahsc_postOutput) ASIOOutputReady();
	       
	        }else {
	                
	                /* CPU usage */
	                Pa_StartUsageCalculation(asioDriverInfo.past);
	                
	                Pa_ASIO_Callback_Input(index);
	                         
	                // Finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
	                if (asioDriverInfo.pahsc_postOutput) ASIOOutputReady();
	                
	                Pa_ASIO_Callback_End();
	                        
	                /* CPU usage */
	                Pa_EndUsageCalculation(asioDriverInfo.past);
	        }
        
       	} while(asioDriverInfo.reenterCount--);
        
        return 0L;
}


//----------------------------------------------------------------------------------
void bufferSwitch(long index, ASIOBool processNow)
{       
        // the actual processing callback.
        // Beware that this is normally in a seperate thread, hence be sure that you take care
        // about thread synchronization. This is omitted here for simplicity.

        // as this is a "back door" into the bufferSwitchTimeInfo a timeInfo needs to be created
        // though it will only set the timeInfo.samplePosition and timeInfo.systemTime fields and the according flags
        
        ASIOTime  timeInfo;
        memset (&timeInfo, 0, sizeof (timeInfo));

        // get the time stamp of the buffer, not necessary if no
        // synchronization to other media is required
        if(ASIOGetSamplePosition(&timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime) == ASE_OK)
                timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;
                
        // Call the real callback
        bufferSwitchTimeInfo (&timeInfo, index, processNow);
}

//----------------------------------------------------------------------------------
unsigned long get_sys_reference_time()
{       
        // get the system reference time
        #if WINDOWS
                return timeGetTime();
	     #elif MAC
                static const double twoRaisedTo32 = 4294967296.;
                UnsignedWide ys;
                Microseconds(&ys);
                double r = ((double)ys.hi * twoRaisedTo32 + (double)ys.lo);
                return (unsigned long)(r / 1000.);
        #endif
}


/*************************************************************
** Calculate 2 LSB dither signal with a triangular distribution.
** Ranged properly for adding to a 32 bit integer prior to >>15.
*/
#define DITHER_BITS   (15)
#define DITHER_SCALE  (1.0f / ((1<<DITHER_BITS)-1))
inline static long Pa_TriangularDither( void )
{
        static unsigned long previous = 0;
        static unsigned long randSeed1 = 22222;
        static unsigned long randSeed2 = 5555555;
        long current, highPass;
/* Generate two random numbers. */
        randSeed1 = (randSeed1 * 196314165) + 907633515;
        randSeed2 = (randSeed2 * 196314165) + 907633515;
/* Generate triangular distribution about 0. */
        current = (((long)randSeed1)>>(32-DITHER_BITS)) + (((long)randSeed2)>>(32-DITHER_BITS));
 /* High pass filter to reduce audibility. */
        highPass = current - previous;
        previous = current;
        return highPass;
}

// TO BE COMPLETED WITH ALL SUPPORTED PA SAMPLE TYPES

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Input_Int16_Float32 (ASIOBufferInfo* nativeBuffer, float *inBufPtr, int framePerBuffer, int NumInputChannels, int index, int hostFrameOffset,int userFrameOffset, bool swap)
{
        long temp;
        int i,j;
        
        for( j=0; j<NumInputChannels; j++ ) {
                short *asioBufPtr = &((short*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                float *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                for (i= 0; i < framePerBuffer; i++)
                { 
                        temp = asioBufPtr[i];
                        if (swap) temp = SwapShort(temp);
                        *userBufPtr = (1.0f / MAX_INT16_FP) * temp;
                        userBufPtr += NumInputChannels;
                }
        }
        
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Input_Int32_Float32 (ASIOBufferInfo* nativeBuffer, float *inBufPtr, int framePerBuffer, int NumInputChannels, int index, int hostFrameOffset,int userFrameOffset,bool swap)
{
        long temp;
        int i,j;
        
        for( j=0; j<NumInputChannels; j++ ) {
                long *asioBufPtr = &((long*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                float *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                for (i= 0; i < framePerBuffer; i++)
                { 
                        temp = asioBufPtr[i];
                        if (swap) temp = SwapLong(temp);
                        *userBufPtr = (1.0f / MAX_INT32_FP) * temp;
                        userBufPtr += NumInputChannels;
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
// MUST BE TESTED
static void Input_Float32_Float32 (ASIOBufferInfo* nativeBuffer, float *inBufPtr, int framePerBuffer, int NumInputChannels, int index, int hostFrameOffset,int userFrameOffset,bool swap)
{
        unsigned long temp;
        int i,j;
        
        for( j=0; j<NumInputChannels; j++ ) {
                unsigned long *asioBufPtr = &((unsigned long*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                float *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                for (i= 0; i < framePerBuffer; i++)
                { 
                        temp = asioBufPtr[i];
                        if (swap) temp = SwapLong(temp);
                        *userBufPtr = (float)temp;
                        userBufPtr += NumInputChannels;
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static  void Input_Int16_Int32 (ASIOBufferInfo* nativeBuffer, long *inBufPtr, int framePerBuffer, int NumInputChannels, int index, int hostFrameOffset,int userFrameOffset,bool swap)
{
        long temp;
        int i,j;
        
        for( j=0; j<NumInputChannels; j++ ) {
                short *asioBufPtr = &((short*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                long *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                for (i= 0; i < framePerBuffer; i++)
                { 
                        temp = asioBufPtr[i];
                        if (swap) temp = SwapShort(temp);
                        *userBufPtr = temp<<16;
                        userBufPtr += NumInputChannels;
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static  void Input_Int32_Int32 (ASIOBufferInfo* nativeBuffer, long *inBufPtr, int framePerBuffer, int NumInputChannels, int index, int hostFrameOffset,int userFrameOffset,bool swap)
{
        long temp;
        int i,j;
        
        for( j=0; j<NumInputChannels; j++ ) {
                long *asioBufPtr = &((long*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                long *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                for (i= 0; i < framePerBuffer; i++)
                { 
                        temp = asioBufPtr[i];
                        if (swap) temp = SwapLong(temp);
                        *userBufPtr = temp;
                        userBufPtr += NumInputChannels;
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
// MUST BE TESTED
static  void Input_Float32_Int32 (ASIOBufferInfo* nativeBuffer, long *inBufPtr, int framePerBuffer, int NumInputChannels, int index, int hostFrameOffset,int userFrameOffset,bool swap)
{
        unsigned long temp;
        int i,j;

        for( j=0; j<NumInputChannels; j++ ) {
                unsigned long *asioBufPtr = &((unsigned long*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                long *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                for (i= 0; i < framePerBuffer; i++)
                { 
                        temp = asioBufPtr[i];
                        if (swap) temp = SwapLong(temp);
                        *userBufPtr = (long)((float)temp * MAX_INT32_FP); // Is temp a value between -1.0 and 1.0 ??
                        userBufPtr += NumInputChannels;
                }
        }
}


//-------------------------------------------------------------------------------------------------------------------------------------------------------
static  void Input_Int16_Int16 (ASIOBufferInfo* nativeBuffer, short *inBufPtr, int framePerBuffer, int NumInputChannels, int index, int hostFrameOffset,int userFrameOffset,bool swap)
{
        long temp;
        int i,j;

        for( j=0; j<NumInputChannels; j++ ) {
                short *asioBufPtr = &((short*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                short *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                for (i= 0; i < framePerBuffer; i++)
                { 
                        temp = asioBufPtr[i];
                        if (swap) temp = SwapShort(temp);
                        *userBufPtr = (short)temp;
                        userBufPtr += NumInputChannels;
                }
        }
}
 
 //-------------------------------------------------------------------------------------------------------------------------------------------------------
static  void Input_Int32_Int16 (ASIOBufferInfo* nativeBuffer, short *inBufPtr, int framePerBuffer, int NumInputChannels, int index, int hostFrameOffset, int userFrameOffset,uint32 flags,bool swap)
{
        long temp;
        int i,j;
        
        if( flags & paDitherOff )
        {
                for( j=0; j<NumInputChannels; j++ ) {
                        long *asioBufPtr = &((long*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                        short *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                        for (i= 0; i < framePerBuffer; i++)
                        { 
                                temp = asioBufPtr[i];
                                if (swap) temp = SwapLong(temp);
                                *userBufPtr = (short)(temp>>16);
                                userBufPtr += NumInputChannels;
                        }
                }
        }
        else
        {
                for( j=0; j<NumInputChannels; j++ ) {
                        long *asioBufPtr = &((long*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                        short *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                        for (i= 0; i < framePerBuffer; i++)
                        { 
                                temp = asioBufPtr[i];
                                if (swap) temp = SwapLong(temp);
                                temp = (temp >> 1) + Pa_TriangularDither();
                                temp = temp >> 15;
                                temp = (short) ClipShort(temp);
                                *userBufPtr = (short)temp;
                                userBufPtr += NumInputChannels;
                        }
                }
        
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
// MUST BE TESTED
static void Input_Float32_Int16 (ASIOBufferInfo* nativeBuffer, short *inBufPtr, int framePerBuffer, int NumInputChannels, int index, int hostFrameOffset,int userFrameOffset,uint32 flags,bool swap)
{
        unsigned long temp;
        int i,j;
        
        if( flags & paDitherOff )
        {
                for( j=0; j<NumInputChannels; j++ ) {
                        unsigned long *asioBufPtr = &((unsigned long*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                        short *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                        for (i= 0; i < framePerBuffer; i++)
                        { 
                                temp = asioBufPtr[i];
                                if (swap) temp = SwapLong(temp);
                                *userBufPtr = (short)((float)temp * MAX_INT16_FP); // Is temp a value between -1.0 and 1.0 ??
                                userBufPtr += NumInputChannels;
                        }
                }
        }
        else
        {
                for( j=0; j<NumInputChannels; j++ ) {
                        unsigned long *asioBufPtr = &((unsigned long*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                        short *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                        for (i= 0; i < framePerBuffer; i++)
                        { 
                                float dither  = Pa_TriangularDither()*DITHER_SCALE;
                                temp = asioBufPtr[i];
                                if (swap) temp = SwapLong(temp);
                                temp = (short)(((float)temp * MAX_INT16_FP) + dither);
                                temp = ClipShort(temp);
                                *userBufPtr = (short)temp;
                                userBufPtr += NumInputChannels;
                        }
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Input_Int16_Int8 (ASIOBufferInfo* nativeBuffer, char *inBufPtr, int framePerBuffer, int NumInputChannels, int index, int hostFrameOffset,int userFrameOffset, uint32 flags,bool swap)
{
        long temp;
        int i,j;
        
        if( flags & paDitherOff )
        {
                for( j=0; j<NumInputChannels; j++ ) {
                        short *asioBufPtr = &((short*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                        char *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                        for (i= 0; i < framePerBuffer; i++)
                        { 
                                temp = asioBufPtr[i];
                                if (swap) temp = SwapShort(temp);
                                *userBufPtr = (char)(temp>>8);
                                userBufPtr += NumInputChannels;
                        }
                }
        }
        else
        {
                for( j=0; j<NumInputChannels; j++ ) {
                        short *asioBufPtr = &((short*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                        char *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                        for (i= 0; i < framePerBuffer; i++)
                        { 
                                temp = asioBufPtr[i];
                                if (swap) temp = SwapShort(temp);
                                temp += Pa_TriangularDither() >> 8;
                                temp = ClipShort(temp);
                                *userBufPtr = (char)(temp>>8);
                                userBufPtr += NumInputChannels;
                        }
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Input_Int32_Int8 (ASIOBufferInfo* nativeBuffer, char *inBufPtr, int framePerBuffer, int NumInputChannels, int index, int hostFrameOffset, int userFrameOffset, uint32 flags,bool swap)
{
        long temp;
        int i,j;
        
        if( flags & paDitherOff )
        {
                for( j=0; j<NumInputChannels; j++ ) {
                        long *asioBufPtr = &((long*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                        char *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                        for (i= 0; i < framePerBuffer; i++)
                        { 
                                temp = asioBufPtr[i];
                                if (swap) temp = SwapLong(temp);
                                *userBufPtr = (char)(temp>>24);
                                userBufPtr += NumInputChannels;
                        }
                }
        }
        else
        {
                for( j=0; j<NumInputChannels; j++ ) {
                        long *asioBufPtr = &((long*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                        char *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                        for (i= 0; i < framePerBuffer; i++)
                        { 
                                temp = asioBufPtr[i];
                                if (swap) temp = SwapLong(temp);
                                temp = temp>>16;  // Shift to get a 16 bit value, then use the 16 bits to 8 bits code (MUST BE CHECHED)
                                temp += Pa_TriangularDither() >> 8;
                                temp = ClipShort(temp);
                                *userBufPtr = (char)(temp >> 8);
                                userBufPtr += NumInputChannels;
                        }
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
// MUST BE TESTED

static void Input_Float32_Int8 (ASIOBufferInfo* nativeBuffer, char *inBufPtr, int framePerBuffer, int NumInputChannels, int index, int hostFrameOffset,int userFrameOffset,  uint32 flags,bool swap)
{
        unsigned long temp;
        int i,j;
        
        if( flags & paDitherOff )
        {
                for( j=0; j<NumInputChannels; j++ ) {
                        unsigned long *asioBufPtr = &((unsigned long*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                        char *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                        for (i= 0; i < framePerBuffer; i++)
                        { 
                                temp = asioBufPtr[i];
                                if (swap) temp = SwapLong(temp);
                                *userBufPtr = (char)((float)temp*MAX_INT8_FP); // Is temp a value between -1.0 and 1.0 ??
                                userBufPtr += NumInputChannels;
                        }
                }
        }
        else
        {
                for( j=0; j<NumInputChannels; j++ ) {
                        unsigned long *asioBufPtr = &((unsigned long*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                        char *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                        for (i= 0; i < framePerBuffer; i++)
                        { 
                                float dither  = Pa_TriangularDither()*DITHER_SCALE;
                                temp = asioBufPtr[i];
                                if (swap) temp = SwapLong(temp);
                                temp = (char)(((float)temp * MAX_INT8_FP) + dither);
                                temp = ClipChar(temp);
                                *userBufPtr = (char)temp;
                                userBufPtr += NumInputChannels;
                        }
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Input_Int16_IntU8 (ASIOBufferInfo* nativeBuffer, unsigned char *inBufPtr, int framePerBuffer, int NumInputChannels, int index, int hostFrameOffset,int userFrameOffset, uint32 flags,bool swap)
{
        long temp;
        int i,j;

        if( flags & paDitherOff )
        {
                for( j=0; j<NumInputChannels; j++ ) {
                        short *asioBufPtr = &((short*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                        unsigned char *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                        for (i= 0; i < framePerBuffer; i++)
                        { 
                                temp = asioBufPtr[i];
                                if (swap) temp = SwapShort(temp);
                                *userBufPtr = (unsigned char)((temp>>8) + 0x80); 
                                userBufPtr += NumInputChannels;
                        }
                }
        }
        else
        {
                for( j=0; j<NumInputChannels; j++ ) {
                        short *asioBufPtr = &((short*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                        unsigned char *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                        for (i= 0; i < framePerBuffer; i++)
                        { 
                                temp = asioBufPtr[i];
                                if (swap) temp = SwapShort(temp);
                                temp += Pa_TriangularDither() >> 8;
                                temp = ClipShort(temp);
                                *userBufPtr = (unsigned char)((temp>>8) + 0x80); 
                                userBufPtr += NumInputChannels;
                        }
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Input_Int32_IntU8 (ASIOBufferInfo* nativeBuffer, unsigned char *inBufPtr, int framePerBuffer, int NumInputChannels, int index, int hostFrameOffset, int userFrameOffset,uint32 flags,bool swap)
{
        long temp;
        int i,j;
        
        if( flags & paDitherOff )
        {
                for( j=0; j<NumInputChannels; j++ ) {
                        long *asioBufPtr = &((long*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                        unsigned char *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                        for (i= 0; i < framePerBuffer; i++)
                        { 
                                temp = asioBufPtr[i];
                                if (swap) temp = SwapLong(temp);
                                *userBufPtr = (unsigned char)((temp>>24) + 0x80); 
                                userBufPtr += NumInputChannels;
                        }
                }
        }
        else
        {
                for( j=0; j<NumInputChannels; j++ ) {
                        long *asioBufPtr = &((long*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                        unsigned char *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                        for (i= 0; i < framePerBuffer; i++)
                        { 
                                temp = asioBufPtr[i];
                                if (swap) temp = SwapLong(temp);
                                temp = temp>>16; // Shift to get a 16 bit value, then use the 16 bits to 8 bits code (MUST BE CHECHED)
                                temp += Pa_TriangularDither() >> 8;
                                temp = ClipShort(temp);
                                *userBufPtr = (unsigned char)((temp>>8) + 0x80); 
                                userBufPtr += NumInputChannels;
                        }
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
// MUST BE TESTED

static void Input_Float32_IntU8 (ASIOBufferInfo* nativeBuffer, unsigned char *inBufPtr, int framePerBuffer, int NumInputChannels, int index, int hostFrameOffset,int userFrameOffset, uint32 flags,bool swap)
{
        unsigned long temp;
        int i,j;
        
        if( flags & paDitherOff )
        {
                for( j=0; j<NumInputChannels; j++ ) {
                        unsigned long *asioBufPtr = &((unsigned long*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                        unsigned char *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                        for (i= 0; i < framePerBuffer; i++)
                        { 
                                temp = asioBufPtr[i];
                                if (swap) temp = SwapLong(temp);
                                *userBufPtr = (unsigned char)(((float)temp*MAX_INT8_FP) + 0x80);
                                userBufPtr += NumInputChannels;
                        }
                }
        }
        else
        {       
                for( j=0; j<NumInputChannels; j++ ) {
                        unsigned long *asioBufPtr = &((unsigned long*)nativeBuffer[j].buffers[index])[hostFrameOffset];
                        unsigned char *userBufPtr = &inBufPtr[j+(userFrameOffset*NumInputChannels)];
                        for (i= 0; i < framePerBuffer; i++)
                        { 
                                float dither  = Pa_TriangularDither()*DITHER_SCALE;
                                temp = asioBufPtr[i];
                                if (swap) temp = SwapLong(temp);
                                temp = (char)(((float)temp * MAX_INT8_FP) + dither);
                                temp = ClipChar(temp);
                                *userBufPtr =  (unsigned char)(temp + 0x80); 
                                userBufPtr += NumInputChannels;
                        }
                }
        }
}

                
// OUPUT 
//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Output_Float32_Int16 (ASIOBufferInfo* nativeBuffer, float *outBufPtr, int framePerBuffer, int NumInputChannels, int NumOuputChannels, int index, int hostFrameOffset, int userFrameOffset,uint32 flags, bool swap)
{
        long temp;
        int i,j;

        if( flags & paDitherOff )
                {
                        if( flags & paClipOff ) /* NOTHING */
                        {
                                for( j=0; j<NumOuputChannels; j++ ) {
                                        short *asioBufPtr = &((short*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                                        float *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                                        
                                        for (i= 0; i < framePerBuffer; i++) 
                                        {
                                                temp = (short) (*userBufPtr * MAX_INT16_FP);
                                                if (swap) temp = SwapShort(temp);
                                                asioBufPtr[i] = (short)temp;
                                                userBufPtr += NumOuputChannels;
                                        }
                                }
                        }
                        else /* CLIP */
                        {
                                for( j=0; j<NumOuputChannels; j++ ) {
                                        short *asioBufPtr = &((short*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                                        float *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                                        
                                        for (i= 0; i < framePerBuffer; i++) 
                                        {
                                                temp = (long) (*userBufPtr * MAX_INT16_FP);
                                                temp = ClipShort(temp);
                                                if (swap) temp = SwapShort(temp);
                                                asioBufPtr[i] = (short)temp;
                                                userBufPtr += NumOuputChannels;
                                        }
                                }
                        }
                }
                else
                {
                        /* If you dither then you have to clip because dithering could push the signal out of range! */
                        for( j=0; j<NumOuputChannels; j++ ) {
                                short *asioBufPtr = &((short*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                                float *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                                
                                for (i= 0; i < framePerBuffer; i++) 
                                {
                                        float dither = Pa_TriangularDither()*DITHER_SCALE;
                                        temp = (long) ((*userBufPtr * MAX_INT16_FP) + dither);
                                        temp = ClipShort(temp);
                                        if (swap) temp = SwapShort(temp);
                                        asioBufPtr[i] = (short)temp;
                                        userBufPtr += NumOuputChannels;
                                }
                        }
                }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Output_Float32_Int32 (ASIOBufferInfo* nativeBuffer, float *outBufPtr, int framePerBuffer, int NumInputChannels, int NumOuputChannels, int index, int hostFrameOffset, int userFrameOffset,uint32 flags,bool swap)
{
        long temp;
        int i,j;
        
        if( flags & paClipOff )
        {
                for (j= 0; j < NumOuputChannels; j++) 
                {
                        long *asioBufPtr = &((long*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                        float *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                        for( i=0; i<framePerBuffer; i++ )
                        {
                                temp = (long) (*userBufPtr * MAX_INT32_FP);
                                if (swap) temp = SwapLong(temp);
                                asioBufPtr[i] = temp;
                                userBufPtr += NumOuputChannels;
                        }
                }
        }
        else // CLIP *
        {
                for (j= 0; j < NumOuputChannels; j++) 
                {
                        long *asioBufPtr = &((long*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                        float *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                        for( i=0; i<framePerBuffer; i++ )
                        {
                                float temp1 = *userBufPtr;
                                temp1 = ClipFloat(temp1);
                                temp = (long) (temp1*MAX_INT32_FP);
                                if (swap) temp = SwapLong(temp);
                                asioBufPtr[i] = temp;
                                userBufPtr += NumOuputChannels;
                        }
                }
        }
        
}


//-------------------------------------------------------------------------------------------------------------------------------------------------------
// MUST BE TESTED

 static void Output_Float32_Float32 (ASIOBufferInfo* nativeBuffer, float *outBufPtr, int framePerBuffer, int NumInputChannels, int NumOuputChannels, int index, int hostFrameOffset, int userFrameOffset,uint32 flags,bool swap)
{
        long temp;
        int i,j;
        
        if( flags & paClipOff )
        {
                for (j= 0; j < NumOuputChannels; j++) 
                {
                        float *asioBufPtr = &((float*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                        float *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                        for( i=0; i<framePerBuffer; i++ )
                        {
                                temp = (long) *userBufPtr;
                                if (swap) temp = SwapLong(temp);
                                asioBufPtr[i] = (float)temp;
                                userBufPtr += NumOuputChannels;
                        }
                }
                
        }
        else /* CLIP */
        {
                for (j= 0; j < NumOuputChannels; j++) 
                {
                        float *asioBufPtr = &((float*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                        float *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                        for( i=0; i<framePerBuffer; i++ )
                        {
                                float temp1 = *userBufPtr;
                                temp1 = ClipFloat(temp1);  // Is is necessary??
                                temp = (long) temp1;
                                if (swap) temp = SwapLong(temp);
                                asioBufPtr[i] = (float)temp;
                                userBufPtr += NumOuputChannels;
                        }
                }
        }
        
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------                                       
static void Output_Int32_Int16(ASIOBufferInfo* nativeBuffer, long *outBufPtr, int framePerBuffer, int NumInputChannels, int NumOuputChannels, int index, int hostFrameOffset,int userFrameOffset,uint32 flags,bool swap)
{
        long temp;
        int i,j;
        
        if( flags & paDitherOff )
        {
                for (j= 0; j < NumOuputChannels; j++) 
                {
                        short *asioBufPtr = &((short*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                        long *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                        for( i=0; i<framePerBuffer; i++ )
                        {
                                temp = (short) ((*userBufPtr) >> 16);
                                if (swap) temp = SwapShort(temp);
                                asioBufPtr[i] = (short)temp;
                                userBufPtr += NumOuputChannels;
                        }
                }
        }
        else
        {
                for (j= 0; j < NumOuputChannels; j++) 
                {
                        short *asioBufPtr = &((short*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                        long *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                        for( i=0; i<framePerBuffer; i++ )
                        {
                                temp = (*userBufPtr >> 1) + Pa_TriangularDither();
                                temp = temp >> 15;
                                temp = (short) ClipShort(temp);
                                if (swap) temp = SwapShort(temp);
                                asioBufPtr[i] = (short)temp;
                                userBufPtr += NumOuputChannels;
                        }
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Output_Int32_Int32(ASIOBufferInfo* nativeBuffer, long *outBufPtr, int framePerBuffer, int NumInputChannels, int NumOuputChannels, int index, int hostFrameOffset,int userFrameOffset,uint32 flags,bool swap)
{
        long temp;
        int i,j;
        
        for (j= 0; j < NumOuputChannels; j++) 
        {
                long *asioBufPtr = &((long*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                long *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                for( i=0; i<framePerBuffer; i++ )
                {
                        temp = *userBufPtr;
                        if (swap) temp = SwapLong(temp);
                        asioBufPtr[i] = temp;
                        userBufPtr += NumOuputChannels;
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
// MUST BE CHECKED

static void Output_Int32_Float32(ASIOBufferInfo* nativeBuffer, long *outBufPtr, int framePerBuffer, int NumInputChannels, int NumOuputChannels, int index, int hostFrameOffset,int userFrameOffset,uint32 flags,bool swap)
{
        long temp;
        int i,j;
        
        for (j= 0; j < NumOuputChannels; j++) 
        {
                float *asioBufPtr = &((float*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                long *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                for( i=0; i<framePerBuffer; i++ )
                {
                        temp = *userBufPtr;
                        if (swap) temp = SwapLong(temp);
                        asioBufPtr[i] = ((float)temp) * (1.0f / MAX_INT32_FP);
                        userBufPtr += NumOuputChannels;
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Output_Int16_Int16(ASIOBufferInfo* nativeBuffer, short *outBufPtr, int framePerBuffer, int NumInputChannels, int NumOuputChannels, int index, int hostFrameOffset, int userFrameOffset,bool swap)
{
        long temp;
        int i,j;

        for (j= 0; j < NumOuputChannels; j++) 
        {
                short *asioBufPtr = &((short*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                short *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                for( i=0; i<framePerBuffer; i++ )
                {
                        temp = *userBufPtr;
                        if (swap) temp = SwapShort(temp);
                        asioBufPtr[i] = (short)temp;
                        userBufPtr += NumOuputChannels;
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Output_Int16_Int32(ASIOBufferInfo* nativeBuffer, short *outBufPtr, int framePerBuffer, int NumInputChannels, int NumOuputChannels, int index, int hostFrameOffset,int userFrameOffset, bool swap)
{
        long temp;
        int i,j;
        
        for (j= 0; j < NumOuputChannels; j++) 
        {
                long *asioBufPtr = &((long*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                short *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                for( i=0; i<framePerBuffer; i++ )
                {
                        temp = (*userBufPtr)<<16;
                        if (swap) temp = SwapLong(temp);
                        asioBufPtr[i] = temp;
                        userBufPtr += NumOuputChannels;
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
// MUST BE CHECKED
static void Output_Int16_Float32(ASIOBufferInfo* nativeBuffer, short *outBufPtr, int framePerBuffer, int NumInputChannels, int NumOuputChannels, int index, int hostFrameOffset,int userFrameOffset, bool swap)
{
        long temp;
        int i,j;
        
        for (j= 0; j < NumOuputChannels; j++) 
        {
                float *asioBufPtr = &((float*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                short *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                for( i=0; i<framePerBuffer; i++ )
                {
                        temp = *userBufPtr;
                        asioBufPtr[i] = ((float)temp) * (1.0f / MAX_INT16_FP);
                        userBufPtr += NumOuputChannels;
                }
        }
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Output_Int8_Int16(ASIOBufferInfo* nativeBuffer, char *outBufPtr, int framePerBuffer, int NumInputChannels, int NumOuputChannels, int index, int hostFrameOffset,int userFrameOffset, bool swap)
{
        long temp;
        int i,j;

        for (j= 0; j < NumOuputChannels; j++) 
        {
                short *asioBufPtr = &((short*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                char *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                for( i=0; i<framePerBuffer; i++ )
                {
                        temp = (short)(*userBufPtr)<<8;
                        if (swap) temp = SwapShort(temp);
                        asioBufPtr[i] = (short)temp;
                        userBufPtr += NumOuputChannels;
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Output_Int8_Int32(ASIOBufferInfo* nativeBuffer, char *outBufPtr, int framePerBuffer, int NumInputChannels, int NumOuputChannels, int index, int hostFrameOffset,int userFrameOffset, bool swap)
{
        long temp;
        int i,j;

        for (j= 0; j < NumOuputChannels; j++) 
        {
                long *asioBufPtr = &((long*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                char *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                for( i=0; i<framePerBuffer; i++ )
                {
                        temp = (short)(*userBufPtr)<<24;
                        if (swap) temp = SwapLong(temp);
                        asioBufPtr[i] = temp;
                        userBufPtr += NumOuputChannels;
                }
        }
}


//-------------------------------------------------------------------------------------------------------------------------------------------------------
// MUST BE CHECKED
static void Output_Int8_Float32(ASIOBufferInfo* nativeBuffer, char *outBufPtr, int framePerBuffer, int NumInputChannels, int NumOuputChannels, int index, int hostFrameOffset,int userFrameOffset, bool swap)
{
        long temp;
        int i,j;
        
        for (j= 0; j < NumOuputChannels; j++) 
        {
                long *asioBufPtr = &((long*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                char *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                for( i=0; i<framePerBuffer; i++ )
                {
                        temp = *userBufPtr;
                        asioBufPtr[i] = (long)(((float)temp) * (1.0f / MAX_INT8_FP));
                        userBufPtr += NumOuputChannels;
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Output_IntU8_Int16(ASIOBufferInfo* nativeBuffer, unsigned char *outBufPtr, int framePerBuffer, int NumInputChannels, int NumOuputChannels, int index, int hostFrameOffset,int userFrameOffset, bool swap)
{
        long temp;
        int i,j;

        for (j= 0; j < NumOuputChannels; j++) 
        {
                short *asioBufPtr = &((short*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                unsigned char *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                for( i=0; i<framePerBuffer; i++ )
                {
                        temp = ((short)((*userBufPtr) - 0x80)) << 8;
                        if (swap) temp = SwapShort(temp);
                        asioBufPtr[i] = (short)temp;
                        userBufPtr += NumOuputChannels;
                }
        }
}       

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Output_IntU8_Int32(ASIOBufferInfo* nativeBuffer, unsigned char *outBufPtr, int framePerBuffer, int NumInputChannels, int NumOuputChannels, int index, int hostFrameOffset,int userFrameOffset, bool swap)
{
        long temp;
        int i,j;
        
        for (j= 0; j < NumOuputChannels; j++) 
        {
                long *asioBufPtr = &((long*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                unsigned char *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                for( i=0; i<framePerBuffer; i++ )
                {
                        temp = ((short)((*userBufPtr) - 0x80)) << 24;
                        if (swap) temp = SwapLong(temp);
                        asioBufPtr[i] = temp;
                        userBufPtr += NumOuputChannels;
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
// MUST BE CHECKED

static void Output_IntU8_Float32(ASIOBufferInfo* nativeBuffer, unsigned char *outBufPtr, int framePerBuffer, int NumInputChannels, int NumOuputChannels, int index, int hostFrameOffset,int userFrameOffset, bool swap)
{
        long temp;
        int i,j;
        
        for (j= 0; j < NumOuputChannels; j++) 
        {
                float *asioBufPtr = &((float*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                unsigned char *userBufPtr = &outBufPtr[j+(userFrameOffset*NumOuputChannels)];
                for( i=0; i<framePerBuffer; i++ )
                {
                        temp = ((short)((*userBufPtr) - 0x80)) << 24;
                        asioBufPtr[i] = ((float)temp) * (1.0f / MAX_INT32_FP);
                        userBufPtr += NumOuputChannels;
                }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Pa_ASIO_Clear_Output_16 (ASIOBufferInfo* nativeBuffer, long frames, long NumInputChannels, long NumOuputChannels, long index, long hostFrameOffset)
{
        int i,j;

        for( j=0; j<NumOuputChannels; j++ ) {
                short *asioBufPtr = &((short*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                for (i= 0; i < frames; i++) {asioBufPtr[i] = 0; }
        }
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Pa_ASIO_Clear_Output_32 (ASIOBufferInfo* nativeBuffer, long frames, long NumInputChannels, long NumOuputChannels, long index, long hostFrameOffset)
{
        int i,j;

        for( j=0; j<NumOuputChannels; j++ ) {
                long *asioBufPtr = &((long*)nativeBuffer[j+NumInputChannels].buffers[index])[hostFrameOffset];
                for (i= 0; i < frames; i++) {asioBufPtr[i] = 0; }
        }
}


//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Pa_ASIO_Adaptor_Init()
{
	if (asioDriverInfo.past->past_FramesPerUserBuffer <= asioDriverInfo.past_FramesPerHostBuffer) {
		asioDriverInfo.pahsc_hostOutputBufferFrameOffset = asioDriverInfo.pahsc_OutputBufferOffset;
		asioDriverInfo.pahsc_userInputBufferFrameOffset = 0; // empty 
		asioDriverInfo.pahsc_userOutputBufferFrameOffset = asioDriverInfo.past->past_FramesPerUserBuffer; // empty 
		DBUG(("Pa_ASIO_Adaptor_Init : shift output\n"));
		DBUG(("Pa_ASIO_Adaptor_Init : userInputBufferFrameOffset %d\n",asioDriverInfo.pahsc_userInputBufferFrameOffset));
		DBUG(("Pa_ASIO_Adaptor_Init : userOutputBufferFrameOffset %d\n",asioDriverInfo.pahsc_userOutputBufferFrameOffset));
		DBUG(("Pa_ASIO_Adaptor_Init : hostOutputBufferFrameOffset %d\n",asioDriverInfo.pahsc_hostOutputBufferFrameOffset));

	}else {
		asioDriverInfo.pahsc_hostOutputBufferFrameOffset = 0; // empty 
		asioDriverInfo.pahsc_userInputBufferFrameOffset = asioDriverInfo.pahsc_InputBufferOffset;
		asioDriverInfo.pahsc_userOutputBufferFrameOffset = asioDriverInfo.past->past_FramesPerUserBuffer;	// empty 
		DBUG(("Pa_ASIO_Adaptor_Init : shift input\n"));
		DBUG(("Pa_ASIO_Adaptor_Init : userInputBufferFrameOffset %d\n",asioDriverInfo.pahsc_userInputBufferFrameOffset));
		DBUG(("Pa_ASIO_Adaptor_Init : userOutputBufferFrameOffset %d\n",asioDriverInfo.pahsc_userOutputBufferFrameOffset));
		DBUG(("Pa_ASIO_Adaptor_Init : hostOutputBufferFrameOffset %d\n",asioDriverInfo.pahsc_hostOutputBufferFrameOffset));
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
// FIXME : optimization for Input only or output only modes (really necessary ??)
static void Pa_ASIO_Callback_Input(long index)
{
        internalPortAudioStream  *past = asioDriverInfo.past;
        long framesInputHostBuffer = asioDriverInfo.past_FramesPerHostBuffer; // number of frames available into the host input buffer
		long framesInputUserBuffer;		// number of frames needed to complete the user input buffer
    	long framesOutputHostBuffer;  	// number of frames needed to complete the host output buffer
    	long framesOuputUserBuffer;		// number of frames available into the user output buffer
    	long userResult;
        long tmp;
        
         /* Fill host ASIO output with remaining frames in user output */
       	framesOutputHostBuffer = asioDriverInfo.past_FramesPerHostBuffer - asioDriverInfo.pahsc_hostOutputBufferFrameOffset;
        framesOuputUserBuffer = asioDriverInfo.past->past_FramesPerUserBuffer - asioDriverInfo.pahsc_userOutputBufferFrameOffset;
        tmp = min(framesOutputHostBuffer, framesOuputUserBuffer);
        framesOutputHostBuffer -= tmp;
        Pa_ASIO_Callback_Output(index,tmp);
        
        /* Available frames in hostInputBuffer */
        while (framesInputHostBuffer > 0) {
                
                /* Number of frames needed to complete an user input buffer */
                framesInputUserBuffer = asioDriverInfo.past->past_FramesPerUserBuffer - asioDriverInfo.pahsc_userInputBufferFrameOffset;
                                
                if (framesInputHostBuffer >= framesInputUserBuffer) {
                
                        /* Convert ASIO input to user input */
                        Pa_ASIO_Convert_Inter_Input (asioDriverInfo.bufferInfos, 
                                                    past->past_InputBuffer, 
                                                    asioDriverInfo.pahsc_NumInputChannels ,
                                					asioDriverInfo.pahsc_NumOutputChannels,
                               						framesInputUserBuffer,
                                					asioDriverInfo.past_FramesPerHostBuffer - framesInputHostBuffer,
                                					asioDriverInfo.pahsc_userInputBufferFrameOffset,
                                					asioDriverInfo.pahsc_channelInfos[0].type,
                                					past->past_InputSampleFormat,
                                					past->past_Flags,
                                					index);
                        
                        /* Call PortAudio callback */
                        userResult = asioDriverInfo.past->past_Callback(past->past_InputBuffer, past->past_OutputBuffer,
                                past->past_FramesPerUserBuffer,past->past_FrameCount,past->past_UserData );
               
		                /* User callback has asked us to stop in the middle of the host buffer  */
		                if( userResult != 0) {
		            
		                    /* Put 0 in the end of the output buffer */
		                     Pa_ASIO_Clear_Output(asioDriverInfo.bufferInfos, 
		                            	asioDriverInfo.pahsc_channelInfos[0].type,
		                            	asioDriverInfo.pahsc_NumInputChannels ,
		                            	asioDriverInfo.pahsc_NumOutputChannels,
		                            	index, 
		                            	asioDriverInfo.pahsc_hostOutputBufferFrameOffset, 
		                            	asioDriverInfo.past_FramesPerHostBuffer - asioDriverInfo.pahsc_hostOutputBufferFrameOffset);
		                    
		                    past->past_StopSoon = 1; 
		                    return;
		            	}
		                
		                
		                /* Full user ouput buffer : write offset */
		                asioDriverInfo.pahsc_userOutputBufferFrameOffset = 0;
		                
		                /*  Empty user input buffer : read offset */
		                asioDriverInfo.pahsc_userInputBufferFrameOffset = 0;
		                
		                /*  Fill host ASIO output  */
                        tmp = min (past->past_FramesPerUserBuffer,framesOutputHostBuffer);
                        Pa_ASIO_Callback_Output(index,tmp);
                        
                        framesOutputHostBuffer -= tmp;
                        framesInputHostBuffer -= framesInputUserBuffer;
                
                }else {
                
                        /* Convert ASIO input to user input */
                        Pa_ASIO_Convert_Inter_Input (asioDriverInfo.bufferInfos, 
                                                    past->past_InputBuffer, 
                                                    asioDriverInfo.pahsc_NumInputChannels ,
                                					asioDriverInfo.pahsc_NumOutputChannels,
                                					framesInputHostBuffer,
                                					asioDriverInfo.past_FramesPerHostBuffer - framesInputHostBuffer,
                                					asioDriverInfo.pahsc_userInputBufferFrameOffset,
                                					asioDriverInfo.pahsc_channelInfos[0].type,
                                					past->past_InputSampleFormat,
                                					past->past_Flags,
                                					index);
                        
                        /* Update pahsc_userInputBufferFrameOffset */
                        asioDriverInfo.pahsc_userInputBufferFrameOffset += framesInputHostBuffer;
                        
                        /* Update framesInputHostBuffer */
                        framesInputHostBuffer = 0; 
                }               
        }

}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Pa_ASIO_Callback_Output(long index, long framePerBuffer)
{
        internalPortAudioStream *past = asioDriverInfo.past;
        
        if (framePerBuffer > 0) {
                
                /* Convert user output to ASIO ouput */
                Pa_ASIO_Convert_Inter_Output (asioDriverInfo.bufferInfos, 
                                            past->past_OutputBuffer,
                                            asioDriverInfo.pahsc_NumInputChannels,
                                        	asioDriverInfo.pahsc_NumOutputChannels,
                                        	framePerBuffer,
                                        	asioDriverInfo.pahsc_hostOutputBufferFrameOffset,
                                        	asioDriverInfo.pahsc_userOutputBufferFrameOffset,
                                        	asioDriverInfo.pahsc_channelInfos[0].type,
                                        	past->past_InputSampleFormat,
                                        	past->past_Flags,
                                        	index);
                
                /* Update hostOuputFrameOffset */
                asioDriverInfo.pahsc_hostOutputBufferFrameOffset += framePerBuffer;

                /* Update userOutputFrameOffset */
                asioDriverInfo.pahsc_userOutputBufferFrameOffset += framePerBuffer;
        }
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Pa_ASIO_Callback_End()
 {
 	 /* Empty ASIO ouput : write offset */
     asioDriverInfo.pahsc_hostOutputBufferFrameOffset = 0;
 }

//-------------------------------------------------------------------------------------------------------------------------------------------------------
static void Pa_ASIO_Clear_User_Buffers()
{
	if( asioDriverInfo.past->past_InputBuffer != NULL )
	{
		memset( asioDriverInfo.past->past_InputBuffer, 0, asioDriverInfo.past->past_InputBufferSize );
	}
	if( asioDriverInfo.past->past_OutputBuffer != NULL )
	{
		memset( asioDriverInfo.past->past_OutputBuffer, 0, asioDriverInfo.past->past_OutputBufferSize );
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------
 static void Pa_ASIO_Clear_Output(ASIOBufferInfo* nativeBuffer, 
        ASIOSampleType nativeFormat,
        long NumInputChannels, 
        long NumOuputChannels,
        long index, 
        long hostFrameOffset, 
        long frames)
{
        
        switch (nativeFormat) {
        
                case ASIOSTInt16MSB:
                case ASIOSTInt16LSB:
                case ASIOSTInt32MSB16:
                case ASIOSTInt32LSB16:
                        Pa_ASIO_Clear_Output_16(nativeBuffer, frames,  NumInputChannels, NumOuputChannels, index, hostFrameOffset);
                        break;
                        
                case ASIOSTFloat64MSB:
                case ASIOSTFloat64LSB:
                        break;
                        
                case ASIOSTFloat32MSB:
                case ASIOSTFloat32LSB:
                case ASIOSTInt32MSB:
                case ASIOSTInt32LSB:
                case ASIOSTInt32MSB18:          
                case ASIOSTInt32MSB20:          
                case ASIOSTInt32MSB24:          
                case ASIOSTInt32LSB18:          
                case ASIOSTInt32LSB20:          
                case ASIOSTInt32LSB24:          
                        Pa_ASIO_Clear_Output_32(nativeBuffer, frames,  NumInputChannels, NumOuputChannels, index, hostFrameOffset);
                        break;
                        
                case ASIOSTInt24MSB:
                case ASIOSTInt24LSB:
                        break;
                        
                default:
                        break;
        }
}


//---------------------------------------------------------------------------------------
static void Pa_ASIO_Convert_Inter_Input(
                ASIOBufferInfo* nativeBuffer, 
                void* inputBuffer,
        		long NumInputChannels, 
        		long NumOuputChannels,
        		long framePerBuffer,
        		long hostFrameOffset,
        		long userFrameOffset,
        		ASIOSampleType nativeFormat, 
        		PaSampleFormat paFormat, 
        		PaStreamFlags flags,
        		long index)
{
                
        if((NumInputChannels > 0) && (nativeBuffer != NULL))
        {
                /* Convert from native format to PA format. */
                switch(paFormat)
                {
                                case paFloat32:
                        {
                                float *inBufPtr = (float *) inputBuffer;
                             
                                switch (nativeFormat) {
                                        case ASIOSTInt16LSB:
                                                Input_Int16_Float32(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset, userFrameOffset, swap);
                                                break;  
                                        case ASIOSTInt16MSB:
                                                Input_Int16_Float32(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset, userFrameOffset,!swap);
                                                break;  
                                        case ASIOSTInt32LSB:
                                                Input_Int32_Float32(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset, userFrameOffset,swap);
                                                break;
                                        case ASIOSTInt32MSB:
                                                Input_Int32_Float32(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset, userFrameOffset,!swap);
                                                break;  
                                        case ASIOSTFloat32LSB:          // IEEE 754 32 bit float, as found on Intel x86 architecture
                                                Input_Float32_Float32(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset, userFrameOffset,swap);
                                                break;  
                                        case ASIOSTFloat32MSB:          // IEEE 754 32 bit float, as found on Intel x86 architecture
                                                Input_Float32_Float32(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset, userFrameOffset,!swap);
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
                                                DBUG(("Not yet implemented : please report the problem\n"));
                                                break;
                                }       
                                
                                break;
                        }
                        
                case paInt32:
                        {
                                long *inBufPtr = (long *)inputBuffer;
                                 
                                switch (nativeFormat) {
                                        case ASIOSTInt16LSB:
                                                Input_Int16_Int32(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, swap);
                                                break;
                                        case ASIOSTInt16MSB:
                                                Input_Int16_Int32(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, !swap);
                                                break;
                                        case ASIOSTInt32LSB:
                                                Input_Int32_Int32(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, swap);
                                                break;
                                        case ASIOSTInt32MSB:
                                                Input_Int32_Int32(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, !swap);
                                                break;
                                        case ASIOSTFloat32LSB:          // IEEE 754 32 bit float, as found on Intel x86 architecture
                                                Input_Float32_Int32(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, swap);
                                                break;  
                                        case ASIOSTFloat32MSB:          // IEEE 754 32 bit float, as found on Intel x86 architecture
                                                Input_Float32_Int32(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, !swap);
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
                                                DBUG(("Not yet implemented : please report the problem\n"));
                                                break;
                                        
                                }
                                break;
                        }
                        
                case paInt16:
                        {
                                short *inBufPtr = (short *) inputBuffer;
                                 
                                switch (nativeFormat) {
                                        case ASIOSTInt16LSB:
                                                Input_Int16_Int16(nativeBuffer, inBufPtr, framePerBuffer , NumInputChannels, index , hostFrameOffset,userFrameOffset, swap);
                                                break;
                                        case ASIOSTInt16MSB:
                                                Input_Int16_Int16(nativeBuffer, inBufPtr, framePerBuffer , NumInputChannels, index , hostFrameOffset,userFrameOffset, !swap);
                                                break;
                                        case ASIOSTInt32LSB:
                                                Input_Int32_Int16(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, flags,swap);
                                                break;
                                        case ASIOSTInt32MSB:
                                                Input_Int32_Int16(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, flags,!swap);
                                                break;
                                        case ASIOSTFloat32LSB:          // IEEE 754 32 bit float, as found on Intel x86 architecture
                                                Input_Float32_Int16(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, flags,swap);
                                                break;  
                                        case ASIOSTFloat32MSB:          // IEEE 754 32 bit float, as found on Intel x86 architecture
                                                Input_Float32_Int16(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, flags,!swap);
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
                                                DBUG(("Not yet implemented : please report the problem\n"));
                                                break;
                        
                                }
                                break;
                        }

                case paInt8:
                        {
                                /* Convert 16 bit data to 8 bit chars */
                                
                                char *inBufPtr = (char *) inputBuffer;
                                
                                switch (nativeFormat) {
                                        case ASIOSTInt16LSB:
                                                Input_Int16_Int8(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset,flags,swap);
                                                break;  
                                        case ASIOSTInt16MSB:
                                                Input_Int16_Int8(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, flags,!swap);
                                                break;  
                                        case ASIOSTInt32LSB:
                                                Input_Int32_Int8(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, flags,swap);
                                                break;
                                        case ASIOSTInt32MSB:
                                                Input_Int32_Int8(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, flags,!swap);
                                                break;
                                        case ASIOSTFloat32LSB:          // IEEE 754 32 bit float, as found on Intel x86 architecture
                                                Input_Float32_Int8(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, flags,swap);
                                                break;  
                                        case ASIOSTFloat32MSB:          // IEEE 754 32 bit float, as found on Intel x86 architecture
                                                Input_Float32_Int8(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, flags,!swap);
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
                                                DBUG(("Not yet implemented : please report the problem\n"));
                                                break;  
                                }       
                                break;
                        }

                case paUInt8:
                        {
                                /* Convert 16 bit data to 8 bit unsigned chars */
                                
                                unsigned char *inBufPtr = (unsigned char *)inputBuffer;
                                 
                                switch (nativeFormat) {
                                        case ASIOSTInt16LSB:
                                                Input_Int16_IntU8(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, flags,swap);
                                                break;  
                                        case ASIOSTInt16MSB:
                                                Input_Int16_IntU8(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, flags,!swap);
                                                break;  
                                        case ASIOSTInt32LSB:
                                                Input_Int32_IntU8(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset,flags,swap);
                                                break;
                                        case ASIOSTInt32MSB:
                                                Input_Int32_IntU8(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset, flags,!swap);
                                                break;
                                        case ASIOSTFloat32LSB:          // IEEE 754 32 bit float, as found on Intel x86 architecture
                                                Input_Float32_IntU8(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset,flags,swap);
                                                break;  
                                        case ASIOSTFloat32MSB:          // IEEE 754 32 bit float, as found on Intel x86 architecture
                                                Input_Float32_IntU8(nativeBuffer, inBufPtr, framePerBuffer, NumInputChannels, index, hostFrameOffset,userFrameOffset,flags,!swap);
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
                                                DBUG(("Not yet implemented : please report the problem\n"));
                                                break;  
                                
                                }       
                                break;
                        }
                        
                default:
                        break;
                }
        }
}


//---------------------------------------------------------------------------------------
static void Pa_ASIO_Convert_Inter_Output(ASIOBufferInfo* nativeBuffer, 
                void* outputBuffer,
       			long NumInputChannels, 
       			long NumOuputChannels,
        		long framePerBuffer,
        		long hostFrameOffset,
        		long userFrameOffset,
        		ASIOSampleType nativeFormat, 
        		PaSampleFormat paFormat, 
        		PaStreamFlags flags,
        		long index)
{
   
        if((NumOuputChannels > 0) && (nativeBuffer != NULL)) 
        {
                /* Convert from PA format to native format */
                
                switch(paFormat)
                {
                        case paFloat32:
                                {
                                        float *outBufPtr = (float *) outputBuffer;
                                        
                                        switch (nativeFormat) {
                                                case ASIOSTInt16LSB:
                                                        Output_Float32_Int16(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset, userFrameOffset, flags, swap);
                                                        break;  
                                                case ASIOSTInt16MSB:
                                                        Output_Float32_Int16(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset, userFrameOffset, flags,!swap);
                                                        break;  
                                                case ASIOSTInt32LSB:
                                                        Output_Float32_Int32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset, userFrameOffset, flags,swap);
                                                        break;
                                                case ASIOSTInt32MSB:
                                                        Output_Float32_Int32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, flags,!swap);
                                                        break;  
                                                case ASIOSTFloat32LSB:
                                                        Output_Float32_Float32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset,flags,swap);
                                                        break;
                                                case ASIOSTFloat32MSB:
                                                        Output_Float32_Float32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, flags,!swap);
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
                                                        DBUG(("Not yet implemented : please report the problem\n"));
                                                        break;
                                        }       
                                        break;
                                }
                                
                        case paInt32:
                                {
                                        long *outBufPtr = (long *) outputBuffer;
                                        
                                        switch (nativeFormat) {
                                                case ASIOSTInt16LSB:
                                                        Output_Int32_Int16(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, flags,swap);
                                                        break;  
                                                case ASIOSTInt16MSB:
                                                        Output_Int32_Int16(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, flags,!swap);
                                                        break;  
                                                case ASIOSTInt32LSB:
                                                        Output_Int32_Int32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, flags,swap);
                                                        break;
                                                case ASIOSTInt32MSB:
                                                        Output_Int32_Int32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, flags,!swap);
                                                        break;  
                                                case ASIOSTFloat32LSB:
                                                        Output_Int32_Float32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, flags,swap);
                                                        break;
                                                case ASIOSTFloat32MSB:
                                                        Output_Int32_Float32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, flags,!swap);
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
                                                        DBUG(("Not yet implemented : please report the problem\n"));
                                                        break;
                                        }       
                                        break;
                                }
                                
                        case paInt16:
                                {
                                        short *outBufPtr = (short *) outputBuffer;
                                        
                                        switch (nativeFormat) {
                                                case ASIOSTInt16LSB:
                                                        Output_Int16_Int16(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, swap);
                                                        break;  
                                                case ASIOSTInt16MSB:
                                                        Output_Int16_Int16(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, !swap);
                                                        break;  
                                                case ASIOSTInt32LSB:
                                                        Output_Int16_Int32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, swap);
                                                        break;
                                                case ASIOSTInt32MSB:
                                                        Output_Int16_Int32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, !swap);
                                                        break;  
                                                case ASIOSTFloat32LSB:
                                                        Output_Int16_Float32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, swap);
                                                        break;
                                                case ASIOSTFloat32MSB:
                                                        Output_Int16_Float32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, !swap);
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
                                                        DBUG(("Not yet implemented : please report the problem\n"));
                                                        break;
                                
                                        }       
                                        break;
                                }


                        case paInt8:
                                {
                                        char *outBufPtr = (char *) outputBuffer;
                                        
                                        switch (nativeFormat) {
                                                case ASIOSTInt16LSB:
                                                        Output_Int8_Int16(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, swap);
                                                        break;  
                                                case ASIOSTInt16MSB:
                                                        Output_Int8_Int16(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, !swap);
                                                        break;  
                                                case ASIOSTInt32LSB:
                                                        Output_Int8_Int32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, swap);
                                                        break;
                                                case ASIOSTInt32MSB:
                                                        Output_Int8_Int32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, !swap);
                                                        break;  
                                                case ASIOSTFloat32LSB:
                                                        Output_Int8_Float32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, swap);
                                                        break;
                                                case ASIOSTFloat32MSB:
                                                        Output_Int8_Float32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, !swap);
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
                                                        DBUG(("Not yet implemented : please report the problem\n"));
                                                        break;
                                        }       
                                        break;
                                }

                        case paUInt8:
                                {
                                        unsigned char *outBufPtr = (unsigned char *) outputBuffer;
                                        
                                        switch (nativeFormat) {
                                                case ASIOSTInt16LSB:
                                                        Output_IntU8_Int16(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, swap);
                                                        break;  
                                                case ASIOSTInt16MSB:
                                                        Output_IntU8_Int16(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, !swap);
                                                        break;  
                                                case ASIOSTInt32LSB:
                                                        Output_IntU8_Int32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, swap);
                                                        break;
                                                case ASIOSTInt32MSB:
                                                        Output_IntU8_Int32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, !swap);
                                                        break;  
                                                case ASIOSTFloat32LSB:
                                                        Output_IntU8_Float32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, swap);
                                                        break;
                                                case ASIOSTFloat32MSB:
                                                        Output_IntU8_Float32(nativeBuffer, outBufPtr, framePerBuffer, NumInputChannels, NumOuputChannels, index, hostFrameOffset,userFrameOffset, !swap);
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
                                                        DBUG(("Not yet implemented : please report the problem\n"));
                                                        break;
                                        }       
                                        break;
                                }

                        default:
                                break;
                        }               
        }

}



/* Load a ASIO driver corresponding to the required device */
static PaError Pa_ASIO_loadDevice (long device) 
{
        PaDeviceInfo * dev = &(sDevices[device].pad_Info);

        if (!Pa_ASIO_loadAsioDriver((char *) dev->name)) return paHostError;
        if (ASIOInit(&asioDriverInfo.pahsc_driverInfo) != ASE_OK) return paHostError;
        if (ASIOGetChannels(&asioDriverInfo.pahsc_NumInputChannels, &asioDriverInfo.pahsc_NumOutputChannels) != ASE_OK) return paHostError;
        if (ASIOGetBufferSize(&asioDriverInfo.pahsc_minSize, &asioDriverInfo.pahsc_maxSize, &asioDriverInfo.pahsc_preferredSize, &asioDriverInfo.pahsc_granularity) != ASE_OK) return paHostError;
        
        if(ASIOOutputReady() == ASE_OK)
                asioDriverInfo.pahsc_postOutput = true;
        else
                asioDriverInfo.pahsc_postOutput = false;
                        
        return paNoError;
}

//---------------------------------------------------
static int GetHighestBitPosition (unsigned long n)
{
        int pos = -1;
        while( n != 0 )
        {
                pos++;
                n = n >> 1;
        }
        return pos;
}

//------------------------------------------------------------------------------------------
static int GetFirstMultiple(long min, long val ){  return ((min + val - 1) / val) * val; }

//------------------------------------------------------------------------------------------
static int GetFirstPossibleDivisor(long max, long val )
{ 
	for (int i = 2; i < 20; i++) {if (((val%i) == 0) && ((val/i) <= max)) return (val/i); }
	return val;
}

//------------------------------------------------------------------------
static int IsPowerOfTwo( unsigned long n ) { return ((n & (n-1)) == 0); }

/*******************************************************************
* Determine size of native ASIO audio buffer size
* Input parameters : FramesPerUserBuffer, NumUserBuffers 
* Output values : FramesPerHostBuffer, OutputBufferOffset or InputtBufferOffset
*/

static PaError PaHost_CalcNumHostBuffers( internalPortAudioStream *past )
{
        PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
        long requestedBufferSize;
        long firstMultiple, firstDivisor;
        
        // Compute requestedBufferSize 
        if( past->past_NumUserBuffers < 1 ){
                requestedBufferSize = past->past_FramesPerUserBuffer;           
        }else{
                requestedBufferSize = past->past_NumUserBuffers * past->past_FramesPerUserBuffer;
        }
        
        // Adjust FramesPerHostBuffer using requestedBufferSize, ASIO minSize and maxSize, 
        if (requestedBufferSize < asioDriverInfo.pahsc_minSize){
        
        		firstMultiple = GetFirstMultiple(asioDriverInfo.pahsc_minSize, requestedBufferSize);
        		
        		if (firstMultiple <= asioDriverInfo.pahsc_maxSize)
        				asioDriverInfo.past_FramesPerHostBuffer = firstMultiple;
        		else
        				asioDriverInfo.past_FramesPerHostBuffer = asioDriverInfo.pahsc_minSize;
        				
        }else if (requestedBufferSize > asioDriverInfo.pahsc_maxSize){
        	
        	   	firstDivisor = GetFirstPossibleDivisor(asioDriverInfo.pahsc_maxSize, requestedBufferSize);
        	   	
        	   	if ((firstDivisor >= asioDriverInfo.pahsc_minSize) && (firstDivisor <= asioDriverInfo.pahsc_maxSize))
                		asioDriverInfo.past_FramesPerHostBuffer = firstDivisor;
               	else
               			asioDriverInfo.past_FramesPerHostBuffer = asioDriverInfo.pahsc_maxSize;
        }else{
                asioDriverInfo.past_FramesPerHostBuffer = requestedBufferSize;
        }
        
        // If ASIO buffer size needs to be a power of two
        if( asioDriverInfo.pahsc_granularity < 0 ){
                // Needs to be a power of two.
                
                if( !IsPowerOfTwo( asioDriverInfo.past_FramesPerHostBuffer ) )
                {
                        int highestBit = GetHighestBitPosition(asioDriverInfo.past_FramesPerHostBuffer);
                        asioDriverInfo.past_FramesPerHostBuffer = 1 << (highestBit + 1);
                }
        }
        
        DBUG(("----------------------------------\n"));
        DBUG(("PaHost_CalcNumHostBuffers : minSize = %ld \n",asioDriverInfo.pahsc_minSize));
        DBUG(("PaHost_CalcNumHostBuffers : preferredSize = %ld \n",asioDriverInfo.pahsc_preferredSize));
        DBUG(("PaHost_CalcNumHostBuffers : maxSize = %ld \n",asioDriverInfo.pahsc_maxSize));
        DBUG(("PaHost_CalcNumHostBuffers : granularity = %ld \n",asioDriverInfo.pahsc_granularity));
        DBUG(("PaHost_CalcNumHostBuffers : User buffer size = %d\n", asioDriverInfo.past->past_FramesPerUserBuffer ));
        DBUG(("PaHost_CalcNumHostBuffers : ASIO buffer size = %d\n", asioDriverInfo.past_FramesPerHostBuffer ));
        
        return paNoError;
}


/***********************************************************************/
int Pa_CountDevices()
{
        PaError err ;
        
        if( sNumDevices <= 0 ) 
        {
                /* Force loading of ASIO drivers  */
                err = Pa_ASIO_QueryDeviceInfo(sDevices);
                if( err != paNoError ) goto error;
        }
        
        return sNumDevices;
        
error:
        PaHost_Term();
        DBUG(("Pa_CountDevices: returns %d\n", err ));
        return err;
}

/***********************************************************************/
PaError PaHost_Init( void )
{
       /* Have we already initialized the device info? */
        PaError err = (PaError) Pa_CountDevices();
    	return ( err < 0 ) ? err : paNoError;
}

/***********************************************************************/
PaError PaHost_Term( void )
{       
        int           i;
        PaDeviceInfo *dev;
        double       *rates;
        PaError      result = paNoError;
         
        if (sNumDevices > 0) {
	        
	        /* Free allocated sample rate arrays  and names*/
	        for( i=0; i<sNumDevices; i++ ){
	                dev =  &sDevices[i].pad_Info;
	                rates = (double *) dev->sampleRates;
	                if ((rates != NULL)) PaHost_FreeFastMemory(rates, MAX_NUMSAMPLINGRATES * sizeof(double)); 
	                dev->sampleRates = NULL;
	               if(dev->name != NULL) PaHost_FreeFastMemory((void *) dev->name, 32);
	                dev->name = NULL;
	        }
	        
	        sNumDevices = 0;
                
            /* If the stream has been closed with PaHost_CloseStream, asioDriverInfo.past == null, otherwise close it now */
	        if(asioDriverInfo.past != NULL) Pa_CloseStream(asioDriverInfo.past);
	          
	        /* remove the loaded ASIO driver */
	        asioDrivers->removeCurrentDriver();
        }

        return result;
}

/***********************************************************************/
PaError PaHost_OpenStream( internalPortAudioStream   *past )
{
        PaError             result = paNoError;
        ASIOError                       err;
        int32                           device;
        
        /*  Check if a stream already runs */
        if (asioDriverInfo.past != NULL) return paHostError;
                        
        /* Check the device number */
        if ((past->past_InputDeviceID != paNoDevice)
                &&(past->past_OutputDeviceID != paNoDevice)
                &&(past->past_InputDeviceID != past->past_OutputDeviceID))
        {
                return paInvalidDeviceId;
        }

        /* Allocation */        
        memset(&asioDriverInfo, 0, sizeof(PaHostSoundControl));
        past->past_DeviceData = (void*) &asioDriverInfo;
        
        /* Reentrancy counter initialisation */
        asioDriverInfo.reenterCount = -1;
        asioDriverInfo.reenterError = 0;

        /* FIXME */
        asioDriverInfo.past = past;
        
        /* load the ASIO device */
        device = (past->past_InputDeviceID < 0) ? past->past_OutputDeviceID : past->past_InputDeviceID;
        result = Pa_ASIO_loadDevice(device);
        if (result != paNoError) goto error;
                
        /* Check ASIO parameters and input parameters */
        if ((past->past_NumInputChannels > asioDriverInfo.pahsc_NumInputChannels) 
                || (past->past_NumOutputChannels > asioDriverInfo.pahsc_NumOutputChannels)) {
                result = paInvalidChannelCount;
                goto error;
        }
        
        /* Set sample rate */
        if (ASIOSetSampleRate(past->past_SampleRate) != ASE_OK) {
                result = paInvalidSampleRate;
                goto error;
        }
        
        /* if OK calc buffer size */
        result = PaHost_CalcNumHostBuffers( past );
        if (result != paNoError) goto error;
        
           
        /* 
        Allocating input and output buffers number for the real past_NumInputChannels and past_NumOutputChannels
        optimize the data transfer.
        */      
        
        asioDriverInfo.pahsc_NumInputChannels = past->past_NumInputChannels;
        asioDriverInfo.pahsc_NumOutputChannels = past->past_NumOutputChannels;
        
        /* Allocate ASIO buffers and callback*/
        err = Pa_ASIO_CreateBuffers(&asioDriverInfo,
                asioDriverInfo.pahsc_NumInputChannels,
                asioDriverInfo.pahsc_NumOutputChannels,
                asioDriverInfo.past_FramesPerHostBuffer);
                
       	
       	/* 
       		Some buggy drivers (like the Hoontech DSP24) give incorrect [min, preferred, max] values
       	   	They should work with the preferred size value, thus if Pa_ASIO_CreateBuffers fails with 
       	   	the hostBufferSize computed in PaHost_CalcNumHostBuffers, we try again with the preferred size. 
       	*/ 
      	
       	if (err != ASE_OK) {
       
        	DBUG(("PaHost_OpenStream : Pa_ASIO_CreateBuffers failed with the requested framesPerBuffer = %ld \n", asioDriverInfo.past_FramesPerHostBuffer));
        	
            err = Pa_ASIO_CreateBuffers(&asioDriverInfo,
                	asioDriverInfo.pahsc_NumInputChannels,
                	asioDriverInfo.pahsc_NumOutputChannels,
                	asioDriverInfo.pahsc_preferredSize);
                                  
            if (err == ASE_OK) {
            	// Adjust FramesPerHostBuffer to take the preferredSize instead of the value computed in PaHost_CalcNumHostBuffers
            	asioDriverInfo.past_FramesPerHostBuffer = asioDriverInfo.pahsc_preferredSize;
            	DBUG(("PaHost_OpenStream : Adjust FramesPerHostBuffer to take the preferredSize instead of the value computed in PaHost_CalcNumHostBuffers\n"));
            } else {
            	DBUG(("PaHost_OpenStream : Pa_ASIO_CreateBuffers failed with the preferred framesPerBuffer = %ld \n", asioDriverInfo.pahsc_preferredSize));
            }
       	}
       	
       	/* Compute buffer adapdation offset */
       	PaHost_CalcBufferOffset(past);
     
        if (err == ASE_OK) 
                return paNoError;
        else if (err == ASE_NoMemory) 
                result = paInsufficientMemory;
        else if (err == ASE_InvalidParameter) 
                result = paInvalidChannelCount;
        else if (err == ASE_InvalidMode) 
                result = paBufferTooBig;
        else 
                result = paHostError;
                 
error:
        ASIOExit();
        return result;

}

/***********************************************************************/
PaError PaHost_CloseStream( internalPortAudioStream   *past )
{
        PaHostSoundControl *pahsc;
        PaError             result = paNoError;

        if( past == NULL ) return paBadStreamPtr;
        pahsc = (PaHostSoundControl *) past->past_DeviceData;
        if( pahsc == NULL ) return paNoError;

        #if PA_TRACE_START_STOP
         AddTraceMessage( "PaHost_CloseStream: pahsc_HWaveOut ", (int) pahsc->pahsc_HWaveOut );
        #endif
        
        /* Free data and device for output. */
        past->past_DeviceData = NULL;
        asioDriverInfo.past = NULL;
        
        /* Dispose */
        if(ASIODisposeBuffers() != ASE_OK) result = paHostError;        
        if(ASIOExit() != ASE_OK) result = paHostError;
                
        return result;
}

/***********************************************************************/
PaError PaHost_StartOutput( internalPortAudioStream   *past )
{
        /* Clear the index 0 host output buffer */
     	Pa_ASIO_Clear_Output(asioDriverInfo.bufferInfos, 
            	asioDriverInfo.pahsc_channelInfos[0].type,
            	asioDriverInfo.pahsc_NumInputChannels,
            	asioDriverInfo.pahsc_NumOutputChannels,
            	0, 
            	0, 
            	asioDriverInfo.past_FramesPerHostBuffer);

		/* Clear the index 1 host output buffer */
     	Pa_ASIO_Clear_Output(asioDriverInfo.bufferInfos, 
            	asioDriverInfo.pahsc_channelInfos[0].type,
            	asioDriverInfo.pahsc_NumInputChannels,
            	asioDriverInfo.pahsc_NumOutputChannels,
            	1, 
            	0, 
            	asioDriverInfo.past_FramesPerHostBuffer);
            	
        Pa_ASIO_Clear_User_Buffers();
        
        Pa_ASIO_Adaptor_Init();

        return paNoError;
}

/***********************************************************************/
PaError PaHost_StopOutput( internalPortAudioStream   *past, int abort )
{
        /* Nothing to do ?? */
        return paNoError;
}

/***********************************************************************/
PaError PaHost_StartInput( internalPortAudioStream   *past )
{
        /* Nothing to do ?? */
        return paNoError;
}

/***********************************************************************/
PaError PaHost_StopInput( internalPortAudioStream   *past, int abort )
{
        /* Nothing to do */
        return paNoError;
}

/***********************************************************************/
PaError PaHost_StartEngine( internalPortAudioStream   *past )
{
	    // TO DO : count of samples
        past->past_IsActive = 1;
        return (ASIOStart() == ASE_OK) ? paNoError : paHostError;
}

/***********************************************************************/
PaError PaHost_StopEngine( internalPortAudioStream *past, int abort )
{
        // TO DO :  count of samples
        past->past_IsActive = 0;
        return (ASIOStop() == ASE_OK) ? paNoError : paHostError;
}

/***********************************************************************/
// TO BE CHECKED 
PaError PaHost_StreamActive( internalPortAudioStream   *past )
{
        PaHostSoundControl *pahsc;
        if( past == NULL ) return paBadStreamPtr;
        pahsc = (PaHostSoundControl *) past->past_DeviceData;
        if( pahsc == NULL ) return paInternalError;
        return (PaError) past->past_IsActive;
}

/*************************************************************************/
PaTimestamp Pa_StreamTime( PortAudioStream *stream )
{
        PaHostSoundControl *pahsc;
        internalPortAudioStream   *past = (internalPortAudioStream *) stream;
        if( past == NULL ) return paBadStreamPtr;
        pahsc = (PaHostSoundControl *) past->past_DeviceData;
        return pahsc->pahsc_NumFramesDone;
}

/*************************************************************************
 * Allocate memory that can be accessed in real-time.
 * This may need to be held in physical memory so that it is not
 * paged to virtual memory.
 * This call MUST be balanced with a call to PaHost_FreeFastMemory().
 */
void *PaHost_AllocateFastMemory( long numBytes )
{
        #if MAC
                void *addr = NewPtrClear( numBytes );
                if( (addr == NULL) || (MemError () != 0) ) return NULL;
                        
                #if (CARBON_COMPATIBLE == 0)
                if( HoldMemory( addr, numBytes ) != noErr )
                {
                        DisposePtr( (Ptr) addr );
                        return NULL;
                }
                #endif
                return addr;
        #elif WINDOWS
                void *addr = malloc( numBytes ); /* FIXME - do we need physical memory? */
                if( addr != NULL ) memset( addr, 0, numBytes );
                return addr;
        #endif
}

/*************************************************************************
 * Free memory that could be accessed in real-time.
 * This call MUST be balanced with a call to PaHost_AllocateFastMemory().
 */
void PaHost_FreeFastMemory( void *addr, long numBytes )
{
        #if MAC
                if( addr == NULL ) return;
                #if CARBON_COMPATIBLE
                (void) numBytes;
                #else
                UnholdMemory( addr, numBytes );
                #endif
                DisposePtr( (Ptr) addr );
        #elif WINDOWS
                if( addr != NULL ) free( addr );
        #endif
}


/*************************************************************************/
void Pa_Sleep( long msec )
{
        #if MAC
                int32 sleepTime, endTime;
                /* Convert to ticks. Round up so we sleep a MINIMUM of msec time. */
                sleepTime = ((msec * 60) + 999) / 1000;
                if( sleepTime < 1 ) sleepTime = 1;
                endTime = TickCount() + sleepTime;
                do{
                        DBUGX(("Sleep for %d ticks.\n", sleepTime ));
                        WaitNextEvent( 0, NULL, sleepTime, NULL );  /* Use this just to sleep without getting events. */
                        sleepTime = endTime - TickCount();
                } while( sleepTime > 0 );
        #elif WINDOWS
                Sleep( msec );
        #endif 
}

/*************************************************************************/
const PaDeviceInfo* Pa_GetDeviceInfo( PaDeviceID id )
{
        if( (id < 0) || ( id >= Pa_CountDevices()) ) return NULL;
        return &sDevices[id].pad_Info;
}

/*************************************************************************/
PaDeviceID Pa_GetDefaultInputDeviceID( void )
{
        return (sNumDevices > 0) ? sDefaultInputDeviceID : paNoDevice;
}

/*************************************************************************/
PaDeviceID Pa_GetDefaultOutputDeviceID( void )
{
		return (sNumDevices > 0) ? sDefaultOutputDeviceID : paNoDevice;
}

/*************************************************************************/
int Pa_GetMinNumBuffers( int framesPerUserBuffer, double sampleRate )
{
        // TO BE IMPLEMENTED : using the ASIOGetLatency call??
        return 2;
}

/*************************************************************************/
int32 Pa_GetHostError( void )
{
        int32 err = sPaHostError;
        sPaHostError = 0;
        return err;
}


#ifdef MAC                                      

/**************************************************************************/
static void Pa_StartUsageCalculation( internalPortAudioStream   *past )
{
        PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
        UnsignedWide widePad;
        if( pahsc == NULL ) return;
/* Query system timer for usage analysis and to prevent overuse of CPU. */
        Microseconds( &widePad );
        pahsc->pahsc_EntryCount = UnsignedWideToUInt64( widePad );
}
/**************************************************************************/
static void Pa_EndUsageCalculation( internalPortAudioStream   *past )
{
        UnsignedWide widePad;
        UInt64    CurrentCount;
        long      InsideCount;
        long      TotalCount;
        PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
        if( pahsc == NULL ) return;
/* Measure CPU utilization during this callback. Note that this calculation
** assumes that we had the processor the whole time.
*/
#define LOWPASS_COEFFICIENT_0   (0.9)
#define LOWPASS_COEFFICIENT_1   (0.99999 - LOWPASS_COEFFICIENT_0)
        Microseconds( &widePad );
        CurrentCount = UnsignedWideToUInt64( widePad );
        if( past->past_IfLastExitValid )
        {
                InsideCount = (long) U64Subtract(CurrentCount, pahsc->pahsc_EntryCount);
                TotalCount  = (long) U64Subtract(CurrentCount, pahsc->pahsc_LastExitCount);
/* Low pass filter the result because sometimes we get called several times in a row.
* That can cause the TotalCount to be very low which can cause the usage to appear
* unnaturally high. So we must filter numerator and denominator separately!!!
*/
                past->past_AverageInsideCount = (( LOWPASS_COEFFICIENT_0 * past->past_AverageInsideCount) +
                        (LOWPASS_COEFFICIENT_1 * InsideCount));
                past->past_AverageTotalCount = (( LOWPASS_COEFFICIENT_0 * past->past_AverageTotalCount) +
                        (LOWPASS_COEFFICIENT_1 * TotalCount));
                past->past_Usage = past->past_AverageInsideCount / past->past_AverageTotalCount;
        }
        pahsc->pahsc_LastExitCount = CurrentCount;
        past->past_IfLastExitValid = 1;
}

#elif WINDOWS

/********************************* BEGIN CPU UTILIZATION MEASUREMENT ****/
static void Pa_StartUsageCalculation( internalPortAudioStream   *past )
{
        PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
        if( pahsc == NULL ) return;
/* Query system timer for usage analysis and to prevent overuse of CPU. */
        QueryPerformanceCounter( &pahsc->pahsc_EntryCount );
}

static void Pa_EndUsageCalculation( internalPortAudioStream   *past )
{
        LARGE_INTEGER CurrentCount = { 0, 0 };
        LONGLONG      InsideCount;
        LONGLONG      TotalCount;
/*
** Measure CPU utilization during this callback. Note that this calculation
** assumes that we had the processor the whole time.
*/
#define LOWPASS_COEFFICIENT_0   (0.9)
#define LOWPASS_COEFFICIENT_1   (0.99999 - LOWPASS_COEFFICIENT_0)

        PaHostSoundControl *pahsc = (PaHostSoundControl *) past->past_DeviceData;
        if( pahsc == NULL ) return;

        if( QueryPerformanceCounter( &CurrentCount ) )
        {
                if( past->past_IfLastExitValid )
                {
                        InsideCount = CurrentCount.QuadPart - pahsc->pahsc_EntryCount.QuadPart; 
                        TotalCount =  CurrentCount.QuadPart - pahsc->pahsc_LastExitCount.QuadPart;
/* Low pass filter the result because sometimes we get called several times in a row.
 * That can cause the TotalCount to be very low which can cause the usage to appear
 * unnaturally high. So we must filter numerator and denominator separately!!!
 */
                        past->past_AverageInsideCount = (( LOWPASS_COEFFICIENT_0 * past->past_AverageInsideCount) +
                                (LOWPASS_COEFFICIENT_1 * InsideCount));
                        past->past_AverageTotalCount = (( LOWPASS_COEFFICIENT_0 * past->past_AverageTotalCount) +
                                (LOWPASS_COEFFICIENT_1 * TotalCount));
                        past->past_Usage = past->past_AverageInsideCount / past->past_AverageTotalCount;
                }
                pahsc->pahsc_LastExitCount = CurrentCount;
                past->past_IfLastExitValid = 1;
        }
}

#endif




