/**
 * \file soundsourcemediafoundation.cpp
 * \author Albert Santoni <alberts at mixxx dot org>
 * \date Jan 10, 2011
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtDebug>
#include <QUrl>
#include <taglib/mp4file.h>


//#define WINVER _WIN32_WINNT_WIN7

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <stdio.h>
#include <mferror.h>
#include <propvarutil.h>

#include "soundsourcemediafoundation.h"

const int kBitsPerSample = 16;
const int kNumChannels = 2;
const int kSampleRate = 44100;
const int kLeftoverSize = 16384;

/** Microsoft examples use this snippet often. */
template<class T> void SafeRelease(T **ppT)
{
    if (*ppT) {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

SoundSourceMediaFoundation::SoundSourceMediaFoundation(QString filename)
    : SoundSource(filename)
    , m_file(filename)
    , m_pAudioType(NULL)
    , m_pReader(NULL)
    , m_nextFrame(0)
    , m_leftoverBuffer(NULL)
    , m_leftoverBufferSize(0)
    , m_leftoverBufferLength(0)
    , m_leftoverBufferPosition(0)
    , m_mfDuration(0)
    , m_dead(false)
{
    // these are always the same, might as well just stick them here
    // -bkgood
    m_iChannels = kNumChannels;
    m_iSampleRate = kSampleRate;

    m_wcFilename = new wchar_t[255];
}

SoundSourceMediaFoundation::~SoundSourceMediaFoundation()
{
    delete [] m_wcFilename;
    delete [] m_leftoverBuffer;

    SafeRelease(&m_pReader);
    SafeRelease(&m_pAudioType);
    MFShutdown();
    CoUninitialize();
}

int SoundSourceMediaFoundation::open()
{
    QString qurlStr(m_qFilename);
    int wcFilenameLength(m_qFilename.toWCharArray(m_wcFilename));
    // toWCharArray does not append a null terminator to the string!
    m_wcFilename[wcFilenameLength] = '\0';

    HRESULT hr(S_OK);
    // Initialize the COM library.
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to initialize COM";
        return ERR;
    }

    // Initialize the Media Foundation platform.
    hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to initialize Media Foundation";
        return ERR;
    }

    // Create the source reader to read the input file.
    hr = MFCreateSourceReaderFromURL(m_wcFilename, NULL, &m_pReader);
    if (FAILED(hr)) {
        qDebug() << "SSMF: Error opening input file:" << m_qFilename;
        return ERR;
    }

    hr = ConfigureAudioStream(m_pReader, &m_pAudioType);
    if (FAILED(hr)) {
        qDebug() << "SSMF: Error configuring audio stream.";
        return ERR;
    }

    if (!ReadProperties()) {
        qDebug() << "SSMF::ReadProperties failed";
        return ERR;
    }

    //Seek to position 0, which forces us to skip over all the header frames.
    //This makes sure we're ready to just let the Analyser rip and it'll
    //get the number of samples it expects (ie. no header frames).
    seek(0);

    return OK;
}

long SoundSourceMediaFoundation::seek(long filepos)
{
    if (m_dead) return filepos;
    //http://msdn.microsoft.com/en-us/library/dd374668(v=VS.85).aspx
    PROPVARIANT v;
    HRESULT hr(S_OK);
    // this doesn't fail, see MS's implementation
    hr = InitPropVariantFromInt64(mfFromFrame(filepos / kNumChannels), &v);

    hr = m_pReader->SetCurrentPosition(GUID_NULL, v);
    if (FAILED(hr)) {
        // nothing we can do here as we can't fail (no facility to other than
        // crashing mixxx)
        qDebug() << "SSMF: failed to seek" << (
            hr == MF_E_INVALIDREQUEST ? "Sample requests still pending" : "");
    }

    // record the next frame so that we can make sure we're there the next
    // time we get a buffer from MFSourceReader
    m_nextFrame = filepos / kNumChannels;

    return filepos;
}

unsigned int SoundSourceMediaFoundation::read(unsigned long size, const SAMPLE *destination)
{
    SAMPLE *destBuffer(const_cast<SAMPLE*>(destination));
    size_t framesNeeded = size / kNumChannels;

    // TODO XXX MAKE SURE WE'RE AT THE POSITION CACHINGREADER EXPECTS US TO BE AT
    // first, copy samples from leftover buffer IF the leftover buffer is at
    // the correct sample
    if (m_leftoverBufferLength > 0 && m_leftoverBufferPosition == m_nextFrame) {
        CopyFrames(destBuffer, &framesNeeded, m_leftoverBuffer, &m_leftoverBufferLength);
        if (m_leftoverBufferLength > 0) {
            Q_ASSERT(framesNeeded == 0); // make sure CopyFrames worked
            // update leftoverbuffer pos
        }
    } else {
        m_leftoverBufferLength = 0;
    }

    if (m_dead) return size - (framesNeeded / kNumChannels);

    while (framesNeeded > 0) {
        HRESULT hr(S_OK);
        DWORD dwFlags(0);
        qint64 timestamp(0);
        IMFSample *pSample(NULL);
        // Read the next "sample" (it's really a buffer of audio).
        hr = m_pReader->ReadSample(
            MF_SOURCE_READER_FIRST_AUDIO_STREAM, // [in] DWORD dwStreamIndex,
            0,                                   // [in] DWORD dwControlFlags,
            NULL,                                // [out] DWORD *pdwActualStreamIndex,
            &dwFlags,                            // [out] DWORD *pdwStreamFlags,
            &timestamp,                          // [out] LONGLONG *pllTimestamp,
            &pSample);                           // [out] IMFSample **ppSample
        if (FAILED(hr)) break;
        if (dwFlags & MF_SOURCE_READERF_ERROR) {
            // our source reader is now dead, according to the docs
            m_dead = true;
            break;
        }
        if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
            qDebug() << "End of input file.";
            break;
        }
        if (dwFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) {
            qDebug() << "Type change";
            break;
        }
        if (pSample == NULL) {
            qDebug() << "No sample";
            continue;
        }

        // get the composite IMFMediaBuffer (why we get the composite is beyond
        // me, we should only have one stream selected)
        IMFMediaBuffer *pBuffer(NULL);
        hr = pSample->ConvertToContiguousBuffer(&pBuffer);
        if (FAILED(hr)) break;

        // get access to the raw data in the buffer...
        quint8 *rawBuffer(NULL);
        DWORD bufferByteLength(0);
        hr = pBuffer->Lock(&rawBuffer, NULL, &bufferByteLength);
        if (FAILED(hr)) break;

        // but in more agreeable terms:
        size_t realBufferLength(bufferByteLength / (kBitsPerSample / 8 * kNumChannels));
        qint16 *realBuffer(reinterpret_cast<qint16*>(rawBuffer));

        // copy the data to destBuffer;
        CopyFrames(destBuffer, &framesNeeded, realBuffer, &realBufferLength);
        if (m_leftoverBufferLength > 0) {
            Q_ASSERT(framesNeeded == 0); // make sure CopyFrames worked
            // update leftover position
        }
        // unlock the buffer.
        hr = pBuffer->Unlock();
        if (FAILED(hr)) break;

        SafeRelease(&pBuffer);
        SafeRelease(&pSample);
    }

    return size - (framesNeeded * kNumChannels);
}

inline unsigned long SoundSourceMediaFoundation::length()
{
    return secondsFromMF(m_mfDuration) * kSampleRate * kNumChannels;
}

int SoundSourceMediaFoundation::parseHeader()
{
    setType("m4a");

    TagLib::MP4::File f(getFilename().toLocal8Bit().constData());
    bool result = processTaglibFile(f);
    TagLib::MP4::Tag* tag = f.tag();

    if (tag) {
        processMP4Tag(tag);
    }

    if (result)
        return OK;
    return ERR;
}


// static
QList<QString> SoundSourceMediaFoundation::supportedFileExtensions()
{
    QList<QString> list;
    list.push_back("m4a");
    list.push_back("mp4");
    return list;
}


//-------------------------------------------------------------------
// ConfigureAudioStream
//
// Selects an audio stream from the source file, and configures the
// stream to deliver decoded PCM audio.
//-------------------------------------------------------------------

/** Cobbled together from:
    http://msdn.microsoft.com/en-us/library/dd757929(v=vs.85).aspx
    and http://msdn.microsoft.com/en-us/library/dd317928(VS.85).aspx
    -- Albert
    If anything in here fails, just bail. I'm not going to decode HRESULTS.
    -- Bill
    */
bool SoundSourceMediaFoundation::ConfigureAudioStream(
    IMFSourceReader *pReader,
    IMFMediaType **ppPCMAudio)
{
    IMFMediaType *pMediaType(NULL);
    HRESULT hr(S_OK);

    // deselect all streams, we only want the first
    hr = pReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to deselect all streams";
        return false;
    }

    hr = pReader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, true);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to select first audio stream";
        return false;
    }

    hr = MFCreateMediaType(&pMediaType);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to create media type";
        return false;
    }

    hr = pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to set major type";
        return false;
    }

    hr = pMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to set subtype";
        return false;
    }

    hr = pMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, true);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to set samples independent";
        return false;
    }

    hr = pMediaType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, true);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to set fixed size samples";
        return false;
    }

    hr = pMediaType->SetUINT32(MF_MT_SAMPLE_SIZE, kLeftoverSize);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to set sample size";
        return false;
    }

    // MSDN for this attribute says that if bps is 8, samples are unsigned.
    // Otherwise, they're signed (so they're signed for us as 16 bps). Why
    // chose to hide this rather useful tidbit here is beyond me -bkgood
    hr = pMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, kBitsPerSample);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to set bits per sample";
        return false;
    }

    hr = pMediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, kNumChannels * (kBitsPerSample / 8));
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to set block alignment";
        return false;
    }

    hr = pMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, kNumChannels);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to set number of channels";
        return false;
    }

    hr = pMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, kSampleRate);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to set sample rate";
        return false;
    }

    // Set this type on the source reader. The source reader will
    // load the necessary decoder.
    hr = pReader->SetCurrentMediaType(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        NULL, pMediaType);

    // the reader has the media type now, free our reference so we can use our
    // pointer for other purposes
    SafeRelease(&pMediaType);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to set media type";
        return false;
    }

    // Get the complete uncompressed format.
    hr = pReader->GetCurrentMediaType(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        &pMediaType);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to retrieve completed media type";
        return false;
    }

    // Ensure the stream is selected.
    hr = pReader->SetStreamSelection(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        true);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to select first audio stream (again)";
        return false;
    }

    quint32 fixedSamples(false);
    hr = pMediaType->GetUINT32(MF_MT_FIXED_SIZE_SAMPLES, &fixedSamples);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to determine fixed sized samples";
        return false;
    } else {
        if (fixedSamples) {
            qDebug() << "SSMF: fixed sized samples enabled";
        } else {
            qDebug() << "SSMF: fixed sized samples disabled";
        }
    }

    // this may not be safe on all platforms as m_leftoverBufferSize is a
    // size_t and this function is writing a uint32. However, on 32-bit
    // Windows 7, size_t is defined as uint which is 32-bits, so we're safe
    // for all supported platforms -bkgood
    hr = pMediaType->GetUINT32(MF_MT_SAMPLE_SIZE, &m_leftoverBufferSize);
    if (FAILED(hr)) {
        qDebug() << "SSMF: failed to get buffer size, we got" << m_leftoverBufferSize;
        return false;
    } else {
        qDebug() << "SSMF: got buffer size" << m_leftoverBufferSize;
    }
    m_leftoverBufferSize /= 2; // convert size in bytes to size in int16s
    m_leftoverBuffer = new qint16[m_leftoverBufferSize];

    // Return the PCM format to the caller.
    *ppPCMAudio = pMediaType;
    (*ppPCMAudio)->AddRef();

    SafeRelease(&pMediaType);
    return true;
}

bool SoundSourceMediaFoundation::ReadProperties()
{
    PROPVARIANT prop;
    HRESULT hr = S_OK;

    //Get the duration, provided as a 64-bit integer of 100-nanosecond units
    hr = m_pReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE,
        MF_PD_DURATION, &prop);
    if (FAILED(hr)) {
        qDebug() << "SSMF: error getting duration";
        return false;
    }
    // QuadPart isn't available on compilers that don't support _int64. Visual
    // Studio 6.0 introduced the type in 1998, so I think we're safe here
    // -bkgood
    m_iDuration = secondsFromMF(prop.hVal.QuadPart);
    m_mfDuration = prop.hVal.QuadPart;
    qDebug() << "SSMF: Duration:" << m_iDuration;
    PropVariantClear(&prop);

    // presentation attribute MF_PD_AUDIO_ENCODING_BITRATE only exists for
    // presentation descriptors, one of which MFSourceReader is not.
    // Therefore, we calculate it ourselves.
    m_iBitrate = kBitsPerSample * kSampleRate * kNumChannels;

    return true;
}

/**
 * Copies min(destFrames, srcFrames) frames to dest from src. Anything leftover
 * is moved to the beginning of m_leftoverBuffer, so empty it first (possibly
 * with this method). If src and dest overlap, I'll hurt you.
 */
void SoundSourceMediaFoundation::CopyFrames(
    qint16 *dest, size_t *destFrames, qint16 *src, size_t *srcFrames)
{
    Q_ASSERT(m_leftoverBufferLength == 0);
    if (*srcFrames > *destFrames) {
        int samplesToCopy(*destFrames * kNumChannels);
        memcpy(dest, src, samplesToCopy);
        *srcFrames -= *destFrames;
        memmove(m_leftoverBuffer,
            src + samplesToCopy,
            *srcFrames);
        *destFrames = 0;
    } else {
        int samplesToCopy(*srcFrames * kNumChannels);
        memcpy(dest, src, samplesToCopy);
        *destFrames -= *srcFrames;
    }
    // we've either emptied src into leftover or dest, so it's empty
    *srcFrames = 0;
}

/**
 * Convert a 100ns Media Foundation value to a number of seconds.
 */
inline qreal SoundSourceMediaFoundation::secondsFromMF(qint64 mf)
{
    return static_cast<qreal>(mf) / 1e7;
}

/**
 * Convert a number of seconds to a 100ns Media Foundation value.
 */
inline qint64 SoundSourceMediaFoundation::mfFromSeconds(qreal sec)
{
    return sec * 1e7;
}

/**
 * Convert a 100ns Media Foundation value to a frame offset.
 */
inline qint64 SoundSourceMediaFoundation::frameFromMF(qint64 mf)
{
    return static_cast<qreal>(mf) * kSampleRate / 1e7;
}

/**
 * Convert a frame offset to a 100ns Media Foundation value.
 */
inline qint64 SoundSourceMediaFoundation::mfFromFrame(qint64 frame)
{
    return static_cast<qreal>(frame) / kSampleRate * 1e7;
}