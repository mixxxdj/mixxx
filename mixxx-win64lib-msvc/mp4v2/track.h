#ifndef MP4V2_TRACK_H
#define MP4V2_TRACK_H

/**************************************************************************//**
 *
 *  @defgroup mp4_track MP4v2 Track
 *  @{
 *
 *****************************************************************************/

/** Add a user defined track.
 *
 *  MP4AddTrack adds a user defined track to the mp4 file. Care should be
 *  taken to avoid any of the standardized track type names. A useful
 *  convention is use only uppercase characters for user defined track types.
 *  The string should be exactly four characters in length, e.g. "MINE".
 *
 *  Note this should not be used to add any of the known track types defined
 *  in the MP4 standard (ISO/IEC 14496−1:2001).
 *
 *  @param hFile handle of file for operation.
 *  @param type specifies the type of track to be added.
 *
 *  @return On success, the track-id of new track.
 *      On failure, #MP4_INVALID_TRACK_ID.
 */
MP4V2_EXPORT
MP4TrackId MP4AddTrack(
    MP4FileHandle hFile,
    const char*   type );

/** Add an MPEG−4 systems track.
 *
 *  MP4AddSystemsTrack adds an MPEG−4 Systems track to the mp4 file. Note
 *  this should not be used to add OD or scene tracks, MP4AddODTrack() and
 *  MP4AddSceneTrack() should be used for those purposes. Other known
 *  MPEG−4 System track types are:
 *      @li #MP4_CLOCK_TRACK_TYPE
 *      @li #MP4_MPEG7_TRACK_TYPE
 *      @li #MP4_OCI_TRACK_TYPE
 *      @li #MP4_IPMP_TRACK_TYPE
 *      @li #MP4_MPEGJ_TRACK_TYPE
 *
 *  @param hFile handle of file for operation.
 *  @param type specifies the type of track to be added.
 *
 *  @return On success, the track-id of new track.
 *      On failure, #MP4_INVALID_TRACK_ID.
 */
MP4V2_EXPORT
MP4TrackId MP4AddSystemsTrack(
    MP4FileHandle hFile,
    const char*   type );

/** Add a object descriptor (OD) track.
 *
 *  MP4AddODTrack adds an object descriptor (aka OD) track to the mp4 file.
 *  MP4WriteSample() can then be used to add the desired OD commands to the
 *  track. The burden is currently on the calling application to understand
 *  OD.
 *
 *  Those wishing to have a simple audio/video scene without understanding
 *  OD may wish to use MP4MakeIsmaCompliant() to create the minimal OD and
 *  BIFS information.
 *
 *  @param hFile handle of file for operation.
 * 
 *  @return On success, the track-id of new track.
 *      On failure, #MP4_INVALID_TRACK_ID.
 */
MP4V2_EXPORT
MP4TrackId MP4AddODTrack(
    MP4FileHandle hFile );

/** Add a scene (BIFS) track.
 *
 *  MP4AddSceneTrack adds a scene (aka BIFS) track to the mp4 file.
 *  MP4WriteSample() can then be used to add the desired BIFS commands to
 *  the track. The burden is currently on the calling application to
 *  understand BIFS.
 *
 *  Those  wishing to have a simple audio/video scene without understanding
 *  BIFS may wish to use MP4MakeIsmaCompliant() to create the minimal OD
 *  and BIFS information.
 *
 *  @param hFile handle of file for operation.
 * 
 *  @return On success, the track-id of new track.
 *      On failure, #MP4_INVALID_TRACK_ID.
 */
MP4V2_EXPORT
MP4TrackId MP4AddSceneTrack(
    MP4FileHandle hFile );

/** Add audio track to mp4 file.
 *
 *  MP4AddAudioTrack adds an audio track to the mp4 file. MP4WriteSample()
 *  can then be used to add the desired audio samples.
 *
 *  It is recommended that the time scale be set to the sampling frequency
 *  (eg. 44100 Hz) of the audio so as to preserve the timing information
 *  accurately.
 *
 *  If the audio encoding uses a fixed duration for each sample that should
 *  be specified here. If not then the value #MP4_INVALID_DURATION
 *  should be given for the sampleDuration argument.
 *
 *  @param hFile handle of file for operation.
 *  @param timeScale the time scale in ticks per second of the track.
 *  @param sampleDuration the fixed  duration for all track samples.
 *      Caveat: the value should be in track-timescale units.
 *  @param audioType the audio encoding type.
 *      See MP4GetTrackEsdsObjectTypeId() for known values.
 *
 *  @return On success, the track-id of the new track.
 *      On error, #MP4_INVALID_TRACK_ID.
 */
MP4V2_EXPORT
MP4TrackId MP4AddAudioTrack(
    MP4FileHandle hFile,
    uint32_t      timeScale,
    MP4Duration   sampleDuration,
    uint8_t       audioType DEFAULT(MP4_MPEG4_AUDIO_TYPE) );

MP4V2_EXPORT
MP4TrackId MP4AddAC3AudioTrack(
    MP4FileHandle hFile,
    uint32_t      samplingRate,
    uint8_t       fscod,
    uint8_t       bsid,
    uint8_t       bsmod,
    uint8_t       acmod,
    uint8_t       lfeon,
    uint8_t       bit_rate_code );

MP4V2_EXPORT
MP4TrackId MP4AddAmrAudioTrack(
    MP4FileHandle hFile,
    uint32_t      timeScale,
    uint16_t      modeSet,
    uint8_t       modeChangePeriod,
    uint8_t       framesPerSample,
    bool          isAmrWB );

MP4V2_EXPORT
void MP4SetAmrVendor(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    uint32_t      vendor );

MP4V2_EXPORT
void MP4SetAmrDecoderVersion(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    uint8_t       decoderVersion );

MP4V2_EXPORT
void MP4SetAmrModeSet(
    MP4FileHandle hFile,
    MP4TrackId    trakId,
    uint16_t      modeSet );

MP4V2_EXPORT
uint16_t MP4GetAmrModeSet(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

MP4V2_EXPORT
MP4TrackId MP4AddHrefTrack(
    MP4FileHandle hFile,
    uint32_t      timeScale,
    MP4Duration   sampleDuration,
    const char*   base_url DEFAULT(NULL) );

MP4V2_EXPORT
const char* MP4GetHrefTrackBaseUrl(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

/** Add a video track.
 *
 *  MP4AddVideoTrack adds a video track to the mp4 file. MP4WriteSample()
 *  can then be used to add the desired video samples.
 *
 *  It is recommended that the time scale be set to 90000 so as to preserve
 *  the timing information accurately for the range of video frame rates
 *  commonly in use.
 *
 *  If the video frame rate is to be fixed then the sampleDuration argument
 *  should be give the appropriate fixed value. If the video frame rate is
 *  to be variable then the value #MP4_INVALID_DURATION should be
 *  given for the sampleDuration argument.
 *
 *  @param hFile handle of file for operation.
 *  @param timeScale the timescale in ticks per second of the track.
 *  @param sampleDuration specifies fixed sample duration for all track
 *      samples. Caveat: the value should be in track timescale units.
 *  @param width specifies the video frame width in pixels.
 *  @param height specifies the video frame height in pixels.
 *  @param videoType specifies the video encoding type.
 *      See MP4GetTrackVideoType() for known values.
 *
 *  @return On success, the track-id of the new track.
 *      On error, #MP4_INVALID_TRACK_ID.
 */
MP4V2_EXPORT
MP4TrackId MP4AddVideoTrack(
    MP4FileHandle hFile,
    uint32_t      timeScale,
    MP4Duration   sampleDuration,
    uint16_t      width,
    uint16_t      height,
    uint8_t       videoType DEFAULT(MP4_MPEG4_VIDEO_TYPE) );

MP4V2_EXPORT
MP4TrackId MP4AddH264VideoTrack(
    MP4FileHandle hFile,
    uint32_t      timeScale,
    MP4Duration   sampleDuration,
    uint16_t      width,
    uint16_t      height,
    uint8_t       AVCProfileIndication,
    uint8_t       profile_compat,
    uint8_t       AVCLevelIndication,
    uint8_t       sampleLenFieldSizeMinusOne );

MP4V2_EXPORT
void MP4AddH264SequenceParameterSet(
    MP4FileHandle  hFile,
    MP4TrackId     trackId,
    const uint8_t* pSequence,
    uint16_t       sequenceLen );

MP4V2_EXPORT
void MP4AddH264PictureParameterSet(
    MP4FileHandle  hFile,
    MP4TrackId     trackId,
    const uint8_t* pPict,
    uint16_t       pictLen );

MP4V2_EXPORT
void MP4SetH263Vendor(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    uint32_t      vendor );

MP4V2_EXPORT
void MP4SetH263DecoderVersion(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    uint8_t       decoderVersion );

MP4V2_EXPORT
void MP4SetH263Bitrates(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    uint32_t      avgBitrate,
    uint32_t      maxBitrate );

MP4V2_EXPORT
MP4TrackId MP4AddH263VideoTrack(
    MP4FileHandle hFile,
    uint32_t      timeScale,
    MP4Duration   sampleDuration,
    uint16_t      width,
    uint16_t      height,
    uint8_t       h263Level,
    uint8_t       h263Profile,
    uint32_t      avgBitrate,
    uint32_t      maxBitrate );

/** Add a hint track.
 *
 *  MP4AddHintTrack adds a hint track to the mp4 file. A hint track is used
 *  to describe how to send the reference media track over a particular
 *  network transport. In the case of the IETF RTP protocol, the hint track
 *  describes how the media data should be placed into packets and any
 *  media specific protocol headers that should be added.
 *
 *  Typically there is a one to one correspondence between reference  media
 *  track  samples  and  hint track samples. The start time, duration, and
 *  sync flags are typically the same, however provisions are made for
 *  deviations from this rule.
 *
 *  The MP4 library provides extensive support for RTP hint tracks. This
 *  includes a easy to use API to create RTP hint tracks, and read out
 *  fully constructed RTP packets based on the hint track.
 *
 *  @param hFile handle of file for operation.
 *  @param refTrackId specifies the reference media track for this hint track.
 *
 *  @return On success, the track-id of the new track.
 *      On error, #MP4_INVALID_TRACK_ID.
 */
MP4V2_EXPORT
MP4TrackId MP4AddHintTrack(
    MP4FileHandle hFile,
    MP4TrackId    refTrackId );

MP4V2_EXPORT
MP4TrackId MP4AddTextTrack(
    MP4FileHandle hFile,
    MP4TrackId    refTrackId );

MP4V2_EXPORT
MP4TrackId MP4AddSubtitleTrack(
    MP4FileHandle hFile,
    uint32_t      timescale,
    uint16_t      width,
    uint16_t      height );

MP4V2_EXPORT
MP4TrackId MP4AddPixelAspectRatio(
    MP4FileHandle hFile,
    MP4TrackId    refTrackId,
    uint32_t      hSpacing,
    uint32_t      vSpacing );

MP4V2_EXPORT
MP4TrackId MP4AddColr(
    MP4FileHandle hFile,
    MP4TrackId    refTrackId,
    uint16_t      primary,
    uint16_t      transfer,
    uint16_t      matrix );

MP4V2_EXPORT
MP4TrackId MP4CloneTrack(
    MP4FileHandle srcFile,
    MP4TrackId    srcTrackId,
    MP4FileHandle dstFile DEFAULT(MP4_INVALID_FILE_HANDLE),
    MP4TrackId    dstHintTrackReferenceTrack DEFAULT(MP4_INVALID_TRACK_ID) );

MP4V2_EXPORT
MP4TrackId MP4CopyTrack(
    MP4FileHandle srcFile,
    MP4TrackId    srcTrackId,
    MP4FileHandle dstFile DEFAULT(MP4_INVALID_FILE_HANDLE),
    bool          applyEdits DEFAULT(false),
    MP4TrackId    dstHintTrackReferenceTrack DEFAULT(MP4_INVALID_TRACK_ID) );

MP4V2_EXPORT
void MP4DeleteTrack(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

MP4V2_EXPORT
uint32_t MP4GetNumberOfTracks(
    MP4FileHandle hFile,
    const char*   type DEFAULT(NULL),
    uint8_t       subType DEFAULT(0) );

MP4V2_EXPORT
MP4TrackId MP4FindTrackId(
    MP4FileHandle hFile,
    uint16_t      index,
    const char*   type DEFAULT(NULL),
    uint8_t       subType DEFAULT(0) );

MP4V2_EXPORT
uint16_t MP4FindTrackIndex(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

/** Get maximum duration of chunk.
 *
 *  MP4GetTrackDurationPerChunk gets the maximum duration for each chunk.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param duration out value of duration in track timescale units.
 *
 *  return <b>true</b> on success, <b>false</b> on failure.
 */
MP4V2_EXPORT
bool MP4GetTrackDurationPerChunk(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4Duration*  duration );

/** Set maximum duration of chunk.
 *
 *  MP4SetTrackDurationPerChunk sets the maximum duration for each chunk.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param duration in timescale units.
 *
 *  return <b>true</b> on success, <b>false</b> on failure.
 */
MP4V2_EXPORT
bool MP4SetTrackDurationPerChunk(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4Duration   duration );

MP4V2_EXPORT
void MP4AddIPodUUID(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

/** @} ***********************************************************************/

#endif /* MP4V2_TRACK_H */
