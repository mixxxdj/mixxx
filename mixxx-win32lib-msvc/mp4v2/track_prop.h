#ifndef MP4V2_TRACK_PROP_H
#define MP4V2_TRACK_PROP_H

/**************************************************************************//**
 *
 *  @defgroup mp4_track_prop MP4v2 Track Property
 *  @{
 *
 *****************************************************************************/

/* specific track properties */

MP4V2_EXPORT
bool MP4HaveTrackAtom(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    const char*   atomname );

/** Get the track type.
 *
 *  MP4GetTrackType gets the type of the track with the specified track id.
 *
 *  Note: the library does not provide a MP4SetTrackType function, the
 *  track type needs to be specified when the track is created, e.g.
 *  MP4AddSystemsTrack(MP4_OCI_TRACK_TYPE).
 *
 *  Known track types are:
 *      @li #MP4_OD_TRACK_TYPE
 *      @li #MP4_SCENE_TRACK_TYPE
 *      @li #MP4_AUDIO_TRACK_TYPE
 *      @li #MP4_VIDEO_TRACK_TYPE
 *      @li #MP4_HINT_TRACK_TYPE
 *      @li #MP4_CNTL_TRACK_TYPE
 *      @li #MP4_TEXT_TRACK_TYPE
 *      @li #MP4_CLOCK_TRACK_TYPE
 *      @li #MP4_MPEG7_TRACK_TYPE
 *      @li #MP4_OCI_TRACK_TYPE
 *      @li #MP4_IPMP_TRACK_TYPE
 *      @li #MP4_MPEGJ_TRACK_TYPE
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *
 *  @return On success, a string indicating track type. On failure, NULL.
 */
MP4V2_EXPORT
const char* MP4GetTrackType(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

MP4V2_EXPORT
const char* MP4GetTrackMediaDataName(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

/*
 * MP4GetTrackMediaDataOriginalFormat is to be used to get the original
 * MediaDataName if a track has been encrypted.
 */

MP4V2_EXPORT
bool MP4GetTrackMediaDataOriginalFormat(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    char*         originalFormat,
    uint32_t      buflen );

MP4V2_EXPORT
MP4Duration MP4GetTrackDuration(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

/** Get the time scale of a track.
 *
 *  MP4GetTrackTimeScale returns the time scale of the specified track in
 *  the mp4 file. The time scale determines the number of clock ticks per
 *  second for this track.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *
 *  @return timescale (ticks per second) of the track in the mp4 file.
 */
MP4V2_EXPORT
uint32_t MP4GetTrackTimeScale(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

/** Set the time scale of a track.
 *
 *  MP4SetTrackTimeScale sets the time scale of the specified track in the
 *  mp4 file. The time scale determines the number of clock ticks per
 *  second for this track.
 *
 *  Typically this value is set once when the track is created. However
 *  this call can be used to modify the value if that is desired. Since
 *  track sample durations are expressed in units of the track time scale,
 *  any change to the time scale value will effect the real time duration
 *  of the samples.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param timeScale desired time scale for the track.
 */
MP4V2_EXPORT
void MP4SetTrackTimeScale(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    uint32_t      value );

/** Get ISO-639-2/T langauge code of a track.
 *  The language code is a 3-char alpha code consisting of lower-case letters.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param code buffer to hold 3-char+null (4-bytes total).
 *
 *  @return <b>true</b> on success, <b>false</b> on failure.
 */
MP4V2_EXPORT
bool MP4GetTrackLanguage(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    char*         code );

/** Set ISO-639-2/T langauge code of a track.
 *  The language code is a 3-char alpha code consisting of lower-case letters.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param code 3-char language code.
 *
 *  @return <b>true</b> on success, <b>false</b> on failure.
 */
MP4V2_EXPORT
bool MP4SetTrackLanguage(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    const char*   code );

/** Get track name.
 *
 *  MP4GetTrackName gets the name of the track via udta.name property.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *
 *  @return <b>true</b> on success, <b>false</b> on failure.
 */
MP4V2_EXPORT
bool MP4GetTrackName(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    char**        name );

/** Set track name.
 *
 *  MP4SetTrackName sets the name of the track via udta.name property.
 *  The udta atom is created if needed.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *
 *  @return <b>true</b> on success, <b>false</b> on failure.
 */
MP4V2_EXPORT
bool MP4SetTrackName(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    const char*   name );

MP4V2_EXPORT
uint8_t MP4GetTrackAudioMpeg4Type(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

MP4V2_EXPORT
uint8_t MP4GetTrackEsdsObjectTypeId(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

/* returns MP4_INVALID_DURATION if track samples do not have a fixed duration */
MP4V2_EXPORT
MP4Duration MP4GetTrackFixedSampleDuration(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

MP4V2_EXPORT
uint32_t MP4GetTrackBitRate(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

MP4V2_EXPORT
bool MP4GetTrackVideoMetadata(
    MP4FileHandle hFile,
    MP4TrackId trackId,
    uint8_t**  ppConfig,
    uint32_t*  pConfigSize );

MP4V2_EXPORT
bool MP4GetTrackESConfiguration(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    uint8_t**     ppConfig,
    uint32_t*     pConfigSize );

MP4V2_EXPORT
bool MP4SetTrackESConfiguration(
    MP4FileHandle  hFile,
    MP4TrackId     trackId,
    const uint8_t* pConfig,
    uint32_t       configSize );

/* h264 information routines */
MP4V2_EXPORT
bool MP4GetTrackH264ProfileLevel(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    uint8_t*      pProfile,
    uint8_t*      pLevel );

MP4V2_EXPORT
void MP4GetTrackH264SeqPictHeaders(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    uint8_t***    pSeqHeaders,
    uint32_t**    pSeqHeaderSize,
    uint8_t***    pPictHeader,
    uint32_t**    pPictHeaderSize );

MP4V2_EXPORT
bool MP4GetTrackH264LengthSize(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    uint32_t*     pLength );

MP4V2_EXPORT
MP4SampleId MP4GetTrackNumberOfSamples(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

MP4V2_EXPORT
uint16_t MP4GetTrackVideoWidth(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

MP4V2_EXPORT
uint16_t MP4GetTrackVideoHeight(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

MP4V2_EXPORT
double MP4GetTrackVideoFrameRate(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

MP4V2_EXPORT
int MP4GetTrackAudioChannels(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

MP4V2_EXPORT
bool MP4IsIsmaCrypMediaTrack(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

/* generic track properties */

MP4V2_EXPORT
bool MP4HaveTrackAtom(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    const char*   atomName );

MP4V2_EXPORT
bool MP4GetTrackIntegerProperty(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    const char*   propName,
    uint64_t*     retvalue );

MP4V2_EXPORT
bool MP4GetTrackFloatProperty(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    const char*   propName,
    float*        ret_value );

MP4V2_EXPORT
bool MP4GetTrackStringProperty(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    const char*   propName,
    const char**  retvalue );

MP4V2_EXPORT
bool MP4GetTrackBytesProperty(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    const char*   propName,
    uint8_t**     ppValue,
    uint32_t*     pValueSize );

MP4V2_EXPORT
bool MP4SetTrackIntegerProperty(
    MP4FileHandle hFile,
    MP4TrackId   trackId,
    const char*  propName,
    int64_t      value );

MP4V2_EXPORT
bool MP4SetTrackFloatProperty(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    const char*   propName,
    float         value );

MP4V2_EXPORT
bool MP4SetTrackStringProperty(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    const char*   propName,
    const char*   value );

MP4V2_EXPORT
bool MP4SetTrackBytesProperty(
    MP4FileHandle  hFile,
    MP4TrackId     trackId,
    const char*    propName,
    const uint8_t* pValue,
    uint32_t       valueSize);

/** @} ***********************************************************************/

#endif /* MP4V2_TRACK_PROP_H */
