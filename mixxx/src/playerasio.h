// PlayerAsio.h: interface for the PlayerAsio class.
//
//////////////////////////////////////////////////////////////////////

#include <qobject.h>
#include <qmutex.h>
#include <qsignal.h>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include "player.h"
#include "asiosys.h"
#include "asio.h"
#include "asiodrivers.h"



#if !defined(PlayerAsio_H)
#define PlayerAsio_H

void __cdecl bufferSwitch(long index, ASIOBool processNow);
long __cdecl asioMessages(long selector, long value, void* message, double* opt);
void __cdecl sampleRateChanged(ASIOSampleRate sRate);
ASIOTime* __cdecl bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow);

enum {
    // number of input and outputs supported by the host application
    // you can change these to higher or lower values
    kMaxInputChannels = 0,
    kMaxOutputChannels = 4
};


// internal data storage
typedef struct DriverInfo
{
    
    ASIODriverInfo driverInfo;
    
    
    long           inputChannels;
    long           outputChannels;
    
    
    long           minSize;
    long           maxSize;
    long           preferredSize;
    long           granularity;
    
    
    ASIOSampleRate sampleRate;
    
    
    bool           postOutput;
    
    
    long           inputLatency;
    long           outputLatency;
    
   
    long inputBuffers;  // becomes number of actual created input buffers
    long outputBuffers; // becomes number of actual created output buffers
    ASIOBufferInfo bufferInfos[kMaxInputChannels + kMaxOutputChannels]; // buffer info's
    
    // ASIOGetChannelInfo()
    ASIOChannelInfo channelInfos[kMaxInputChannels + kMaxOutputChannels]; // channel info's
    
    
    double         nanoSeconds;
    double         samples;
    double         tcSamples;   // time code samples
    
    
    ASIOTime       tInfo;           // time info state
    unsigned long  sysRefTime;      // system reference time, when bufferSwitch() was called
    
  
} DriverInfo;


class PlayerAsio : public Player 
{
    
public:
    
    
    PlayerAsio(ConfigObject<ConfigValue> *config);
    virtual ~PlayerAsio();
    
    bool initialize();
    bool open();
    void close();
    void startAsio();
    
    int getBufferSize(void);
    int getSampleRate(void);
    int getChannelCount(void);
    
    QStringList getInterfaces();
    QStringList getSampleRates();
    
    void setDefaults();
    
    static QString getSoundApi() { return "ASIO"; }
    QString getSoundApiName() { return getSoundApi(); };
    
    void process(const CSAMPLE *, const CSAMPLE *, const int) { };
    void processCallback(long bufferIndex);
    
    
private:
    bool initDriver(void);

    void Output_Float32_Int16(int channelNumber, float* inputBuffer, short* outputBuffer, bool swap); 
    void Output_Float32_Int32(int channelNumber, float* inputBuffer, float* outputBuffer, bool swap); 
    void Output_Float32_Int24(int channelNumber, float* inputBuffer, float* outputBuffer, bool swap); 
    void float32toInt24inPlace(float* buffer, long frames, bool swap);
    void float32toInt32inPlace(float* buffer, long frames, bool swap);
    void SwapBuffer(short* buffer);
    
    ASIOError create_asio_buffers (DriverInfo *asioDriverInfo);
    long init_asio_static_data (DriverInfo *asioDriverInfo);
    
    AsioDrivers* driver;
    int m_reenter;
    bool driverIsLoaded;
    int drvNum;
    int bufferLength;
    char* driverList[32];
    /** Used in sample convertion */
    float *m_pTempBuffer;
     
};

#endif 
