/**
 * \file soundsourcemediafoundation.cpp
 * \author Bill Good <bkgood at gmail dot com>
 * \author Albert Santoni <alberts at mixxx dot org>
 * \date Jan 10, 2011
 * \note This file uses COM interfaces defined in Windows 7 and later added to
 * Vista and Server 2008 via the "Platform Update Supplement for Windows Vista
 * and for Windows Server 2008" (http://support.microsoft.com/kb/2117917).
 * Earlier versions of Vista (and possibly Server 2008) have some Media
 * Foundation interfaces but not the required IMFSourceReader, and are missing
 * the Microsoft-provided AAC decoder. XP does not include Media Foundation.
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
#include <taglib/mp4file.h>
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <propvarutil.h>

#include "soundsourcemediafoundation.h"

const int kBitsPerSample = 16;
const int kNumChannels = 2;
const int kSampleRate = 44100;
const int kLeftoverSize = 4096; // in int16's, this seems to be the size MF AAC
// decoder likes to give

const static bool sDebug = false;

/** Microsoft examples use this snippet often. */
template<class T> static void safeRelease(T **ppT)
{
    if (*ppT) {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

SoundSourceMediaFoundation::SoundSourceMediaFoundation(QString filename)
    : SoundSource(filename)
    , m_file(filename)
    , m_pReader(NULL)
    , m_pAudioType(NULL)
    , m_wcFilename(NULL)
    , m_nextFrame(0)
    , m_leftoverBuffer(NULL)
    , m_leftoverBufferSize(0)
    , m_leftoverBufferLength(0)
    , m_leftoverBufferPosition(0)
    , m_mfDuration(0)
    , m_iCurrentPosition(0)
    , m_dead(false)
    , m_seeking(false)
{
    // these are always the same, might as well just stick them here
    // -bkgood
    m_iChannels = kNumChannels;
    m_iSampleRate = kSampleRate;

    // http://social.msdn.microsoft.com/Forums/en/netfxbcl/thread/35c6a451-3507-40c8-9d1c-8d4edde7c0cc
    // gives maximum path + file length as 248 + 260, using that -bkgood
    m_wcFilename = new wchar_t[248 + 260];
}

SoundSourceMediaFoundation::~SoundSourceMediaFoundation()
{
    delete [] m_wcFilename;
    delete [] m_leftoverBuffer;

    safeRelease(&m_pReader);
    safeRelease(&m_pAudioType);
    MFShutdown();
    CoUninitialize();
}

int SoundSourceMediaFoundation::open()
{
    if (sDebug) {
        qDebug() << "open()" << m_qFilename;
    }

    QString qurlStr(m_qFilename);
    int wcFilenameLength(m_qFilename.toWCharArray(m_wcFilename));
    // toWCharArray does not append a null terminator to the string!
    m_wcFilename[wcFilenameLength] = '\0';

    HRESULT hr(S_OK);
    // Initialize the COM library.
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to initialize COM";
        return ERR;
    }

    // Initialize the Media Foundation platform.
    hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to initialize Media Foundation";
        return ERR;
    }

    // Create the source reader to read the input file.
    hr = MFCreateSourceReaderFromURL(m_wcFilename, NULL, &m_pReader);
    if (FAILED(hr)) {
        qWarning() << "SSMF: Error opening input file:" << m_qFilename;
        return ERR;
    }

    if (!configureAudioStream()) {
        qWarning() << "SSMF: Error configuring audio stream.";
        return ERR;
    }

    if (!readProperties()) {
        qWarning() << "SSMF::readProperties failed";
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
    if (sDebug) { qDebug() << "seek()" << filepos; }
    PROPVARIANT prop;
    HRESULT hr(S_OK);
    qint64 seekTarget(filepos / kNumChannels);
    qint64 mfSeekTarget(mfFromFrame(seekTarget) - 1);
    // minus 1 here seems to make our seeking work properly, otherwise we will
    // (more often than not, maybe always) seek a bit too far (although not
    // enough for our calculatedFrameFromMF <= nextFrame assertion in ::read).
    // Has something to do with 100ns MF units being much smaller than most
    // frame offsets (in seconds) -bkgood
    long result = m_iCurrentPosition;
    if (m_dead) {
        return result;
    }

    // this doesn't fail, see MS's implementation
    hr = InitPropVariantFromInt64(mfSeekTarget < 0 ? 0 : mfSeekTarget, &prop);


    hr = m_pReader->Flush(MF_SOURCE_READER_FIRST_AUDIO_STREAM);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to flush before seek";
    }

    // http://msdn.microsoft.com/en-us/library/dd374668(v=VS.85).aspx
    hr = m_pReader->SetCurrentPosition(GUID_NULL, prop);
    if (FAILED(hr)) {
        // nothing we can do here as we can't fail (no facility to other than
        // crashing mixxx)
        qWarning() << "SSMF: failed to seek" << (
            hr == MF_E_INVALIDREQUEST ? "Sample requests still pending" : "");
    } else {
        result = filepos;
    }
    PropVariantClear(&prop);

    // record the next frame so that we can make sure we're there the next
    // time we get a buffer from MFSourceReader
    m_nextFrame = seekTarget;
    m_seeking = true;
    m_iCurrentPosition = result;
    return result;
}

unsigned int SoundSourceMediaFoundation::read(unsigned long size,
    const SAMPLE *destination)
{
    if (sDebug) { qDebug() << "read()" << size; }
    SAMPLE *destBuffer(const_cast<SAMPLE*>(destination));
    size_t framesRequested(size / kNumChannels);
    size_t framesNeeded(framesRequested);

    // first, copy frames from leftover buffer IF the leftover buffer is at
    // the correct frame
    if (m_leftoverBufferLength > 0 && m_leftoverBufferPosition == m_nextFrame) {
        copyFrames(destBuffer, &framesNeeded, m_leftoverBuffer,
            m_leftoverBufferLength);
        if (m_leftoverBufferLength > 0) {
            if (framesNeeded != 0) {
                qWarning() << __FILE__ << __LINE__
                           << "WARNING: Expected frames needed to be 0. Abandoning this file.";
                m_dead = true;
            }
            m_leftoverBufferPosition += framesRequested;
        }
    } else {
        // leftoverBuffer already empty or in the wrong position, clear it
        m_leftoverBufferLength = 0;
    }

    while (!m_dead && framesNeeded > 0) {
        HRESULT hr(S_OK);
        DWORD dwFlags(0);
        qint64 timestamp(0);
        IMFSample *pSample(NULL);
        bool error(false); // set to true to break after releasing

        hr = m_pReader->ReadSample(
            MF_SOURCE_READER_FIRST_AUDIO_STREAM, // [in] DWORD dwStreamIndex,
            0,                                   // [in] DWORD dwControlFlags,
            NULL,                                // [out] DWORD *pdwActualStreamIndex,
            &dwFlags,                            // [out] DWORD *pdwStreamFlags,
            &timestamp,                          // [out] LONGLONG *pllTimestamp,
            &pSample);                           // [out] IMFSample **ppSample
        if (FAILED(hr)) {
            if (sDebug) { qDebug() << "ReadSample failed."; }
            break;
        }

        if (sDebug) {
            qDebug() << "ReadSample timestamp:" << timestamp
                     << "frame:" << frameFromMF(timestamp)
                     << "dwflags:" << dwFlags;
        }

        if (dwFlags & MF_SOURCE_READERF_ERROR) {
            // our source reader is now dead, according to the docs
            qWarning() << "SSMF: ReadSample set ERROR, SourceReader is now dead";
            m_dead = true;
            break;
        } else if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
            qDebug() << "SSMF: End of input file.";
            break;
        } else if (dwFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) {
            qWarning() << "SSMF: Type change";
            break;
        } else if (pSample == NULL) {
            // generally this will happen when dwFlags contains ENDOFSTREAM,
            // so it'll be caught before now -bkgood
            qWarning() << "SSMF: No sample";
            continue;
        } // we now own a ref to the instance at pSample

        IMFMediaBuffer *pMBuffer(NULL);
        // I know this does at least a memcopy and maybe a malloc, if we have
        // xrun issues with this we might want to look into using
        // IMFSample::GetBufferByIndex (although MS doesn't recommend this)
        if (FAILED(hr = pSample->ConvertToContiguousBuffer(&pMBuffer))) {
            error = true;
            goto releaseSample;
        }
        qint16 *buffer(NULL);
        size_t bufferLength(0);
        hr = pMBuffer->Lock(reinterpret_cast<quint8**>(&buffer), NULL,
            reinterpret_cast<DWORD*>(&bufferLength));
        if (FAILED(hr)) {
            error = true;
            goto releaseMBuffer;
        }
        bufferLength /= (kBitsPerSample / 8 * kNumChannels); // now in frames

        if (m_seeking) {
            qint64 bufferPosition(frameFromMF(timestamp));
            if (sDebug) {
                qDebug() << "While seeking to "
                         << m_nextFrame << "WMF put us at" << bufferPosition;

            }
            if (m_nextFrame < bufferPosition) {
                // Uh oh. We are farther forward than our seek target. Emit
                // silence? We can't seek backwards here.
                SAMPLE* pBufferCurpos = destBuffer +
                        (size - framesNeeded * kNumChannels);
                qint64 offshootFrames = bufferPosition - m_nextFrame;

                // If we can correct this immediately, write zeros and adjust
                // m_nextFrame to pretend it never happened.

                if (offshootFrames <= framesNeeded) {
                    qWarning() << __FILE__ << __LINE__
                               << "Working around inaccurate seeking. Writing silence for"
                               << offshootFrames << "frames";
                    // Set offshootFrames * kNumChannels samples to zero.
                    memset(pBufferCurpos, 0,
                           sizeof(*pBufferCurpos) * offshootFrames *
                           kNumChannels);
                    // Now m_nextFrame == bufferPosition
                    m_nextFrame += offshootFrames;
                    framesNeeded -= offshootFrames;
                } else {
                    // It's more complicated. The buffer we have just decoded is
                    // more than framesNeeded frames away from us. It's too hard
                    // for us to handle this correctly currently, so let's just
                    // try to get on with our lives.
                    m_seeking = false;
                    m_nextFrame = bufferPosition;
                    qWarning() << __FILE__ << __LINE__
                               << "Seek offshoot is too drastic. Cutting losses and pretending the current decoded audio buffer is the right seek point.";
                }
            }

            if (m_nextFrame >= bufferPosition &&
                m_nextFrame < bufferPosition + bufferLength) {
                // m_nextFrame is in this buffer.
                buffer += (m_nextFrame - bufferPosition) * kNumChannels;
                bufferLength -= m_nextFrame - bufferPosition;
                m_seeking = false;
            } else {
                // we need to keep going forward
                goto releaseRawBuffer;
            }
        }

        // If the bufferLength is larger than the leftover buffer, re-allocate
        // it with 2x the space.
        if (bufferLength * kNumChannels > m_leftoverBufferSize) {
            int newSize = m_leftoverBufferSize;

            while (newSize < bufferLength * kNumChannels) {
                newSize *= 2;
            }
            qint16* newBuffer = new qint16[newSize];
            memcpy(newBuffer, m_leftoverBuffer,
                   sizeof(m_leftoverBuffer[0]) * m_leftoverBufferSize);
            delete [] m_leftoverBuffer;
            m_leftoverBuffer = newBuffer;
            m_leftoverBufferSize = newSize;
        }
        copyFrames(destBuffer + (size - framesNeeded * kNumChannels),
            &framesNeeded, buffer, bufferLength);

releaseRawBuffer:
        hr = pMBuffer->Unlock();
        // I'm ignoring this, MSDN for IMFMediaBuffer::Unlock stipulates
        // nothing about the state of the instance if this fails so might as
        // well just let it be released.
        //if (FAILED(hr)) break;
releaseMBuffer:
        safeRelease(&pMBuffer);
releaseSample:
        safeRelease(&pSample);
        if (error) break;
    }

    m_nextFrame += framesRequested - framesNeeded;
    if (m_leftoverBufferLength > 0) {
        if (framesNeeded != 0) {
            qWarning() << __FILE__ << __LINE__
                       << "WARNING: Expected frames needed to be 0. Abandoning this file.";
            m_dead = true;
        }
        m_leftoverBufferPosition = m_nextFrame;
    }
    long samples_read = size - framesNeeded * kNumChannels;
    m_iCurrentPosition += samples_read;
    if (sDebug) { qDebug() << "read()" << size << "returning" << samples_read; }
    return samples_read;
}

inline unsigned long SoundSourceMediaFoundation::length()
{
    unsigned long len(secondsFromMF(m_mfDuration) * kSampleRate * kNumChannels);
    return len % kNumChannels == 0 ? len : len + 1;
}

int SoundSourceMediaFoundation::parseHeader()
{
    setType("m4a");

    // Must be toLocal8Bit since Windows fopen does not do UTF-8
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
// configureAudioStream
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
bool SoundSourceMediaFoundation::configureAudioStream()
{
    HRESULT hr(S_OK);

    // deselect all streams, we only want the first
    hr = m_pReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, false);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to deselect all streams";
        return false;
    }

    hr = m_pReader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, true);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to select first audio stream";
        return false;
    }

    hr = MFCreateMediaType(&m_pAudioType);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to create media type";
        return false;
    }

    hr = m_pAudioType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to set major type";
        return false;
    }

    hr = m_pAudioType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to set subtype";
        return false;
    }

    hr = m_pAudioType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, true);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to set samples independent";
        return false;
    }

    hr = m_pAudioType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, true);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to set fixed size samples";
        return false;
    }

    hr = m_pAudioType->SetUINT32(MF_MT_SAMPLE_SIZE, kLeftoverSize);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to set sample size";
        return false;
    }

    // MSDN for this attribute says that if bps is 8, samples are unsigned.
    // Otherwise, they're signed (so they're signed for us as 16 bps). Why
    // chose to hide this rather useful tidbit here is beyond me -bkgood
    hr = m_pAudioType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, kBitsPerSample);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to set bits per sample";
        return false;
    }

    hr = m_pAudioType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT,
        kNumChannels * (kBitsPerSample / 8));
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to set block alignment";
        return false;
    }

    hr = m_pAudioType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, kNumChannels);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to set number of channels";
        return false;
    }

    hr = m_pAudioType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, kSampleRate);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to set sample rate";
        return false;
    }

    // Set this type on the source reader. The source reader will
    // load the necessary decoder.
    hr = m_pReader->SetCurrentMediaType(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        NULL, m_pAudioType);

    // the reader has the media type now, free our reference so we can use our
    // pointer for other purposes. Do this before checking for failure so we
    // don't dangle.
    safeRelease(&m_pAudioType);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to set media type";
        return false;
    }

    // Get the complete uncompressed format.
    hr = m_pReader->GetCurrentMediaType(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        &m_pAudioType);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to retrieve completed media type";
        return false;
    }

    // Ensure the stream is selected.
    hr = m_pReader->SetStreamSelection(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        true);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to select first audio stream (again)";
        return false;
    }

    // this may not be safe on all platforms as m_leftoverBufferSize is a
    // size_t and this function is writing a uint32. However, on 32-bit
    // Windows 7, size_t is defined as uint which is 32-bits, so we're safe
    // for all supported platforms -bkgood
    UINT32 leftoverBufferSize = 0;
    hr = m_pAudioType->GetUINT32(MF_MT_SAMPLE_SIZE, &leftoverBufferSize);
    if (FAILED(hr)) {
        qWarning() << "SSMF: failed to get buffer size";
        return false;
    }
    m_leftoverBufferSize = static_cast<size_t>(leftoverBufferSize);
    m_leftoverBufferSize /= 2; // convert size in bytes to size in int16s
    m_leftoverBuffer = new qint16[m_leftoverBufferSize];

    return true;
}

bool SoundSourceMediaFoundation::readProperties()
{
    PROPVARIANT prop;
    HRESULT hr = S_OK;

    //Get the duration, provided as a 64-bit integer of 100-nanosecond units
    hr = m_pReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE,
        MF_PD_DURATION, &prop);
    if (FAILED(hr)) {
        qWarning() << "SSMF: error getting duration";
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
void SoundSourceMediaFoundation::copyFrames(
    qint16 *dest, size_t *destFrames, const qint16 *src, size_t srcFrames)
{
    if (srcFrames > *destFrames) {
        int samplesToCopy(*destFrames * kNumChannels);
        memcpy(dest, src, samplesToCopy * sizeof(*src));
        srcFrames -= *destFrames;
        memmove(m_leftoverBuffer,
            src + samplesToCopy,
            srcFrames * kNumChannels * sizeof(*src));
        *destFrames = 0;
        m_leftoverBufferLength = srcFrames;
    } else {
        int samplesToCopy(srcFrames * kNumChannels);
        memcpy(dest, src, samplesToCopy * sizeof(*src));
        *destFrames -= srcFrames;
        if (src == m_leftoverBuffer) {
            m_leftoverBufferLength = 0;
        }
    }
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
