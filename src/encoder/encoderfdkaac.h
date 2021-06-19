#pragma once

#include <QLibrary>
#include <QString>
#include <memory>

#include "encoder/encoder.h"
#include "util/fifo.h"

class EncoderFdkAac : public Encoder {
  public:
    EncoderFdkAac(EncoderCallback* pCallback);
    virtual ~EncoderFdkAac();

    int initEncoder(int samplerate, QString* pUserErrorMessage) override;
    void encodeBuffer(const CSAMPLE* samples, const int sampleCount) override;
    void updateMetaData(const QString& artist, const QString& title, const QString& album) override;
    void flush() override;
    void setEncoderSettings(const EncoderSettings& settings) override;

  private:
    QString buttWindowsFdkAac();

    // libfdk-aac common AOTs
    static const int AOT_AAC_LC = 2; // AAC-LC: Low Complexity (iTunes)
    static const int AOT_SBR = 5;    // HE-AAC: with Spectral Band Replication
    static const int AOT_PS = 29;    // HE-AACv2: Parametric Stereo (includes SBR)

    // libfdk-aac types and structs
    typedef signed int INT;
    typedef unsigned int UINT;
    typedef signed short SHORT;
    typedef unsigned short USHORT;
    typedef signed char SCHAR;
    typedef unsigned char UCHAR;

    typedef enum {
        AACENC_OK = 0x0000,

        AACENC_INVALID_HANDLE = 0x0020,
        AACENC_MEMORY_ERROR = 0x0021,
        AACENC_UNSUPPORTED_PARAMETER = 0x0022,
        AACENC_INVALID_CONFIG = 0x0023,

        AACENC_INIT_ERROR = 0x0040,
        AACENC_INIT_AAC_ERROR = 0x0041,
        AACENC_INIT_SBR_ERROR = 0x0042,
        AACENC_INIT_TP_ERROR = 0x0043,
        AACENC_INIT_META_ERROR = 0x0044,

        AACENC_ENCODE_ERROR = 0x0060,

        AACENC_ENCODE_EOF = 0x0080,
    } AACENC_ERROR;

    typedef enum {
        AACENC_AOT = 0x0100,
        AACENC_BITRATE = 0x0101,
        AACENC_BITRATEMODE = 0x0102,
        AACENC_SAMPLERATE = 0x0103,
        AACENC_SBR_MODE = 0x0104,
        AACENC_GRANULE_LENGTH = 0x0105,
        AACENC_CHANNELMODE = 0x0106,
        AACENC_CHANNELORDER = 0x0107,
        AACENC_SBR_RATIO = 0x0108,
        AACENC_AFTERBURNER = 0x0200,
        AACENC_BANDWIDTH = 0x0203,
        AACENC_PEAK_BITRATE = 0x0207,
        AACENC_TRANSMUX = 0x0300,
        AACENC_HEADER_PERIOD = 0x0301,
        AACENC_SIGNALING_MODE = 0x0302,
        AACENC_TPSUBFRAMES = 0x0303,
        AACENC_AUDIOMUXVER = 0x0304,
        AACENC_PROTECTION = 0x0306,
        AACENC_ANCILLARY_BITRATE = 0x0500,
        AACENC_METADATA_MODE = 0x0600,
        AACENC_CONTROL_STATE = 0xFF00,
        AACENC_NONE = 0xFFFF
    } AACENC_PARAM;

    typedef enum {
        IN_AUDIO_DATA = 0,
        IN_ANCILLRY_DATA = 1, // as is in fdk-aac
        IN_METADATA_SETUP = 2,
        OUT_BITSTREAM_DATA = 3,
        OUT_AU_SIZES = 4
    } AACENC_BufferIdentifier;

    typedef enum {
        FDK_NONE = 0,
        FDK_TOOLS = 1,
        FDK_SYSLIB = 2,
        FDK_AACDEC = 3,
        FDK_AACENC = 4,
        FDK_SBRDEC = 5,
        FDK_SBRENC = 6,
        FDK_TPDEC = 7,
        FDK_TPENC = 8,
        FDK_MPSDEC = 9,
        FDK_MPEGFILEREAD = 10,
        FDK_MPEGFILEWRITE = 11,
        FDK_MP2DEC = 12,
        FDK_DABDEC = 13,
        FDK_DABPARSE = 14,
        FDK_DRMDEC = 15,
        FDK_DRMPARSE = 16,
        FDK_AACLDENC = 17,
        FDK_MP2ENC = 18,
        FDK_MP3ENC = 19,
        FDK_MP3DEC = 20,
        FDK_MP3HEADPHONE = 21,
        FDK_MP3SDEC = 22,
        FDK_MP3SENC = 23,
        FDK_EAEC = 24,
        FDK_DABENC = 25,
        FDK_DMBDEC = 26,
        FDK_FDREVERB = 27,
        FDK_DRMENC = 28,
        FDK_METADATATRANSCODER = 29,
        FDK_AC3DEC = 30,
        FDK_PCMDMX = 31,

        FDK_MODULE_LAST
    } FDK_MODULE_ID;

    typedef struct AACENCODER* HANDLE_AACENCODER;
    typedef struct {
        UINT maxOutBufBytes;
        UINT maxAncBytes;
        UINT inBufFillLevel;
        UINT inputChannels;
        UINT frameLength;
        UINT encoderDelay;
        UCHAR confBuf[64];
        UINT confSize;
    } AACENC_InfoStruct;
    typedef struct {
        INT numBufs;
        void** bufs;
        INT* bufferIdentifiers;
        INT* bufSizes;
        INT* bufElSizes;
    } AACENC_BufDesc;
    typedef struct {
        INT numInSamples;
        INT numAncBytes;
    } AACENC_InArgs;
    typedef struct {
        INT numOutBytes;
        INT numInSamples;
        INT numAncBytes;
    } AACENC_OutArgs;
    typedef struct {
        const char* title;
        const char* build_date;
        const char* build_time;
        FDK_MODULE_ID module_id;
        INT version;
        UINT flags;
        char versionStr[32];
    } LIB_INFO;

    // libfdk-aac functions prototypes
    typedef AACENC_ERROR (*aacEncGetLibInfo_)(LIB_INFO*);

    typedef AACENC_ERROR (*aacEncOpen_)(
            HANDLE_AACENCODER*,
            const UINT,
            const UINT);

    typedef AACENC_ERROR (*aacEncClose_)(HANDLE_AACENCODER*);

    typedef AACENC_ERROR (*aacEncEncode_)(
            const HANDLE_AACENCODER,
            const AACENC_BufDesc*,
            const AACENC_BufDesc*,
            const AACENC_InArgs*,
            AACENC_OutArgs*);

    typedef AACENC_ERROR (*aacEncInfo_)(
            const HANDLE_AACENCODER,
            AACENC_InfoStruct*);

    typedef AACENC_ERROR (*aacEncoder_SetParam_)(
            const HANDLE_AACENCODER,
            const AACENC_PARAM,
            const UINT);

    // libfdk-aac function pointers
    aacEncGetLibInfo_ aacEncGetLibInfo;
    aacEncOpen_ aacEncOpen;
    aacEncClose_ aacEncClose;
    aacEncEncode_ aacEncEncode;
    aacEncInfo_ aacEncInfo;
    aacEncoder_SetParam_ aacEncoder_SetParam;

    // Instance methods
    void processFIFO();

    // Instance attributes
    int m_aacAot;
    int m_bitrate;
    int m_channels;
    int m_samplerate;
    EncoderCallback* m_pCallback;
    std::unique_ptr<QLibrary> m_pLibrary;
    FIFO<SAMPLE>* m_pInputFifo;
    SAMPLE* m_pFifoChunkBuffer;
    int m_readRequired;
    HANDLE_AACENCODER m_aacEnc;
    unsigned char* m_pAacDataBuffer;
    AACENC_InfoStruct m_aacInfo;
    bool m_hasSbr;
};
