#pragma once

#include <QLibrary>
#include <QtDebug>
#include <memory>

namespace faad2 {

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
constexpr unsigned char FMT_16BIT = 1;
constexpr unsigned char FMT_24BIT = 2;
constexpr unsigned char FMT_32BIT = 3;
constexpr unsigned char FMT_FLOAT = 4;
constexpr unsigned char FMT_FIXED = FMT_FLOAT;
constexpr unsigned char FMT_DOUBLE = 5;

enum class FrameError : unsigned char {
    InvalidNumberOfChannels = 12,
    InvalidChannelConfiguration = 21,
};

typedef void* DecoderHandle;

extern "C" {

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

} // extern "C"

class LibLoader {
  public:
    static LibLoader* Instance();

    bool isLoaded() const;

    DecoderHandle Open() const;

    Configuration* GetCurrentConfiguration(
            DecoderHandle hDecoder) const;

    unsigned char SetConfiguration(
            DecoderHandle hDecoder, Configuration* config) const;

    // Init the decoder using the AAC stream (ADTS/ADIF)
    long Init(
            DecoderHandle hDecoder,
            unsigned char* pBuffer,
            unsigned long sizeofBuffer,
            unsigned long* pSampleRate,
            unsigned char* pChannels) const;

    // Init the decoder using a DecoderSpecificInfo
    char Init2(
            DecoderHandle hDecoder,
            unsigned char* pBuffer,
            unsigned long sizeofBuffer,
            unsigned long* pSampleRate,
            unsigned char* pChannels) const;

    void Close(DecoderHandle hDecoder) const;

    void PostSeekReset(DecoderHandle hDecoder, long frame) const;

    void* Decode2(
            DecoderHandle hDecoder,
            FrameInfo* pInfo,
            unsigned char* pBuffer,
            unsigned long bufferSize,
            void** ppSampleBuffer,
            unsigned long sampleBufferSize) const;

    char* GetErrorMessage(unsigned char errcode) const;

    int GetVersion(
            char** faad2_id_string,
            char** faad2_copyright_string) const;

  private:
    LibLoader();

    LibLoader(const LibLoader&) = delete;
    LibLoader& operator=(const LibLoader&) = delete;

    // QLibrary is not copy-able
    std::unique_ptr<QLibrary> m_pLibrary;

    typedef DecoderHandle (*NeAACDecOpen_t)();
    NeAACDecOpen_t m_neAACDecOpen;

    typedef Configuration* (*NeAACDecGetCurrentConfiguration_t)(DecoderHandle);
    NeAACDecGetCurrentConfiguration_t m_neAACDecGetCurrentConfiguration;

    typedef unsigned char (*NeAACDecSetConfiguration_t)(
            DecoderHandle, Configuration*);
    NeAACDecSetConfiguration_t m_neAACDecSetConfiguration;

    typedef long (*NeAACDecInit_t)(
            DecoderHandle, unsigned char*, unsigned long, unsigned long*, unsigned char*);
    NeAACDecInit_t m_neAACDecInit;

    typedef char (*NeAACDecInit2_t)(
            DecoderHandle, unsigned char*, unsigned long, unsigned long*, unsigned char*);
    NeAACDecInit2_t m_neAACDecInit2;

    typedef void (*NeAACDecClose_t)(DecoderHandle);
    NeAACDecClose_t m_neAACDecClose;

    typedef void (*NeAACDecPostSeekReset_t)(DecoderHandle, long);
    NeAACDecPostSeekReset_t m_neAACDecPostSeekReset;

    typedef void* (*NeAACDecDecode2_t)(
            DecoderHandle, FrameInfo*, unsigned char*, unsigned long, void**, unsigned long);
    NeAACDecDecode2_t m_neAACDecDecode2;

    typedef char* (*NeAACDecGetErrorMessage_t)(unsigned char);
    NeAACDecGetErrorMessage_t m_neAACDecGetErrorMessage;

    typedef int (*NeAACDecGetVersion_t)(
            char** faad2_id_string,
            char** faad2_copyright_string);
    NeAACDecGetVersion_t m_neAACDecGetVersion;
};

} // namespace faad2

QDebug operator<<(
        QDebug dbg,
        const faad2::Configuration& cfg);
