/***************************************************************************
 soundsourcem4a.cpp  -  mp4/m4a decoder
 -------------------
 copyright            : (C) 2008 by Garth Dahlstrom
 email                : ironstorm@users.sf.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "soundsourcem4a.h"

#include "trackmetadatataglib.h"
#include "sampleutil.h"

#include <taglib/mp4file.h>

#ifdef __WINDOWS__
#include <io.h>
#include <fcntl.h>
#endif

#ifdef _MSC_VER
#define S_ISDIR(mode) (mode & _S_IFDIR)
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

#ifdef __M4AHACK__
typedef uint32_t SAMPLERATE_TYPE;
#else
typedef unsigned long SAMPLERATE_TYPE;
#endif

namespace Mixxx {

namespace {
// AAC: "... each block decodes to 1024 time-domain samples."
const AudioSource::size_type kFramesPerSampleBlock = 1024;

// MP4SampleId is 1-based
const MP4SampleId kMinSampleId = 1;

// Decoding will be restarted one or more blocks of samples
// before the actual position to avoid audible glitches.
// One block of samples seems to be enough here.
const MP4SampleId kSampleIdPrefetchCount = 1;
}

SoundSourceM4A::SoundSourceM4A(QString qFileName)
        : Super(qFileName, "m4a"), m_hFile(MP4_INVALID_FILE_HANDLE), m_trackId(
                MP4_INVALID_TRACK_ID), m_maxSampleId(MP4_INVALID_SAMPLE_ID), m_curSampleId(
                MP4_INVALID_SAMPLE_ID), m_inputBufferOffset(0), m_inputBufferLength(
                0), m_hDecoder(NULL), m_curFrameIndex(0) {
}

SoundSourceM4A::~SoundSourceM4A() {
    close();
}

namespace {
MP4TrackId findAacTrackId(MP4FileHandle hFile) {
    const MP4TrackId maxTrackId = MP4GetNumberOfTracks(hFile, NULL, 0);
    for (MP4TrackId trackId = 1; trackId <= maxTrackId; ++trackId) {
        const char* trackType = MP4GetTrackType(hFile, trackId);
        if ((NULL == trackType) || !MP4_IS_AUDIO_TRACK_TYPE(trackType)) {
            continue;
        }
        const char* mediaDataName = MP4GetTrackMediaDataName(hFile, trackId);
        if (0 != strcasecmp(mediaDataName, "mp4a")) {
            continue;
        }
        const u_int8_t audioType = MP4GetTrackEsdsObjectTypeId(hFile, trackId);
        if (MP4_INVALID_AUDIO_TYPE == audioType) {
            continue;
        }
        if (MP4_MPEG4_AUDIO_TYPE == audioType) {
            const u_int8_t audioMpeg4Type = MP4GetTrackAudioMpeg4Type(hFile,
                    trackId);
            if (MP4_IS_MPEG4_AAC_AUDIO_TYPE(audioMpeg4Type)) {
                return trackId;
            }
        } else if (MP4_IS_AAC_AUDIO_TYPE(audioType)) {
            return trackId;
        }
    }
    return MP4_INVALID_TRACK_ID;
}
}

Result SoundSourceM4A::open() {
    if (MP4_INVALID_FILE_HANDLE != m_hFile) {
        qWarning() << "File has already been opened!";
        return ERR;
    }

    /* open MP4 file, check for >= ver 1.9.1 */
    const QByteArray qbaFilename(getFilename().toLocal8Bit());
#if MP4V2_PROJECT_version_hex <= 0x00010901
    m_hFile = MP4Read(qbaFilename.constData(), 0);
#else
    m_hFile = MP4Read(qbaFilename.constData());
#endif
    if (MP4_INVALID_FILE_HANDLE == m_hFile) {
        close();
        qWarning() << "Failed to open file for reading:" << getFilename();
        return ERR;
    }

    m_trackId = findAacTrackId(m_hFile);
    if (MP4_INVALID_TRACK_ID == m_trackId) {
        close();
        qWarning() << "No AAC track found in file:" << getFilename();
        return ERR;
    }

    m_maxSampleId = MP4GetTrackNumberOfSamples(m_hFile, m_trackId);
    if (MP4_INVALID_SAMPLE_ID == m_maxSampleId) {
        close();
        qWarning() << "Failed to read file structure:" << getFilename();
        return ERR;
    }
    m_curSampleId = MP4_INVALID_SAMPLE_ID;

    m_inputBuffer.resize(MP4GetTrackMaxSampleSize(m_hFile, m_trackId), 0);
    m_inputBufferOffset = 0;
    m_inputBufferLength = 0;

    if (!m_hDecoder) {
        // lazy initialization of the AAC decoder
        m_hDecoder = faacDecOpen();
        if (!m_hDecoder) {
            qWarning() << "Failed to open the AAC decoder!";
            close();
            return ERR;
        }
        faacDecConfigurationPtr pDecoderConfig = faacDecGetCurrentConfiguration(
                m_hDecoder);
        pDecoderConfig->outputFormat = FAAD_FMT_FLOAT; /* 32-bit float */
        pDecoderConfig->downMatrix = 1; /* 5.1 -> stereo */
        pDecoderConfig->defObjectType = LC;
        if (!faacDecSetConfiguration(m_hDecoder, pDecoderConfig)) {
            close();
            qWarning() << "Failed to configure AAC decoder!";
            return ERR;
        }
    }

    u_int8_t* configBuffer = NULL;
    u_int32_t configBufferSize = 0;
    if (!MP4GetTrackESConfiguration(m_hFile, m_trackId, &configBuffer,
            &configBufferSize)) {
        /* failed to get mpeg-4 audio config... this is ok.
         * faacDecInit2() will simply use default values instead.
         */
        qWarning()
                << "Failed to read the MP4 audio configuration. Continuing with default values.";
    }

    SAMPLERATE_TYPE sampleRate;
    unsigned char channelCount;
    if (0 > faacDecInit2(m_hDecoder, configBuffer, configBufferSize,
                    &sampleRate, &channelCount)) {
        free(configBuffer);
        close();
        qWarning() << "Failed to initialize the AAC decoder!";
        return ERR;
    } else {
        free(configBuffer);
    }

    setChannelCount(channelCount);
    setFrameRate(sampleRate);
    setFrameCount(m_maxSampleId * kFramesPerSampleBlock);

    const SampleBuffer::size_type prefetchSampleBufferSize = (kSampleIdPrefetchCount + 1) * frames2samples(kFramesPerSampleBlock);
    SampleBuffer(prefetchSampleBufferSize).swap(m_prefetchSampleBuffer);

    // invalidate current frame index
    m_curFrameIndex = getFrameCount();
    // seek to beginning of file
    if (0 != seekFrame(0)) {
        close();
        qWarning() << "Failed to seek to the beginning of the file!";
        return ERR;
    }

    return OK;
}

void SoundSourceM4A::close() {
    if (m_hDecoder) {
        NeAACDecClose(m_hDecoder);
        m_hDecoder = NULL;
    }
    if (MP4_INVALID_FILE_HANDLE != m_hFile) {
        MP4Close(m_hFile);
        m_hFile = MP4_INVALID_FILE_HANDLE;
    }
    m_trackId = MP4_INVALID_TRACK_ID;
    m_maxSampleId = MP4_INVALID_SAMPLE_ID;
    m_curSampleId = MP4_INVALID_SAMPLE_ID;
    m_inputBuffer.clear();
    m_inputBufferOffset = 0;
    m_inputBufferLength = 0;
    m_curFrameIndex = 0;
    Super::reset();
}

bool SoundSourceM4A::isValidSampleId(MP4SampleId sampleId) const {
    return (sampleId >= kMinSampleId) && (sampleId <= m_maxSampleId);
}

AudioSource::diff_type SoundSourceM4A::seekFrame(diff_type frameIndex) {
    if (m_curFrameIndex != frameIndex) {
        const MP4SampleId sampleId = kMinSampleId
                + (frameIndex / kFramesPerSampleBlock);
        if (!isValidSampleId(sampleId)) {
            return m_curFrameIndex;
        }
        if ((m_curSampleId != sampleId) || (frameIndex < m_curFrameIndex)) {
            // restart decoding one or more blocks of samples
            // backwards to avoid audible glitches
            m_curSampleId = math_max(sampleId - kSampleIdPrefetchCount, kMinSampleId);
            m_curFrameIndex = (m_curSampleId - kMinSampleId) * kFramesPerSampleBlock;
            // rryan 9/2009 -- the documentation is sketchy on this, but I think that
            // it tells the decoder that you are seeking so it should flush its state
            faacDecPostSeekReset(m_hDecoder, m_curSampleId);
            // discard input buffer
            m_inputBufferOffset = 0;
            m_inputBufferLength = 0;
        }
        // decoding starts before the actual target position
        Q_ASSERT(m_curFrameIndex <= frameIndex);
        const size_type prefetchFrameCount = frameIndex - m_curFrameIndex;
        // prefetch (decode and discard) all samples up to the target position
        Q_ASSERT(frames2samples(prefetchFrameCount) <= m_prefetchSampleBuffer.size());
        readFrameSamplesInterleaved(prefetchFrameCount, &m_prefetchSampleBuffer[0]);
    }
    Q_ASSERT(m_curFrameIndex == frameIndex);
    return frameIndex;
}

AudioSource::size_type SoundSourceM4A::readFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer) {
    sample_type* pSampleBuffer = sampleBuffer;
    const diff_type readFrameIndex = m_curFrameIndex;
    size_type readFrameCount = m_curFrameIndex - readFrameIndex;
    while ((readFrameCount = m_curFrameIndex - readFrameIndex) < frameCount) {
        if (isValidSampleId(m_curSampleId) && (0 == m_inputBufferLength)) {
            // fill input buffer with block of samples
            InputBuffer::value_type* pInputBuffer = &m_inputBuffer[0];
            m_inputBufferOffset = 0;
            m_inputBufferLength = m_inputBuffer.size(); // in/out parameter
            if (!MP4ReadSample(m_hFile, m_trackId, m_curSampleId++,
                    &pInputBuffer, &m_inputBufferLength,
                    NULL, NULL, NULL, NULL)) {
                qWarning() << "Failed to read MP4 sample data" << m_curSampleId;
                break; // abort
            }
        }
        if (0 == m_inputBufferLength) {
            // no more input data available (EOF)
            break;// done
        }
        faacDecFrameInfo decFrameInfo;
        decFrameInfo.bytesconsumed = 0;
        decFrameInfo.samples = 0;
        // decode samples into sampleBuffer
        const size_type readFrameCount = m_curFrameIndex - readFrameIndex;
        const size_type decodeBufferCapacityInBytes = frames2samples(
                frameCount - readFrameCount) * sizeof(*sampleBuffer);
        Q_ASSERT(0 < decodeBufferCapacityInBytes);
        void* pDecodeBuffer = pSampleBuffer;
        NeAACDecDecode2(m_hDecoder, &decFrameInfo,
                &m_inputBuffer[m_inputBufferOffset],
                m_inputBufferLength / sizeof(m_inputBuffer[0]),
                &pDecodeBuffer, decodeBufferCapacityInBytes);
        // samples should have been decoded into our own buffer
        Q_ASSERT(pSampleBuffer == pDecodeBuffer);
        pSampleBuffer += decFrameInfo.samples;
        // only the input data that is available should have been read
        Q_ASSERT(
                decFrameInfo.bytesconsumed
                        <= (m_inputBufferLength / sizeof(m_inputBuffer[0])));
        // consume input data
        m_inputBufferOffset += decFrameInfo.bytesconsumed
                * sizeof(m_inputBuffer[0]);
        m_inputBufferLength -= decFrameInfo.bytesconsumed
                * sizeof(m_inputBuffer[0]);
        m_curFrameIndex += samples2frames(decFrameInfo.samples);
        if (0 != decFrameInfo.error) {
            qWarning() << "AAC decoding error:"
                    << faacDecGetErrorMessage(decFrameInfo.error);
            break; // abort
        }
        if (getChannelCount() != decFrameInfo.channels) {
            qWarning() << "Unexpected number of channels:"
                    << decFrameInfo.channels;
            break; // abort
        }
        if (getFrameRate() != decFrameInfo.samplerate) {
            qWarning() << "Unexpected sample rate:" << decFrameInfo.samplerate;
            break; // abort
        }
    }
    return readFrameCount;
}

Result SoundSourceM4A::parseMetadata(Mixxx::TrackMetadata* pMetadata) {
    TagLib::MP4::File f(getFilename().toLocal8Bit().constData());

    if (!readAudioProperties(pMetadata, f)) {
        return ERR;
    }

    TagLib::MP4::Tag *mp4(f.tag());
    if (mp4) {
        readMP4Tag(pMetadata, *mp4);
    } else {
        // fallback
        const TagLib::Tag *tag(f.tag());
        if (tag) {
            readTag(pMetadata, *tag);
        } else {
            return ERR;
        }
    }

    return OK;
}

QImage SoundSourceM4A::parseCoverArt() {
    TagLib::MP4::File f(getFilename().toLocal8Bit().constData());
    TagLib::MP4::Tag *mp4(f.tag());
    if (mp4) {
        return readMP4TagCover(*mp4);
    } else {
        return QImage();
    }
}

QList<QString> SoundSourceM4A::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("m4a");
    list.push_back("mp4");
    return list;
}

}
