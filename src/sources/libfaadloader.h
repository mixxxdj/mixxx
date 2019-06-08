#pragma once

#include <QLibrary>

#include "util/memory.h"

// object types for AAC
constexpr unsigned char MAIN = 1;
constexpr unsigned char LC = 2;
constexpr unsigned char SSR = 3;
constexpr unsigned char LTP = 4;
constexpr unsigned char HE_AAC = 5;
constexpr unsigned char ER_LC = 17;
constexpr unsigned char ER_LTP = 19;
constexpr unsigned char LD = 23;
constexpr unsigned char DRM_ER_LC = 27; // special object type for DRM

// library output formats
constexpr unsigned char FAAD_FMT_16BIT = 1;
constexpr unsigned char FAAD_FMT_24BIT = 2;
constexpr unsigned char FAAD_FMT_32BIT = 3;
constexpr unsigned char FAAD_FMT_FLOAT = 4;
constexpr unsigned char FAAD_FMT_FIXED = FAAD_FMT_FLOAT;
constexpr unsigned char FAAD_FMT_DOUBLE = 5;

class LibFaadLoader {
  public:
    typedef void* Handle;

    typedef struct
    {
        unsigned char defObjectType;
        unsigned long defSampleRate;
        unsigned char outputFormat;
        unsigned char downMatrix;
        unsigned char useOldADTSFormat;
        unsigned char dontUpSampleImplicitSBR;
    } Configuration;

    typedef struct
    {
        unsigned long bytesconsumed;
        unsigned long samples;
        unsigned char channels;
        unsigned char error;
        unsigned long samplerate;

        // SBR: 0: off, 1: on; upsample, 2: on; downsampled, 3: off; upsampled
        unsigned char sbr;

        // MPEG-4 ObjectType
        unsigned char object_type;

        // AAC header type; MP4 will be signalled as RAW also
        unsigned char header_type;

        // multichannel configuration
        unsigned char num_front_channels;
        unsigned char num_side_channels;
        unsigned char num_back_channels;
        unsigned char num_lfe_channels;
        unsigned char channel_position[64];

        // PS: 0: off, 1: on
        unsigned char ps;
    } FrameInfo;

    static LibFaadLoader* Instance();

    bool isLoaded();

    Handle Open();

    Configuration* GetCurrentConfiguration(
            Handle hDecoder);

    unsigned char SetConfiguration(
            Handle hDecoder, Configuration* config);

    // Init the library using a DecoderSpecificInfo
    char Init2(
            Handle hDecoder,
            unsigned char* pBuffer,
            unsigned long SizeOfDecoderSpecificInfo,
            unsigned long* pSamplerate,
            unsigned char* pChannels);

    void Close(Handle hDecoder);

    void PostSeekReset(Handle hDecoder, long frame);

    void* Decode2(
            Handle hDecoder,
            FrameInfo* pInfo,
            unsigned char* pBuffer,
            unsigned long bufferSize,
            void** ppSampleBuffer,
            unsigned long sampleBufferSize);

    char* GetErrorMessage(unsigned char errcode);

  private:
    LibFaadLoader();

    LibFaadLoader(const LibFaadLoader&) = delete;
    LibFaadLoader& operator=(const LibFaadLoader&) = delete;

    // QLibrary is not copy-able
    std::unique_ptr<QLibrary> m_pLibrary;

    typedef Handle (*NeAACDecOpen_t)();
    NeAACDecOpen_t m_neAACDecOpen;

    typedef Configuration* (*NeAACDecGetCurrentConfiguration_t)(Handle);
    NeAACDecGetCurrentConfiguration_t m_neAACDecGetCurrentConfiguration;

    typedef unsigned char (*NeAACDecSetConfiguration_t)(
            Handle, Configuration*);
    NeAACDecSetConfiguration_t m_neAACDecSetConfiguration;

    typedef char (*NeAACDecInit2_t)(
            Handle, unsigned char*, unsigned long, unsigned long*, unsigned char*);
    NeAACDecInit2_t m_neAACDecInit2;

    typedef void (*NeAACDecClose_t)(Handle);
    NeAACDecClose_t m_neAACDecClose;

    typedef void (*NeAACDecPostSeekReset_t)(Handle, long);
    NeAACDecPostSeekReset_t m_neAACDecPostSeekReset;

    typedef void* (*NeAACDecDecode2_t)(
            Handle, FrameInfo*, unsigned char*, unsigned long, void**, unsigned long);
    NeAACDecDecode2_t m_neAACDecDecode2;

    typedef char* (*NeAACDecGetErrorMessage_t)(unsigned char);
    NeAACDecGetErrorMessage_t m_neAACDecGetErrorMessage;
};
