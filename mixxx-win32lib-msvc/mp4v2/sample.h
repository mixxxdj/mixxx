#ifndef MP4V2_SAMPLE_H
#define MP4V2_SAMPLE_H

/**************************************************************************//**
 *
 *  @defgroup mp4_sample MP4v2 Sample
 *  @{
 *
 *****************************************************************************/

/** Sample dependency types.
 *
 *  Bit combinations 0x03, 0x30, 0xc0 are reserved.
 */
typedef enum MP4SampleDependencyType_e {
    MP4_SDT_UNKNOWN                       = 0x00, /**< unknown */
    MP4_SDT_HAS_REDUNDANT_CODING          = 0x01, /**< contains redundant coding */
    MP4_SDT_HAS_NO_REDUNDANT_CODING       = 0x02, /**< does not contain redundant coding */
    MP4_SDT_HAS_DEPENDENTS                = 0x04, /**< referenced by other samples */
    MP4_SDT_HAS_NO_DEPENDENTS             = 0x08, /**< not referenced by other samples */
    MP4_SDT_IS_DEPENDENT                  = 0x10, /**< references other samples */
    MP4_SDT_IS_INDEPENDENT                = 0x20, /**< does not reference other samples */
    MP4_SDT_EARLIER_DISPLAY_TIMES_ALLOWED = 0x40, /**< subequent samples in GOP may display earlier */
    _MP4_SDT_RESERVED                     = 0x80, /**< reserved */
} MP4SampleDependencyType;

/** Read a track sample.
 *
 *  MP4ReadSample reads the specified sample from the specified track.
 *  Typically this sample is then decoded in a codec dependent fashion and
 *  rendered in an appropriate fashion.
 *
 *  The argument <b>ppBytes</b> allows for two possible approaches for
 *  buffering:
 *
 *  If the calling application wishes to handle its own buffering it can set
 *  *ppBytes to the buffer it wishes to use. The calling application is
 *  responsible for ensuring that the buffer is large enough to hold the
 *  sample. This can be done by using either MP4GetSampleSize() or
 *  MP4GetTrackMaxSampleSize() to determine before−hand how large the
 *  receiving buffer must be.
 *
 *  If the value of *ppBytes is NULL, then an appropriately sized buffer is
 *  automatically malloc’ed for the sample data and *ppBytes set to this
 *  pointer. The calling application is responsible for free’ing this
 *  memory.
 *
 *  The last four arguments are pointers to variables that can receive
 *  optional sample information.
 *
 *  Typically for audio none of these are needed. MPEG audio such as MP3 or
 *  AAC has a fixed sample duration and every sample can be accessed at
 *  random.
 *
 *  For video, all of these optional values could be needed. MPEG video can
 *  be encoded at a variable frame rate, with only occasional random access
 *  points, and with "B frames" which cause the rendering  (display)  order
 *  of the video frames to differ from the storage/decoding order.
 *
 *  Other media types fall between these two extremes.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param sampleId specifies which sample is to be read.
 *      Caveat: the first sample has id <b>1</b> not <b>0</b>.
 *  @param ppBytes pointer to the pointer to the sample data.
 *  @param pNumBytes pointer to variable that will be hold the size in bytes
 *      of the sample.
 *  @param pStartTime if non−NULL, pointer to variable that will receive the
 *      starting timestamp for this sample. Caveat: The timestamp is in
 *      <b>trackId</b>'s timescale.
 *  @param pDuration if non−NULL, pointer to variable that will receive the
 *      duration for this sample. Caveat: The duration is in
 *      <b>trackId</b>'s timescale.
 *  @param pRenderingOffset if non−NULL, pointer to variable that will
 *      receive the rendering offset for this sample. Currently the only
 *      media type that needs this feature is MPEG video. Caveat: The offset
 *      is in <b>trackId</b>'s timescale.
 *  @param pIsSyncSample if non−NULL, pointer to variable that will receive
 *      the state of the sync/random access flag for this sample.
 *
 *  @return <b>true</b> on success, <b>false</b> on failure.
 *
 *  @see MP4GetSampleSize().
 *  @see MP4GetTrackMaxSampleSize().
 */
MP4V2_EXPORT
bool MP4ReadSample(
    /* input parameters */
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4SampleId   sampleId,
    /* input/output parameters */
    uint8_t** ppBytes,
    uint32_t* pNumBytes,
    /* output parameters */
    MP4Timestamp* pStartTime DEFAULT(NULL),
    MP4Duration*  pDuration DEFAULT(NULL),
    MP4Duration*  pRenderingOffset DEFAULT(NULL),
    bool*         pIsSyncSample DEFAULT(NULL) );

/** Read a track sample based on a specified time.
 *
 *  MP4ReadSampleFromTime is similar to MP4ReadSample() except the sample
 *  is specified by using a timestamp instead of sampleId.
 *  Typically this sample is then decoded in a codec dependent fashion and
 *  rendered in an appropriate fashion.
 *
 *  The argument <b>ppBytes</b> allows for two possible approaches for
 *  buffering:
 *
 *  If the calling application wishes to handle its own buffering it can set
 *  *ppBytes to the buffer it wishes to use. The calling application is
 *  responsible for ensuring that the buffer is large enough to hold the
 *  sample. This can be done by using either MP4GetSampleSize() or
 *  MP4GetTrackMaxSampleSize() to determine before−hand how large the
 *  receiving buffer must be.
 *
 *  If the value of *ppBytes is NULL, then an appropriately sized buffer is
 *  automatically malloc’ed for the sample data and *ppBytes set to this
 *  pointer. The calling application is responsible for free’ing this
 *  memory.
 *
 *  The last four arguments are pointers to variables that can receive
 *  optional sample information.
 *
 *  Typically for audio none of these are needed. MPEG audio such as MP3 or
 *  AAC has a fixed sample duration and every sample can be accessed at
 *  random.
 *
 *  For video, all of these optional values could be needed. MPEG video can
 *  be encoded at a variable frame rate, with only occasional random access
 *  points, and with "B frames" which cause the rendering  (display)  order
 *  of the video frames to differ from the storage/decoding order.
 *
 *  Other media types fall between these two extremes.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param when specifies which sample is to be read based on a time in the
 *      track timeline. See MP4GetSampleIdFromTime() for details.
 *  @param ppBytes pointer to the pointer to the sample data.
 *  @param pNumBytes pointer to variable that will be hold the size in bytes
 *      of the sample.
 *  @param pStartTime if non−NULL, pointer to variable that will receive the
 *      starting timestamp for this sample. Caveat: The timestamp is in
 *      <b>trackId</b>'s timescale.
 *  @param pDuration if non−NULL, pointer to variable that will receive the
 *      duration for this sample. Caveat: The duration is in
 *      <b>trackId</b>'s timescale.
 *  @param pRenderingOffset if non−NULL, pointer to variable that will
 *      receive the rendering offset for this sample. Currently the only
 *      media type that needs this feature is MPEG video. Caveat: The offset
 *      is in <b>trackId</b>'s timescale.
 *  @param pIsSyncSample if non−NULL, pointer to variable that will receive
 *      the state of the sync/random access flag for this sample.
 *
 *  @return <b>true</b> on success, <b>false</b> on failure.
 *
 *  @see MP4ReadSample().
 *  @see MP4GetSampleIdFromTime().
 *  @see MP4GetSampleSize().
 *  @see MP4GetTrackMaxSampleSize().
 */
MP4V2_EXPORT
bool MP4ReadSampleFromTime(
    /* input parameters */
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4Timestamp  when,
    /* input/output parameters */
    uint8_t** ppBytes,
    uint32_t* pNumBytes,
    /* output parameters */
    MP4Timestamp* pStartTime DEFAULT(NULL),
    MP4Duration*  pDuration DEFAULT(NULL),
    MP4Duration*  pRenderingOffset DEFAULT(NULL),
    bool*         pIsSyncSample DEFAULT(NULL) );

/** Write a track sample.
 *
 *  MP4WriteSample writes the given sample at the end of the specified track.
 *  Currently the library does not support random insertion of samples into
 *  the track timeline. Note that with mp4 there cannot be any holes or
 *  overlapping samples in the track timeline. The last three arguments give
 *  optional sample information.
 *
 *  The value of duration can be given as #MP4_INVALID_DURATION if all samples
 *  in the track have the same duration. This can be specified with
 *  MP4AddTrack() and related functions.
 *
 *  Typically for audio none of the optional arguments are needed. MPEG audio
 *  such as MP3 or AAC has a fixed sample duration and every sample can be
 *  accessed at random.
 *
 *  For video, all of the optional arguments could be  needed. MPEG video
 *  can be encoded at a variable frame rate, with only occasional random
 *  access points, and with "B frames" which cause the rendering  (display)
 *  order of the video frames to differ from the storage/decoding order.
 *
 *  Other media types fall between these two extremes.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param pBytes pointer to sample data.
 *  @param numBytes length of sample data in bytes.
 *  @param duration sample duration. Caveat: should be in track timescale.
 *  @param renderingOffset the rendering offset for this sample.
 *      Currently the only media type that needs this feature is MPEG
 *      video. Caveat: The offset should be in the track timescale.
 *  @param isSyncSample the sync/random access flag for this sample.
 *
 *  @return <b>true</b> on success, <b>false</b> on failure.
 *
 *  @see MP4AddTrack().
 */
MP4V2_EXPORT
bool MP4WriteSample(
    MP4FileHandle  hFile,
    MP4TrackId     trackId,
    const uint8_t* pBytes,
    uint32_t       numBytes,
    MP4Duration    duration DEFAULT(MP4_INVALID_DURATION),
    MP4Duration    renderingOffset DEFAULT(0),
    bool           isSyncSample DEFAULT(true) );

/** Write a track sample and supply dependency information.
 *
 *  MP4WriteSampleDependency writes the given sample at the end of the specified track.
 *  Currently the library does not support random insertion of samples into
 *  the track timeline. Note that with mp4 there cannot be any holes or
 *  overlapping samples in the track timeline. The last three arguments give
 *  optional sample information.
 *
 *  The value of duration can be given as #MP4_INVALID_DURATION if all samples
 *  in the track have the same duration. This can be specified with
 *  MP4AddTrack() and related functions.
 *
 *  When this method is used instead of MP4WriteSample() it enables <b>sdtp</b>
 *  atom to be written out. This atom may be used by advanced players to
 *  optimize trick-operations such as fast-fwd, reverse or scrubbing.
 *
 *  An <b>sdtp</b> atom will always be written out if this method is used.
 *  To avoid writing the atom, use MP4WriteSample() instead.
 *
 *  Intermixing use of MP4WriteSampleDependency() and MP4WriteSample() on the
 *  same track is not permitted.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param pBytes pointer to sample data.
 *  @param numBytes length of sample data in bytes.
 *  @param duration sample duration. Caveat: should be in track timescale.
 *  @param renderingOffset the rendering offset for this sample.
 *      Currently the only media type that needs this feature is MPEG
 *      video. Caveat: The offset should be in the track timescale.
 *  @param isSyncSample the sync/random access flag for this sample.
 *  @param dependencyFlags bitmask specifying sample dependency characteristics.
 *      See #MP4SampleDependencyType for bit constants.
 *
 *  @return <b>true</b> on success, <b>false</b> on failure.
 *
 *  @see MP4AddTrack().
 */
MP4V2_EXPORT
bool MP4WriteSampleDependency(
    MP4FileHandle  hFile,
    MP4TrackId     trackId,
    const uint8_t* pBytes,
    uint32_t       numBytes,
    MP4Duration    duration,
    MP4Duration    renderingOffset,
    bool           isSyncSample,
    uint32_t       dependencyFlags );

/** Make a copy of a sample.
 *
 *  MP4CopySample creates a new sample based on an existing sample. Note that
 *  another copy of the media sample data is created in the file using this
 *  function. ie. this call is equivalent to MP4ReadSample() followed by
 *  MP4WriteSample().
 *
 *  Note that is the responsibility of the caller to ensure that the copied
 *  media sample makes sense in the destination track. eg. copying a video
 *  sample to an audio track is unlikely to result in anything good happening,
 *  even copying a sample from video track to another requires that the tracks
 *  use the same encoding and that issues such as image size are addressed.
 *
 *  @param srcFile source sample file handle.
 *  @param srcTrackId source sample track id.
 *  @param srcSampleId source sample id.
 *  @param dstFile destination file handle for new (copied) sample.
 *      If the value is #MP4_INVALID_FILE_HANDLE, the copy is created in
 *      the same file as <b>srcFile</b>.
 *  @param dstTrackId destination track id for new sample.
 *      If the value is #MP4_INVALID_TRACK_ID, the the copy is created in
 *      the same track as the <b>srcTrackId</b>.
 *  @param dstSampleDuration duration in track timescale for new sample.
 *      If the value is #MP4_INVALID_DURATION, then the duration of
 *      the source sample is used.
 *
 *  @return On success, thew id of the new sample.
 *      On error, #MP4_INVALID_SAMPLE_ID.
 *
 *  @see MP4ReadSample().
 *  @see MP4WriteSample().
 */
MP4V2_EXPORT
bool MP4CopySample(
    MP4FileHandle srcFile,
    MP4TrackId    srcTrackId,
    MP4SampleId   srcSampleId,
    MP4FileHandle dstFile DEFAULT(MP4_INVALID_FILE_HANDLE),
    MP4TrackId    dstTrackId DEFAULT(MP4_INVALID_TRACK_ID),
    MP4Duration   dstSampleDuration DEFAULT(MP4_INVALID_DURATION) );

/** Make a copy of a sample.
 *
 *  MP4EncAndCopySample is similar to MP4CopySample() except that it
 *  offers an encryption hook to the caller.
 *
 *  @param srcFile source sample file handle.
 *  @param srcTrackId source sample track id.
 *  @param srcSampleId source sample id.
 *  @param encfcnp undocumented.
 *  @param encfcnparam1 undocumented.
 *  @param dstFile destination file handle for new (copied) sample.
 *      If the value is #MP4_INVALID_FILE_HANDLE, the copy is created in
 *      the same file as <b>srcFile</b>.
 *  @param dstTrackId destination track id for new sample.
 *      If the value is #MP4_INVALID_TRACK_ID, the the copy is created in
 *      the same track as the <b>srcTrackId</b>.
 *  @param dstSampleDuration duration in track timescale for new sample.
 *      If the value is #MP4_INVALID_DURATION, then the duration of
 *      the source sample is used.
 *
 *  @return On success, thew id of the new sample.
 *      On error, #MP4_INVALID_SAMPLE_ID.
 *
 *  @see MP4CopySample().
 */
MP4V2_EXPORT
bool MP4EncAndCopySample(
    MP4FileHandle srcFile,
    MP4TrackId    srcTrackId,
    MP4SampleId   srcSampleId,
    encryptFunc_t encfcnp,
    uint32_t      encfcnparam1,
    MP4FileHandle dstFile DEFAULT(MP4_INVALID_FILE_HANDLE),
    MP4TrackId    dstTrackId DEFAULT(MP4_INVALID_TRACK_ID),
    MP4Duration   dstSampleDuration DEFAULT(MP4_INVALID_DURATION) );

/** Not implemented.
 */
MP4V2_EXPORT
bool MP4ReferenceSample(
    MP4FileHandle srcFile,
    MP4TrackId    srcTrackId,
    MP4SampleId   srcSampleId,
    MP4FileHandle dstFile,
    MP4TrackId    dstTrackId,
    MP4Duration   dstSampleDuration DEFAULT(MP4_INVALID_DURATION) );

/** Get size of a track sample.
 *
 *  MP4GetSampleSize returns the size in bytes of the specified sample from the
 *  the specified track.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param sampleId id of sample for operation. Caveat: the first sample has
 *      id <b>1</b>, not <b>0</b>.
 *
 *  @return On success the sample size in bytes. On error, <b>0</b>.
 */
MP4V2_EXPORT
uint32_t MP4GetSampleSize(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4SampleId   sampleId);

/** Get the maximum sample size of a track.
 *
 *  MP4GetTrackMaxSampleSize returns the maximum size in bytes of all the
 *  samples in the specified track.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *
 *  @return On success, the maximum sample size in bytes. On error, <b>0</b>.
 *
 *  @see MP4GetSampleSize().
 */
MP4V2_EXPORT
uint32_t MP4GetTrackMaxSampleSize(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

/** Get sample id of a specified time.
 *
 *  MP4GetSampleIdFromTime returns the sample id of the track sample in which
 *  the specified time occurs.
 *
 *  The specified time should be in the track timescale.
 *
 *  It is wise to use MP4GetSampleTime() with the returned sample id so one
 *  can adjust for any difference between the specified time and the actual
 *  start time of the sample.
 *
 *  If the calling application needs a sample that can be accessed randomly
 *  then the <b>wantSyncSample</b> argument should be set to true. This could
 *  be the case for a player that is implementing a positioning function and
 *  needs to be able to start decoding a track from the returned sample id.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param when time in track timescale.
 *  @param wantSyncSample specifies if the result sample id must correspond
 *      to a sample whose sync/random access flag is true.
 *
 *  @return On success, the sample id that occurs at the specified time.
 *      On error, #MP4_INVALID_SAMPLE_ID.
 *
 *  @see MP4ConvertToTrackTimestamp() for how to map a time value to this
 *      timescale.
 */
MP4V2_EXPORT
MP4SampleId MP4GetSampleIdFromTime(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4Timestamp  when,
    bool          wantSyncSample DEFAULT(false) );

/** Get start time of track sample.
 *
 *  MP4GetSampleTime returns the start time of the specified sample from
 *  the specified track in the track timescale units.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param sampleId id of sample for operation. Caveat: the first sample has
 *      id <b>1</b>, not <b>0</b>.
 *
 *  @return On success, sample start time in track timescale units.
 *      On error, #MP4_INVALID_TIMESTAMP.
 *
 *  @see MP4ConvertFromTrackTimestamp() for how to map this value to another
 *      timescale.
 */
MP4V2_EXPORT
MP4Timestamp MP4GetSampleTime(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4SampleId   sampleId );

/** Get the duration of a track sample.
 *
 *  MP4GetSampleDuration returns the duration of the specified sample from
 *  the specified track in the track timescale units.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param sampleId id of sample for operation. Caveat: the first sample has
 *      id <b>1</b>, not <b>0</b>.
 *
 *  @return On success, the sample duration in track timescale units.
 *      On error, #MP4_INVALID_DURATION.
 *
 *  @see MP4ConvertFromTrackDuration() for how to map this value to another
 *      timescale.
 */
MP4V2_EXPORT
MP4Duration MP4GetSampleDuration(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4SampleId   sampleId );

/** Get the rendering offset of a track sample.
 *
 *  MP4GetSampleRenderingOffset returns the rendering offset of the specified
 *  sample from the specified track in the track timescale units.
 *
 *  The sample rendering offset is typically zero for all media types other
 *  than video. For video, encodings such as those  defined by MPEG have
 *  three  types  of  frames: I, P, and B. To increase coding efficiency B
 *  frames can depend on I or P frames that should be rendered after the B
 *  frame. However to decode the B frame the I or P frame must already have
 *  been decoded. This situation is addressed  by placing the frames in
 *  decoding order in the video track, and then setting the rendering offset
 *  property to indicate when the video frame should actually be rendered to
 *  the screen. Hence the start time of a sample indicates when it should be
 *  decoded, the start time plus the rendering offset indicates when it
 *  should be rendered.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param sampleId id of sample for operation. Caveat: the first sample has
 *      id <b>1</b>, not <b>0</b>.
 *
 *  @return On success, the rendering offset in track timescale units.
 *      On error, #MP4_INVALID_DURATION.
 *
 *  @see MP4ConvertFromTrackDuration() for how to map this value to another
 *      timescale.
 */
MP4V2_EXPORT
MP4Duration MP4GetSampleRenderingOffset(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4SampleId   sampleId );

/** Set the rendering offset of a track sample.
 *
 *  MP4SetSampleRenderingOffset sets the rendering offset of the specified
 *  sample from the specified track in the track timescale units.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param sampleId id of sample for operation. Caveat: the first sample has
 *      id <b>1</b>, not <b>0</b>.
 *  @param renderingOffset new offset value in timescale units.
 *
 *  @return <b>true</b> on success, <b>false</b> on failure.
 *
 *  @see MP4ConvertToTrackDuration() for how to map this value from another
 *      timescale.
 *  @see MP4GetSampleRenderingOffset() for a description of this sample
 *      property.
 */
MP4V2_EXPORT
bool MP4SetSampleRenderingOffset(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4SampleId   sampleId,
    MP4Duration   renderingOffset );

/** Get sync/random access state of sample.
 *
 *  MP4GetSampleSync returns the state of the sync/random access flag of
 *  the specified sample from the specified track.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param sampleId id of sample for operation. Caveat: the first sample has
 *      id <b>1</b>, not <b>0</b>.
 *
 *  @return <b>1</b> when true, <b>0</b> when false. On error, <b>-1</b>.
 */
MP4V2_EXPORT
int8_t MP4GetSampleSync(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4SampleId   sampleId );

/* @} ***********************************************************************/

#endif /* MP4V2_SAMPLE_H */
