#ifndef MP4V2_ISMA_H
#define MP4V2_ISMA_H

/**************************************************************************//**
 *
 *  @defgroup mp4_isma MP4v2 ISMA (Internet Streaming Media Alliance)
 *  @{
 *
 *****************************************************************************/

/** something */
typedef struct mp4v2_ismacryp_session_params {
    uint32_t    scheme_type;
    uint16_t    scheme_version;
    uint8_t     key_ind_len;
    uint8_t     iv_len;
    uint8_t     selective_enc;
    const char* kms_uri;
} mp4v2_ismacrypParams;

/*
 * API to initialize ismacryp properties to sensible defaults
 * if input param is null then mallocs a params struct
 */

MP4V2_EXPORT
mp4v2_ismacrypParams* MP4DefaultISMACrypParams( mp4v2_ismacrypParams* ptr );

MP4V2_EXPORT
MP4TrackId MP4AddEncAudioTrack(
    MP4FileHandle         hFile,
    uint32_t              timeScale,
    MP4Duration           sampleDuration,
    mp4v2_ismacrypParams* icPp,
    uint8_t               audioType DEFAULT(MP4_MPEG4_AUDIO_TYPE) );

MP4V2_EXPORT
MP4TrackId MP4AddEncVideoTrack(
    MP4FileHandle         hFile,
    uint32_t              timeScale,
    MP4Duration           sampleDuration,
    uint16_t              width,
    uint16_t              height,
    mp4v2_ismacrypParams* icPp,
    uint8_t               videoType DEFAULT(MP4_MPEG4_VIDEO_TYPE),
    const char*           oFormat DEFAULT(NULL) );

MP4V2_EXPORT
MP4TrackId MP4AddEncH264VideoTrack(
    MP4FileHandle         dstFile,
    uint32_t              timeScale,
    MP4Duration           sampleDuration,
    uint16_t              width,
    uint16_t              height,
    MP4FileHandle         srcFile,
    MP4TrackId            srcTrackId,
    mp4v2_ismacrypParams* icPp );

MP4V2_EXPORT
MP4TrackId MP4EncAndCloneTrack(
    MP4FileHandle         srcFile,
    MP4TrackId            srcTrackId,
    mp4v2_ismacrypParams* icPp,
    MP4FileHandle         dstFile DEFAULT(MP4_INVALID_FILE_HANDLE),
    MP4TrackId            dstHintTrackReferenceTrack DEFAULT(MP4_INVALID_TRACK_ID) );

MP4V2_EXPORT
MP4TrackId MP4EncAndCopyTrack(
    MP4FileHandle         srcFile,
    MP4TrackId            srcTrackId,
    mp4v2_ismacrypParams* icPp,
    encryptFunc_t         encfcnp,
    uint32_t              encfcnparam1,
    MP4FileHandle         dstFile DEFAULT(MP4_INVALID_FILE_HANDLE),
    bool                  applyEdits DEFAULT(false),
    MP4TrackId            dstHintTrackReferenceTrack DEFAULT(MP4_INVALID_TRACK_ID) );

MP4V2_EXPORT
bool MP4MakeIsmaCompliant(
    const char* fileName,
    uint32_t    verbosity DEFAULT(0),
    bool        addIsmaComplianceSdp DEFAULT(true) );

MP4V2_EXPORT
char* MP4MakeIsmaSdpIod(
    uint8_t  videoProfile,
    uint32_t videoBitrate,
    uint8_t* videoConfig,
    uint32_t videoConfigLength,
    uint8_t  audioProfile,
    uint32_t audioBitrate,
    uint8_t* audioConfig,
    uint32_t audioConfigLength,
    uint32_t verbosity DEFAULT(0) );

/** @} ***********************************************************************/

#endif /* MP4V2_ISMA_H */
