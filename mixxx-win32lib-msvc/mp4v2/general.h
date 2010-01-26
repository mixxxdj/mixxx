#ifndef MP4V2_GENERAL_H
#define MP4V2_GENERAL_H

/**************************************************************************//**
 *
 *  @defgroup mp4_general MP4v2 General
 *  @{
 *
 *****************************************************************************/

/* MP4 API types */
typedef void*       MP4FileHandle;
typedef uint32_t    MP4TrackId;
typedef uint32_t    MP4SampleId;
typedef uint64_t    MP4Timestamp;
typedef uint64_t    MP4Duration;
typedef uint32_t    MP4EditId;

/*****************************************************************************/

typedef void (*error_msg_func_t)(
    int         loglevel,
    const char* lib,
    const char* fmt,
    va_list     ap );

typedef void (*lib_message_func_t)(
    int         loglevel,
    const char* lib,
    const char* fmt,
    ... );

/*****************************************************************************/

/** Encryption function pointer.
 *
 * @see MP4EncAndCopySample().
 * @see MP4EncAndCopyTrack().
 */
typedef uint32_t (*encryptFunc_t)( uint32_t, uint32_t, uint8_t*, uint32_t*, uint8_t** );

/*****************************************************************************/

#define MP4_INVALID_FILE_HANDLE ((MP4FileHandle)NULL) /**< Constant: invalid MP4FileHandle. */
#define MP4_INVALID_TRACK_ID    ((MP4TrackId)0)       /**< Constant: invalid MP4TrackId. */
#define MP4_INVALID_SAMPLE_ID   ((MP4SampleId)0)      /**< Constant: invalid MP4SampleId. */
#define MP4_INVALID_TIMESTAMP   ((MP4Timestamp)-1)    /**< Constant: invalid MP4Timestamp. */
#define MP4_INVALID_DURATION    ((MP4Duration)-1)     /**< Constant: invalid MP4Duration. */
#define MP4_INVALID_EDIT_ID     ((MP4EditId)0)        /**< Constant: invalid MP4EditId. */

/* Macros to test for API type validity */
#define MP4_IS_VALID_FILE_HANDLE(x) ((x) != MP4_INVALID_FILE_HANDLE)
#define MP4_IS_VALID_TRACK_ID(x)    ((x) != MP4_INVALID_TRACK_ID)
#define MP4_IS_VALID_SAMPLE_ID(x)   ((x) != MP4_INVALID_SAMPLE_ID)
#define MP4_IS_VALID_TIMESTAMP(x)   ((x) != MP4_INVALID_TIMESTAMP)
#define MP4_IS_VALID_DURATION(x)    ((x) != MP4_INVALID_DURATION)
#define MP4_IS_VALID_EDIT_ID(x)     ((x) != MP4_INVALID_EDIT_ID)

#define MP4_DETAILS_READ_ALL        \
    (MP4_DETAILS_READ | MP4_DETAILS_TABLE | MP4_DETAILS_SAMPLE)
#define MP4_DETAILS_WRITE_ALL       \
    (MP4_DETAILS_WRITE | MP4_DETAILS_TABLE | MP4_DETAILS_SAMPLE)

/*
 * MP4 Known track type names - e.g. MP4GetNumberOfTracks(type)
 *
 * Note this first group of track types should be created
 * via the MP4Add<Type>Track() functions, and not MP4AddTrack(type)
 */
#define MP4_OD_TRACK_TYPE       "odsm"  /**< Constant: OD track. */
#define MP4_SCENE_TRACK_TYPE    "sdsm"  /**< Constant: scene track. */
#define MP4_AUDIO_TRACK_TYPE    "soun"  /**< Constant: audio track. */
#define MP4_VIDEO_TRACK_TYPE    "vide"  /**< Constant: video track. */
#define MP4_HINT_TRACK_TYPE     "hint"  /**< Constant: hint track. */
#define MP4_CNTL_TRACK_TYPE     "cntl"  /**< Constant: control track. */
#define MP4_TEXT_TRACK_TYPE     "text"  /**< Constant: text track. */
#define MP4_SUBTITLE_TRACK_TYPE "sbtl"  /**< Constant: subtitle track. */
/*
 * This second set of track types should be created
 * via MP4AddSystemsTrack(type)
 */
#define MP4_CLOCK_TRACK_TYPE    "crsm"  /**< Constant: clock track. */
#define MP4_MPEG7_TRACK_TYPE    "m7sm"  /**< Constant: mpeg7 track. */
#define MP4_OCI_TRACK_TYPE      "ocsm"  /**< Constant: OCI track. */
#define MP4_IPMP_TRACK_TYPE     "ipsm"  /**< Constant: IPMP track. */
#define MP4_MPEGJ_TRACK_TYPE    "mjsm"  /**< Constant: MPEGJ track. */

#define MP4_IS_VIDEO_TRACK_TYPE(type) \
    (!strcasecmp(type, MP4_VIDEO_TRACK_TYPE))

#define MP4_IS_AUDIO_TRACK_TYPE(type) \
    (!strcasecmp(type, MP4_AUDIO_TRACK_TYPE))

#define MP4_IS_CNTL_TRACK_TYPE(type) \
        (!strcasecmp(type, MP4_CNTL_TRACK_TYPE))

#define MP4_IS_OD_TRACK_TYPE(type) \
    (!strcasecmp(type, MP4_OD_TRACK_TYPE))

#define MP4_IS_SCENE_TRACK_TYPE(type) \
    (!strcasecmp(type, MP4_SCENE_TRACK_TYPE))

#define MP4_IS_HINT_TRACK_TYPE(type) \
    (!strcasecmp(type, MP4_HINT_TRACK_TYPE))

#define MP4_IS_SYSTEMS_TRACK_TYPE(type) \
    (!strcasecmp(type, MP4_CLOCK_TRACK_TYPE) \
    || !strcasecmp(type, MP4_MPEG7_TRACK_TYPE) \
    || !strcasecmp(type, MP4_OCI_TRACK_TYPE) \
    || !strcasecmp(type, MP4_IPMP_TRACK_TYPE) \
    || !strcasecmp(type, MP4_MPEGJ_TRACK_TYPE))

/* MP4 Audio track types - see MP4AddAudioTrack()*/
#define MP4_INVALID_AUDIO_TYPE          0x00
#define MP4_MPEG1_AUDIO_TYPE            0x6B
#define MP4_MPEG2_AUDIO_TYPE            0x69
#define MP4_MP3_AUDIO_TYPE              MP4_MPEG2_AUDIO_TYPE
#define MP4_MPEG2_AAC_MAIN_AUDIO_TYPE   0x66
#define MP4_MPEG2_AAC_LC_AUDIO_TYPE     0x67
#define MP4_MPEG2_AAC_SSR_AUDIO_TYPE    0x68
#define MP4_MPEG2_AAC_AUDIO_TYPE        MP4_MPEG2_AAC_MAIN_AUDIO_TYPE
#define MP4_MPEG4_AUDIO_TYPE            0x40
#define MP4_PRIVATE_AUDIO_TYPE          0xC0
#define MP4_PCM16_LITTLE_ENDIAN_AUDIO_TYPE  0xE0 /* a private definition */
#define MP4_VORBIS_AUDIO_TYPE           0xE1 /* a private definition */
#define MP4_AC3_AUDIO_TYPE              0xE2 /* a private definition */
#define MP4_ALAW_AUDIO_TYPE             0xE3 /* a private definition */
#define MP4_ULAW_AUDIO_TYPE             0xE4 /* a private definition */
#define MP4_G723_AUDIO_TYPE             0xE5 /* a private definition */
#define MP4_PCM16_BIG_ENDIAN_AUDIO_TYPE 0xE6 /* a private definition */

/* MP4 MPEG-4 Audio types from 14496-3 Table 1.5.1 */
#define MP4_MPEG4_INVALID_AUDIO_TYPE        0
#define MP4_MPEG4_AAC_MAIN_AUDIO_TYPE       1
#define MP4_MPEG4_AAC_LC_AUDIO_TYPE         2
#define MP4_MPEG4_AAC_SSR_AUDIO_TYPE        3
#define MP4_MPEG4_AAC_LTP_AUDIO_TYPE        4
#define MP4_MPEG4_AAC_HE_AUDIO_TYPE         5
#define MP4_MPEG4_AAC_SCALABLE_AUDIO_TYPE   6
#define MP4_MPEG4_CELP_AUDIO_TYPE           8
#define MP4_MPEG4_HVXC_AUDIO_TYPE           9
#define MP4_MPEG4_TTSI_AUDIO_TYPE           12
#define MP4_MPEG4_MAIN_SYNTHETIC_AUDIO_TYPE 13
#define MP4_MPEG4_WAVETABLE_AUDIO_TYPE      14
#define MP4_MPEG4_MIDI_AUDIO_TYPE           15
#define MP4_MPEG4_ALGORITHMIC_FX_AUDIO_TYPE 16
#define MP4_MPEG4_ALS_AUDIO_TYPE            31
#define MP4_MPEG4_LAYER1_AUDIO_TYPE         32
#define MP4_MPEG4_LAYER2_AUDIO_TYPE         33
#define MP4_MPEG4_LAYER3_AUDIO_TYPE         34
#define MP4_MPEG4_SLS_AUDIO_TYPE            35

/* MP4 Audio type utilities following common usage */
#define MP4_IS_MP3_AUDIO_TYPE(type) \
    ((type) == MP4_MPEG1_AUDIO_TYPE || (type) == MP4_MPEG2_AUDIO_TYPE)

#define MP4_IS_MPEG2_AAC_AUDIO_TYPE(type) \
    (((type) >= MP4_MPEG2_AAC_MAIN_AUDIO_TYPE \
        && (type) <= MP4_MPEG2_AAC_SSR_AUDIO_TYPE))

#define MP4_IS_MPEG4_AAC_AUDIO_TYPE(mpeg4Type) \
    (((mpeg4Type) >= MP4_MPEG4_AAC_MAIN_AUDIO_TYPE \
        && (mpeg4Type) <= MP4_MPEG4_AAC_HE_AUDIO_TYPE) \
      || (mpeg4Type) == MP4_MPEG4_AAC_SCALABLE_AUDIO_TYPE \
          || (mpeg4Type) == 17)

#define MP4_IS_AAC_AUDIO_TYPE(type) \
    (MP4_IS_MPEG2_AAC_AUDIO_TYPE(type) \
    || (type) == MP4_MPEG4_AUDIO_TYPE)

/* MP4 Video track types - see MP4AddVideoTrack() */
#define MP4_INVALID_VIDEO_TYPE          0x00
#define MP4_MPEG1_VIDEO_TYPE            0x6A
#define MP4_MPEG2_SIMPLE_VIDEO_TYPE     0x60
#define MP4_MPEG2_MAIN_VIDEO_TYPE       0x61
#define MP4_MPEG2_SNR_VIDEO_TYPE        0x62
#define MP4_MPEG2_SPATIAL_VIDEO_TYPE    0x63
#define MP4_MPEG2_HIGH_VIDEO_TYPE       0x64
#define MP4_MPEG2_442_VIDEO_TYPE        0x65
#define MP4_MPEG2_VIDEO_TYPE            MP4_MPEG2_MAIN_VIDEO_TYPE
#define MP4_MPEG4_VIDEO_TYPE            0x20
#define MP4_JPEG_VIDEO_TYPE             0x6C
#define MP4_PRIVATE_VIDEO_TYPE          0xD0
#define MP4_YUV12_VIDEO_TYPE            0xF0    /* a private definition */
#define MP4_H263_VIDEO_TYPE             0xF2    /* a private definition */
#define MP4_H261_VIDEO_TYPE             0xF3    /* a private definition */

/* MP4 Video type utilities */
#define MP4_IS_MPEG1_VIDEO_TYPE(type) \
    ((type) == MP4_MPEG1_VIDEO_TYPE)

#define MP4_IS_MPEG2_VIDEO_TYPE(type) \
    (((type) >= MP4_MPEG2_SIMPLE_VIDEO_TYPE \
        && (type) <= MP4_MPEG2_442_VIDEO_TYPE) \
      || MP4_IS_MPEG1_VIDEO_TYPE(type))

#define MP4_IS_MPEG4_VIDEO_TYPE(type) \
    ((type) == MP4_MPEG4_VIDEO_TYPE)

/* Mpeg4 Visual Profile Defines - ISO/IEC 14496-2:2001/Amd.2:2002(E) */
#define MPEG4_SP_L1 (0x1)
#define MPEG4_SP_L2 (0x2)
#define MPEG4_SP_L3 (0x3)
#define MPEG4_SP_L0 (0x8)
#define MPEG4_SSP_L1 (0x11)
#define MPEG4_SSP_L2 (0x12)
#define MPEG4_CP_L1 (0x21)
#define MPEG4_CP_L2 (0x22)
#define MPEG4_MP_L2 (0x32)
#define MPEG4_MP_L3 (0x33)
#define MPEG4_MP_L4 (0x34)
#define MPEG4_NBP_L2 (0x42)
#define MPEG4_STP_L1 (0x51)
#define MPEG4_SFAP_L1 (0x61)
#define MPEG4_SFAP_L2 (0x62)
#define MPEG4_SFBAP_L1 (0x63)
#define MPEG4_SFBAP_L2 (0x64)
#define MPEG4_BATP_L1 (0x71)
#define MPEG4_BATP_L2 (0x72)
#define MPEG4_HP_L1 (0x81)
#define MPEG4_HP_L2 (0x82)
#define MPEG4_ARTSP_L1 (0x91)
#define MPEG4_ARTSP_L2 (0x92)
#define MPEG4_ARTSP_L3 (0x93)
#define MPEG4_ARTSP_L4 (0x94)
#define MPEG4_CSP_L1 (0xa1)
#define MPEG4_CSP_L2 (0xa2)
#define MPEG4_CSP_L3 (0xa3)
#define MPEG4_ACEP_L1 (0xb1)
#define MPEG4_ACEP_L2 (0xb2)
#define MPEG4_ACEP_L3 (0xb3)
#define MPEG4_ACEP_L4 (0xb4)
#define MPEG4_ACP_L1 (0xc1)
#define MPEG4_ACP_L2 (0xc2)
#define MPEG4_AST_L1 (0xd1)
#define MPEG4_AST_L2 (0xd2)
#define MPEG4_AST_L3 (0xd3)
#define MPEG4_S_STUDIO_P_L1 (0xe1)
#define MPEG4_S_STUDIO_P_L2 (0xe2)
#define MPEG4_S_STUDIO_P_L3 (0xe3)
#define MPEG4_S_STUDIO_P_L4 (0xe4)
#define MPEG4_C_STUDIO_P_L1 (0xe5)
#define MPEG4_C_STUDIO_P_L2 (0xe6)
#define MPEG4_C_STUDIO_P_L3 (0xe7)
#define MPEG4_C_STUDIO_P_L4 (0xe8)
#define MPEG4_ASP_L0 (0xF0)
#define MPEG4_ASP_L1 (0xF1)
#define MPEG4_ASP_L2 (0xF2)
#define MPEG4_ASP_L3 (0xF3)
#define MPEG4_ASP_L4 (0xF4)
#define MPEG4_ASP_L5 (0xF5)
#define MPEG4_ASP_L3B (0xF7)
#define MPEG4_FGSP_L0 (0xf8)
#define MPEG4_FGSP_L1 (0xf9)
#define MPEG4_FGSP_L2 (0xfa)
#define MPEG4_FGSP_L3 (0xfb)
#define MPEG4_FGSP_L4 (0xfc)
#define MPEG4_FGSP_L5 (0xfd)

/*****************************************************************************/

/* 3GP specific utilities */

MP4V2_EXPORT
bool MP4Make3GPCompliant(
    const char* fileName,
    uint32_t    verbosity DEFAULT(0),
    char*       majorBrand DEFAULT(0),
    uint32_t    minorVersion DEFAULT(0),
    char**      supportedBrands DEFAULT(NULL),
    uint32_t    supportedBrandsCount DEFAULT(0),
    bool        deleteIodsAtom DEFAULT(true) );

/* NOTE this section of functionality has not yet been fully tested */

MP4V2_EXPORT
MP4EditId MP4AddTrackEdit(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4EditId     editId DEFAULT(MP4_INVALID_EDIT_ID),
    MP4Timestamp  startTime DEFAULT(0),
    MP4Duration   duration DEFAULT(0),
    bool          dwell DEFAULT(false) );

MP4V2_EXPORT
bool MP4DeleteTrackEdit(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4EditId     editId );

MP4V2_EXPORT
uint32_t MP4GetTrackNumberOfEdits(
    MP4FileHandle hFile,
    MP4TrackId    trackId );

MP4V2_EXPORT
MP4Timestamp MP4GetTrackEditStart(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4EditId     editId );

MP4V2_EXPORT
MP4Duration MP4GetTrackEditTotalDuration(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4EditId     editId DEFAULT(MP4_INVALID_EDIT_ID) );

MP4V2_EXPORT
MP4Timestamp MP4GetTrackEditMediaStart(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4EditId     editId );

MP4V2_EXPORT
bool MP4SetTrackEditMediaStart(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4EditId     editId,
    MP4Timestamp  startTime );

MP4V2_EXPORT
MP4Duration MP4GetTrackEditDuration(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4EditId     editId );

MP4V2_EXPORT
bool MP4SetTrackEditDuration(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4EditId     editId,
    MP4Duration   duration );

MP4V2_EXPORT
int8_t MP4GetTrackEditDwell(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4EditId     editId );

MP4V2_EXPORT
bool MP4SetTrackEditDwell(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4EditId     editId,
    bool          dwell );

MP4V2_EXPORT
bool MP4ReadSampleFromEditTime(
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

MP4V2_EXPORT
MP4SampleId MP4GetSampleIdFromEditTime(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4Timestamp  when,
    MP4Timestamp* pStartTime DEFAULT(NULL),
    MP4Duration*  pDuration DEFAULT(NULL) );

/* time conversion utilties */

/* predefined values for timeScale parameter below */
#define MP4_SECONDS_TIME_SCALE      1
#define MP4_MILLISECONDS_TIME_SCALE 1000
#define MP4_MICROSECONDS_TIME_SCALE 1000000
#define MP4_NANOSECONDS_TIME_SCALE  1000000000

#define MP4_SECS_TIME_SCALE     MP4_SECONDS_TIME_SCALE
#define MP4_MSECS_TIME_SCALE    MP4_MILLISECONDS_TIME_SCALE
#define MP4_USECS_TIME_SCALE    MP4_MICROSECONDS_TIME_SCALE
#define MP4_NSECS_TIME_SCALE    MP4_NANOSECONDS_TIME_SCALE

MP4V2_EXPORT
uint64_t MP4ConvertFromMovieDuration(
    MP4FileHandle hFile,
    MP4Duration   duration,
    uint32_t      timeScale );

MP4V2_EXPORT
uint64_t MP4ConvertFromTrackTimestamp(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4Timestamp  timeStamp,
    uint32_t      timeScale );

MP4V2_EXPORT
MP4Timestamp MP4ConvertToTrackTimestamp(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    uint64_t      timeStamp,
    uint32_t      timeScale );

/** Convert duration from track time scale to an arbitrary time scale.
 *
 *  MP4ConvertFromTrackDuration converts a duration such as a sample duration
 *  from the track time scale to another time scale. This can be used by a
 *  player application to map all track samples to a common time scale.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param duration value to be converted.
 *  @param timeScale time scale in ticks per second.
 *
 *  @return On success, the duration in arbitrary time scale units.
 *      On error, <b>0</b>.
 *
 *  @see MP4GetSampleDuration().
 *  @see MP4ConvertToTrackDuration().
 */
MP4V2_EXPORT
uint64_t MP4ConvertFromTrackDuration(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    MP4Duration   duration,
    uint32_t      timeScale );

/** Convert duration from arbitrary time scale to track time scale.
 *
 *  MP4ConvertToTrackDuration converts a duration such as a sample duration
 *  from the specified time scale to the track time scale.
 *
 *  @param hFile handle of file for operation.
 *  @param trackId id of track for operation.
 *  @param duration value to be converted.
 *  @param timeScale time scale in ticks per second.
 *
 *  @return On success, the duration in track time scale units.
 *      On error, #MP4_INVALID_DURATION.
 *
 *  @see MP4ConvertFromTrackDuration().
 */
MP4V2_EXPORT
MP4Duration MP4ConvertToTrackDuration(
    MP4FileHandle hFile,
    MP4TrackId    trackId,
    uint64_t      duration,
    uint32_t      timeScale );

MP4V2_EXPORT
char* MP4BinaryToBase16(
    const uint8_t* pData,
    uint32_t       dataSize );

MP4V2_EXPORT
char* MP4BinaryToBase64(
    const uint8_t* pData,
    uint32_t       dataSize );

MP4V2_EXPORT
uint8_t* Base64ToBinary(
    const char* pData,
    uint32_t    decodeSize,
    uint32_t*   pDataSize );

MP4V2_EXPORT
void MP4Free(
    void* p );

MP4V2_EXPORT
void MP4SetLibFunc(
    lib_message_func_t libfunc );

/** @} ***********************************************************************/

#endif /* MP4V2_GENERAL_H */
