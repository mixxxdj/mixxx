/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is MPEG4IP.
 * 
 * The Initial Developer of the Original Code is Cisco Systems Inc.
 * Portions created by Cisco Systems Inc. are
 * Copyright (C) Cisco Systems Inc. 2001 - 2005.  All Rights Reserved.
 * 
 * 3GPP features implementation is based on 3GPP's TS26.234-v5.60,
 * and was contributed by Ximpo Group Ltd.
 *
 * Portions created by Ximpo Group Ltd. are
 * Copyright (C) Ximpo Group Ltd. 2003, 2004.  All Rights Reserved.
 *
 * Contributor(s): 
 *		Dave Mackie			dmackie@cisco.com
 *		Alix Marchandise-Franquet	alix@cisco.com
 *              Ximpo Group Ltd.                mp4v2@ximpo.com
 *              Bill May                        wmay@cisco.com
 */

#ifndef __MP4_INCLUDED__
#define __MP4_INCLUDED__

/* include system and project specific headers */
#include "mpeg4ip.h"
#include <math.h>	/* to define float HUGE_VAL and/or NAN */

#ifdef __MINGW32__
typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;
#endif

#ifndef NAN
#define NAN HUGE_VAL
#endif

#ifndef DEFAULT
#ifdef __cplusplus
/* exploit C++ ability of default values for function parameters */
#define DEFAULT(x)	=x
#else
#define DEFAULT(x)
#endif
#endif

/* MP4 API types */
typedef void*		MP4FileHandle;
typedef u_int32_t	MP4TrackId;
typedef u_int32_t	MP4SampleId;
typedef u_int64_t	MP4Timestamp;
typedef u_int64_t	MP4Duration;
typedef u_int32_t	MP4EditId;

typedef u_int64_t (*VIRTUALIO_GETFILELENGTH)(void *user); // return file length in bytes
typedef int (*VIRTUALIO_SETPOSITION)(void *user, u_int64_t position); // return 0 on success
typedef int (*VIRTUALIO_GETPOSITION)(void *user, u_int64_t *position); // fill position, return 0 on success
typedef size_t (*VIRTUALIO_READ)(void *user, void *buffer, size_t size); // return number of bytes actually read
typedef size_t (*VIRTUALIO_WRITE)(void *user, void *buffer, size_t size); // return number of bytes actually written
typedef int (*VIRTUALIO_ENDOFFILE)(void *user); // return 1 if file hit EOF
typedef int (*VIRTUALIO_CLOSE)(void *user); // return 0 on success

typedef struct Virtual_IO
{
	VIRTUALIO_GETFILELENGTH	GetFileLength;
	VIRTUALIO_SETPOSITION SetPosition;
	VIRTUALIO_GETPOSITION GetPosition;
	VIRTUALIO_READ Read;
	VIRTUALIO_WRITE Write;
	VIRTUALIO_ENDOFFILE EndOfFile;
	VIRTUALIO_CLOSE Close;
} Virtual_IO_t;


/* Invalid values for API types */
#define MP4_INVALID_FILE_HANDLE	((MP4FileHandle)NULL)
#define MP4_INVALID_TRACK_ID	((MP4TrackId)0)
#define MP4_INVALID_SAMPLE_ID	((MP4SampleId)0)
#define MP4_INVALID_TIMESTAMP	((MP4Timestamp)-1)
#define MP4_INVALID_DURATION	((MP4Duration)-1)
#define MP4_INVALID_EDIT_ID		((MP4EditId)0)

/* Macros to test for API type validity */
#define MP4_IS_VALID_FILE_HANDLE(x)	((x) != MP4_INVALID_FILE_HANDLE) 
#define MP4_IS_VALID_TRACK_ID(x)	((x) != MP4_INVALID_TRACK_ID) 
#define MP4_IS_VALID_SAMPLE_ID(x)	((x) != MP4_INVALID_SAMPLE_ID) 
#define MP4_IS_VALID_TIMESTAMP(x)	((x) != MP4_INVALID_TIMESTAMP) 
#define MP4_IS_VALID_DURATION(x)	((x) != MP4_INVALID_DURATION) 
#define MP4_IS_VALID_EDIT_ID(x)		((x) != MP4_INVALID_EDIT_ID) 

/* MP4 verbosity levels - e.g. MP4SetVerbosity() */
#define MP4_DETAILS_ALL				0xFFFFFFFF
#define MP4_DETAILS_ERROR			0x00000001
#define MP4_DETAILS_WARNING			0x00000002
#define MP4_DETAILS_READ			0x00000004
#define MP4_DETAILS_WRITE			0x00000008
#define MP4_DETAILS_FIND			0x00000010
#define MP4_DETAILS_TABLE			0x00000020
#define MP4_DETAILS_SAMPLE			0x00000040
#define MP4_DETAILS_HINT			0x00000080
#define MP4_DETAILS_ISMA			0x00000100
#define MP4_DETAILS_EDIT			0x00000200

#define MP4_DETAILS_READ_ALL		\
	(MP4_DETAILS_READ | MP4_DETAILS_TABLE | MP4_DETAILS_SAMPLE)
#define MP4_DETAILS_WRITE_ALL		\
	(MP4_DETAILS_WRITE | MP4_DETAILS_TABLE | MP4_DETAILS_SAMPLE)

/*
 * MP4 Known track type names - e.g. MP4GetNumberOfTracks(type) 
 *
 * Note this first group of track types should be created 
 * via the MP4Add<Type>Track() functions, and not MP4AddTrack(type)
 */
#define MP4_OD_TRACK_TYPE		"odsm"
#define MP4_SCENE_TRACK_TYPE	"sdsm"
#define MP4_AUDIO_TRACK_TYPE	"soun"
#define MP4_VIDEO_TRACK_TYPE	"vide"
#define MP4_HINT_TRACK_TYPE		"hint"
#define MP4_CNTL_TRACK_TYPE     "cntl"
#define MP4_TEXT_TRACK_TYPE		"text"
/*
 * This second set of track types should be created 
 * via MP4AddSystemsTrack(type)
 */
#define MP4_CLOCK_TRACK_TYPE	"crsm"
#define MP4_MPEG7_TRACK_TYPE	"m7sm"
#define MP4_OCI_TRACK_TYPE		"ocsm"
#define MP4_IPMP_TRACK_TYPE		"ipsm"
#define MP4_MPEGJ_TRACK_TYPE	"mjsm"

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
#define MP4_INVALID_AUDIO_TYPE			0x00
#define MP4_MPEG1_AUDIO_TYPE			0x6B
#define MP4_MPEG2_AUDIO_TYPE			0x69
#define MP4_MP3_AUDIO_TYPE				MP4_MPEG2_AUDIO_TYPE
#define MP4_MPEG2_AAC_MAIN_AUDIO_TYPE	0x66
#define MP4_MPEG2_AAC_LC_AUDIO_TYPE		0x67
#define MP4_MPEG2_AAC_SSR_AUDIO_TYPE	0x68
#define MP4_MPEG2_AAC_AUDIO_TYPE		MP4_MPEG2_AAC_MAIN_AUDIO_TYPE
#define MP4_MPEG4_AUDIO_TYPE			0x40
#define MP4_PRIVATE_AUDIO_TYPE			0xC0
#define MP4_PCM16_LITTLE_ENDIAN_AUDIO_TYPE	0xE0	/* a private definition */
#define MP4_VORBIS_AUDIO_TYPE			0xE1	/* a private definition */
#define MP4_AC3_AUDIO_TYPE				0xE2	/* a private definition */
#define MP4_ALAW_AUDIO_TYPE				0xE3	/* a private definition */
#define MP4_ULAW_AUDIO_TYPE				0xE4	/* a private definition */
#define MP4_G723_AUDIO_TYPE                             0xE5    /* a private definition */
#define MP4_PCM16_BIG_ENDIAN_AUDIO_TYPE         0xE6 /* a private definition */

/* MP4 MPEG-4 Audio types from 14496-3 Table 1.5.1 */
#define MP4_MPEG4_INVALID_AUDIO_TYPE		0
#define MP4_MPEG4_AAC_MAIN_AUDIO_TYPE		1
#define MP4_MPEG4_AAC_LC_AUDIO_TYPE			2
#define MP4_MPEG4_AAC_SSR_AUDIO_TYPE		3
#define MP4_MPEG4_AAC_LTP_AUDIO_TYPE		4
#define MP4_MPEG4_AAC_HE_AUDIO_TYPE             5
#define MP4_MPEG4_AAC_SCALABLE_AUDIO_TYPE	6
#define MP4_MPEG4_CELP_AUDIO_TYPE			8
#define MP4_MPEG4_HVXC_AUDIO_TYPE			9
#define MP4_MPEG4_TTSI_AUDIO_TYPE			12
#define MP4_MPEG4_MAIN_SYNTHETIC_AUDIO_TYPE	13
#define MP4_MPEG4_WAVETABLE_AUDIO_TYPE		14
#define MP4_MPEG4_MIDI_AUDIO_TYPE			15
#define MP4_MPEG4_ALGORITHMIC_FX_AUDIO_TYPE	16
#define MP4_MPEG4_ALS_AUDIO_TYPE    31
#define MP4_MPEG4_LAYER1_AUDIO_TYPE 32
#define MP4_MPEG4_LAYER2_AUDIO_TYPE 33
#define MP4_MPEG4_LAYER3_AUDIO_TYPE 34
#define MP4_MPEG4_SLS_AUDIO_TYPE    35

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
#define MP4_INVALID_VIDEO_TYPE			0x00
#define MP4_MPEG1_VIDEO_TYPE			0x6A
#define MP4_MPEG2_SIMPLE_VIDEO_TYPE		0x60
#define MP4_MPEG2_MAIN_VIDEO_TYPE		0x61
#define MP4_MPEG2_SNR_VIDEO_TYPE		0x62
#define MP4_MPEG2_SPATIAL_VIDEO_TYPE	0x63
#define MP4_MPEG2_HIGH_VIDEO_TYPE		0x64
#define MP4_MPEG2_442_VIDEO_TYPE		0x65
#define MP4_MPEG2_VIDEO_TYPE			MP4_MPEG2_MAIN_VIDEO_TYPE
#define MP4_MPEG4_VIDEO_TYPE			0x20
#define MP4_JPEG_VIDEO_TYPE				0x6C
#define MP4_PRIVATE_VIDEO_TYPE			0xD0
#define MP4_YUV12_VIDEO_TYPE			0xF0	/* a private definition */
#define MP4_H263_VIDEO_TYPE				0xF2	/* a private definition */
#define MP4_H261_VIDEO_TYPE				0xF3	/* a private definition */

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

/* MP4 API declarations */

#ifdef __cplusplus
extern "C" {
#endif

/* file operations */
#define MP4_CREATE_64BIT_DATA (0x01)
#define MP4_CREATE_64BIT_TIME (0x02) // Quicktime is not compatible with this
#define MP4_CREATE_64BIT (MP4_CREATE_64BIT_DATA | MP4_CREATE_64BIT_TIME)
#define MP4_CREATE_EXTENSIBLE_FORMAT (0x04)

MP4FileHandle MP4Create(
	const char* fileName, 
	u_int32_t verbosity DEFAULT(0),
	u_int32_t flags DEFAULT(0));
MP4FileHandle MP4CreateEx(
        const char *fileName,
	u_int32_t verbosity DEFAULT(0),
	u_int32_t flags DEFAULT(0),
	int add_ftyp DEFAULT(1),
	int add_iods DEFAULT(1),
	char* majorBrand DEFAULT(0),
	u_int32_t minorVersion DEFAULT(0),
	char** supportedBrands DEFAULT(0),
	u_int32_t supportedBrandsCount DEFAULT(0));

MP4FileHandle MP4Modify(
	const char* fileName, 
	u_int32_t verbosity DEFAULT(0),
	u_int32_t flags DEFAULT(0));

MP4FileHandle MP4Read(
	const char* fileName, 
	u_int32_t verbosity DEFAULT(0));

// benski>
MP4FileHandle MP4ReadEx(const char* fileName,
			void *user, 
			Virtual_IO_t *virtual_IO,
			u_int32_t verbosity DEFAULT(0));
 
void MP4Close(
	MP4FileHandle hFile);

bool MP4Optimize(
	const char* existingFileName, 
	const char* newFileName DEFAULT(NULL), 
	u_int32_t verbosity DEFAULT(0));

bool MP4Dump(
	MP4FileHandle hFile, 
	FILE* pDumpFile DEFAULT(NULL), 
	bool dumpImplicits DEFAULT(0));

char* MP4Info(
	MP4FileHandle hFile, 
	MP4TrackId trackId DEFAULT(MP4_INVALID_TRACK_ID));

char* MP4FileInfo(
	const char* fileName,
	MP4TrackId trackId DEFAULT(MP4_INVALID_TRACK_ID));

/* file properties */

/* specific file properties */

u_int32_t MP4GetVerbosity(MP4FileHandle hFile);

void MP4SetVerbosity(MP4FileHandle hFile, u_int32_t verbosity);

MP4Duration MP4GetDuration(MP4FileHandle hFile);

u_int32_t MP4GetTimeScale(MP4FileHandle hFile);

bool MP4SetTimeScale(MP4FileHandle hFile, u_int32_t value);

u_int8_t MP4GetODProfileLevel(MP4FileHandle hFile);

bool MP4SetODProfileLevel(MP4FileHandle hFile, u_int8_t value);

u_int8_t MP4GetSceneProfileLevel(MP4FileHandle hFile);

bool MP4SetSceneProfileLevel(MP4FileHandle hFile, u_int8_t value);

u_int8_t MP4GetVideoProfileLevel(MP4FileHandle hFile,
				 MP4TrackId trackId DEFAULT(MP4_INVALID_TRACK_ID));

void MP4SetVideoProfileLevel(MP4FileHandle hFile, u_int8_t value);

u_int8_t MP4GetAudioProfileLevel(MP4FileHandle hFile);

void MP4SetAudioProfileLevel(MP4FileHandle hFile, u_int8_t value);

u_int8_t MP4GetGraphicsProfileLevel(MP4FileHandle hFile);

bool MP4SetGraphicsProfileLevel(MP4FileHandle hFile, u_int8_t value);

/* generic file properties */
bool MP4HaveAtom(MP4FileHandle hFile, 
		 const char *atomName);

bool MP4GetIntegerProperty(
	MP4FileHandle hFile, 
	const char* propName,
	u_int64_t *retval );


bool MP4GetFloatProperty(
	MP4FileHandle hFile, 
	const char* propName,
	float *retvalue);

bool MP4GetStringProperty(
	MP4FileHandle hFile, 
	const char* propName,
	const char **retvalue);

bool MP4GetBytesProperty(
	MP4FileHandle hFile, 
	const char* propName,
	u_int8_t** ppValue, 
	u_int32_t* pValueSize);

bool MP4SetIntegerProperty(
	MP4FileHandle hFile, 
	const char* propName, 
	int64_t value);

bool MP4SetFloatProperty(
	MP4FileHandle hFile, 
	const char* propName, 
	float value);

bool MP4SetStringProperty(
	MP4FileHandle hFile, const char* propName, const char* value);

bool MP4SetBytesProperty(
	MP4FileHandle hFile, const char* propName, 
	const u_int8_t* pValue, u_int32_t valueSize);

/* track operations */

MP4TrackId MP4AddTrack(
	MP4FileHandle hFile, 
	const char* type);

MP4TrackId MP4AddSystemsTrack(
	MP4FileHandle hFile, 
	const char* type);

MP4TrackId MP4AddODTrack(
	MP4FileHandle hFile);

MP4TrackId MP4AddSceneTrack(
	MP4FileHandle hFile);

MP4TrackId MP4AddAudioTrack(
	MP4FileHandle hFile, 
	u_int32_t timeScale, 
	MP4Duration sampleDuration,
	u_int8_t audioType DEFAULT(MP4_MPEG4_AUDIO_TYPE));

typedef struct mp4v2_ismacryp_session_params {
  u_int32_t  scheme_type;
  u_int16_t  scheme_version;
  u_int8_t  key_ind_len;
  u_int8_t  iv_len;
  u_int8_t  selective_enc;
  const char   *kms_uri;
} mp4v2_ismacrypParams;

// API to initialize ismacryp properties to sensible defaults
// if input param is null then mallocs a params struct
mp4v2_ismacrypParams *MP4DefaultISMACrypParams(mp4v2_ismacrypParams *ptr);

MP4TrackId MP4AddEncAudioTrack(
	MP4FileHandle hFile, 
	u_int32_t timeScale, 
	MP4Duration sampleDuration,
        mp4v2_ismacrypParams *icPp,
	u_int8_t audioType DEFAULT(MP4_MPEG4_AUDIO_TYPE));

MP4TrackId MP4AddAmrAudioTrack(
		MP4FileHandle hFile,
		u_int32_t timeScale,
		u_int16_t modeSet,
		u_int8_t modeChangePeriod,
		u_int8_t framesPerSample,
		bool isAmrWB);

void MP4SetAmrVendor(
		MP4FileHandle hFile,
		MP4TrackId trackId,
		u_int32_t vendor);

void MP4SetAmrDecoderVersion(
		MP4FileHandle hFile,
		MP4TrackId trackId,
		u_int8_t decoderVersion);

void MP4SetAmrModeSet(MP4FileHandle hFile, MP4TrackId trakId, uint16_t modeSet);
uint16_t MP4GetAmrModeSet(MP4FileHandle hFile, MP4TrackId trackId);

MP4TrackId MP4AddHrefTrack(MP4FileHandle hFile, 
			   uint32_t timeScale, 
			   MP4Duration sampleDuration,
			   const char *base_url DEFAULT(NULL));

const char *MP4GetHrefTrackBaseUrl(MP4FileHandle hFile,
				   MP4TrackId trackId);
MP4TrackId MP4AddVideoTrack(
	MP4FileHandle hFile, 
	u_int32_t timeScale, 
	MP4Duration sampleDuration,
	u_int16_t width, 
	u_int16_t height,
	u_int8_t videoType DEFAULT(MP4_MPEG4_VIDEO_TYPE));

MP4TrackId MP4AddEncVideoTrack(
	MP4FileHandle hFile, 
	u_int32_t timeScale, 
	MP4Duration sampleDuration,
	u_int16_t width, 
	u_int16_t height,
        mp4v2_ismacrypParams *icPp,
	u_int8_t videoType DEFAULT(MP4_MPEG4_VIDEO_TYPE),
	const char *oFormat DEFAULT(NULL));

MP4TrackId MP4AddH264VideoTrack(
				MP4FileHandle hFile,
				u_int32_t timeScale, 
				MP4Duration sampleDuration, 
				u_int16_t width, 
				u_int16_t height, 
				uint8_t AVCProfileIndication,
				uint8_t profile_compat,
				uint8_t AVCLevelIndication,
				uint8_t sampleLenFieldSizeMinusOne);

MP4TrackId MP4AddEncH264VideoTrack(
				MP4FileHandle dstFile,
				u_int32_t timeScale, 
				MP4Duration sampleDuration, 
				u_int16_t width, 
				u_int16_t height, 
				MP4FileHandle srcFile,
				MP4TrackId srcTrackId,
				mp4v2_ismacrypParams *icPp);

void MP4AddH264SequenceParameterSet(MP4FileHandle hFile,
				    MP4TrackId trackId,
				    const uint8_t *pSequence,
				    uint16_t sequenceLen);
void MP4AddH264PictureParameterSet(MP4FileHandle hFile,
				   MP4TrackId trackId,
				   const uint8_t *pPict,
				   uint16_t pictLen);
void MP4SetH263Vendor(
		MP4FileHandle hFile,
		MP4TrackId trackId,
		u_int32_t vendor);

void  MP4SetH263DecoderVersion(
		MP4FileHandle hFile,
		MP4TrackId trackId,
		u_int8_t decoderVersion);

void MP4SetH263Bitrates(
		MP4FileHandle hFile,
		MP4TrackId trackId,
		u_int32_t avgBitrate,
		u_int32_t maxBitrate);

MP4TrackId MP4AddH263VideoTrack(
		MP4FileHandle hFile,
		u_int32_t timeScale,
		MP4Duration sampleDuration,
		u_int16_t width,
		u_int16_t height,
		u_int8_t h263Level,
		u_int8_t h263Profile,
		u_int32_t avgBitrate,
		u_int32_t maxBitrate);

MP4TrackId MP4AddHintTrack(
	MP4FileHandle hFile, 
	MP4TrackId refTrackId);

MP4TrackId MP4AddTextTrack(
	MP4FileHandle hFile, 
	MP4TrackId refTrackId);

MP4TrackId MP4AddChapterTextTrack(
	MP4FileHandle hFile, 
	MP4TrackId refTrackId);

MP4TrackId MP4CloneTrack(
	MP4FileHandle srcFile, 
	MP4TrackId srcTrackId,
	MP4FileHandle dstFile DEFAULT(MP4_INVALID_FILE_HANDLE),
	MP4TrackId dstHintTrackReferenceTrack DEFAULT(MP4_INVALID_TRACK_ID));

MP4TrackId MP4EncAndCloneTrack(
        MP4FileHandle srcFile,
        MP4TrackId srcTrackId,
        mp4v2_ismacrypParams *icPp,
        MP4FileHandle dstFile DEFAULT(MP4_INVALID_FILE_HANDLE),
        MP4TrackId dstHintTrackReferenceTrack DEFAULT(MP4_INVALID_TRACK_ID));

MP4TrackId MP4CopyTrack(
	MP4FileHandle srcFile, 
	MP4TrackId srcTrackId,
	MP4FileHandle dstFile DEFAULT(MP4_INVALID_FILE_HANDLE), 
	bool applyEdits DEFAULT(false),
	MP4TrackId dstHintTrackReferenceTrack DEFAULT(MP4_INVALID_TRACK_ID));

typedef u_int32_t (*encryptFunc_t)(u_int32_t, u_int32_t, u_int8_t*, u_int32_t*, u_int8_t **);

MP4TrackId MP4EncAndCopyTrack(
	MP4FileHandle srcFile, 
	MP4TrackId srcTrackId,
        mp4v2_ismacrypParams *icPp,
        encryptFunc_t encfcnp,
        u_int32_t encfcnparam1,
	MP4FileHandle dstFile DEFAULT(MP4_INVALID_FILE_HANDLE),
	bool applyEdits DEFAULT(false),
	MP4TrackId dstHintTrackReferenceTrack DEFAULT(MP4_INVALID_TRACK_ID));

void MP4DeleteTrack(
	MP4FileHandle hFile, 
	MP4TrackId trackId);

u_int32_t MP4GetNumberOfTracks(
	MP4FileHandle hFile, 
	const char* type DEFAULT(NULL),
	u_int8_t subType DEFAULT(0));

MP4TrackId MP4FindTrackId(
	MP4FileHandle hFile, 
	u_int16_t index, 
	const char* type DEFAULT(NULL),
	u_int8_t subType DEFAULT(0));

u_int16_t MP4FindTrackIndex(
	MP4FileHandle hFile, 
	MP4TrackId trackId);

/* track properties */

/* specific track properties */

bool MP4HaveTrackAtom(MP4FileHandle hFile, 
		      MP4TrackId trackId, 
		      const char *atomname);

const char* MP4GetTrackType(
	MP4FileHandle hFile, 
	MP4TrackId trackId);

const char *MP4GetTrackMediaDataName(MP4FileHandle hFile,
				     MP4TrackId trackId);

// MP4GetTrackMediaDataOriginalFormat is to be used to get the original
// MediaDataName if a track has been encrypted.
bool MP4GetTrackMediaDataOriginalFormat(MP4FileHandle hFile,
	    MP4TrackId trackId, char *originalFormat, u_int32_t buflen);

MP4Duration MP4GetTrackDuration(
	MP4FileHandle hFile, 
	MP4TrackId trackId);

u_int32_t MP4GetTrackTimeScale(
	MP4FileHandle hFile, 
	MP4TrackId trackId);

void MP4SetTrackTimeScale(
	MP4FileHandle hFile, 
	MP4TrackId trackId, 
	u_int32_t value);

u_int8_t MP4GetTrackAudioMpeg4Type(
	MP4FileHandle hFile, 
	MP4TrackId trackId);

u_int8_t MP4GetTrackEsdsObjectTypeId(
	MP4FileHandle hFile, 
	MP4TrackId trackId);

/* returns MP4_INVALID_DURATION if track samples do not have a fixed duration */
MP4Duration MP4GetTrackFixedSampleDuration(
	MP4FileHandle hFile, 
	MP4TrackId trackId);

u_int32_t MP4GetTrackBitRate(
	MP4FileHandle hFile, 
	MP4TrackId trackId);

bool MP4GetTrackVideoMetadata(MP4FileHandle hFile,
			      MP4TrackId trackId,
			      uint8_t **ppConfig,
			      uint32_t *pConfigSize);

bool MP4GetTrackESConfiguration(
	MP4FileHandle hFile, 
	MP4TrackId trackId, 
	u_int8_t** ppConfig, 
	u_int32_t* pConfigSize);

bool MP4SetTrackESConfiguration(
	MP4FileHandle hFile, 
	MP4TrackId trackId, 
	const u_int8_t* pConfig, 
	u_int32_t configSize);

/* h264 information routines */
bool MP4GetTrackH264ProfileLevel(MP4FileHandle hFile,
				 MP4TrackId trackId,
				 uint8_t *pProfile,
				 uint8_t *pLevel);
void MP4GetTrackH264SeqPictHeaders(MP4FileHandle hFile,
				   MP4TrackId trackId,
				   uint8_t ***pSeqHeaders,
				   uint32_t **pSeqHeaderSize,
				   uint8_t ***pPictHeader,
				   uint32_t **pPictHeaderSize);
bool MP4GetTrackH264LengthSize(MP4FileHandle hFile,
			       MP4TrackId trackId,
			       uint32_t *pLength);
MP4SampleId MP4GetTrackNumberOfSamples(
	MP4FileHandle hFile, 
	MP4TrackId trackId);

u_int16_t MP4GetTrackVideoWidth(
	MP4FileHandle hFile, 
	MP4TrackId trackId);

u_int16_t MP4GetTrackVideoHeight(
	MP4FileHandle hFile, 
	MP4TrackId trackId);

double MP4GetTrackVideoFrameRate(
	MP4FileHandle hFile, 
	MP4TrackId trackId);

int MP4GetTrackAudioChannels(MP4FileHandle hFile, 
				  MP4TrackId trackId);

bool MP4IsIsmaCrypMediaTrack(
        MP4FileHandle hFile,
        MP4TrackId trackId);

/* generic track properties */

bool MP4HaveTrackAtom(MP4FileHandle hFile, 
		      MP4TrackId trackId, 
		      const char *atomName);
			
bool MP4GetTrackIntegerProperty(
	MP4FileHandle hFile, 
	MP4TrackId trackId, 
	const char* propName,
	u_int64_t *retvalue);

bool MP4GetTrackFloatProperty(
	MP4FileHandle hFile, 
	MP4TrackId trackId, 
	const char* propName,
	float *ret_value);

bool MP4GetTrackStringProperty(
	MP4FileHandle hFile, 
	MP4TrackId trackId, 
	const char* propName,
	const char **retvalue);

bool MP4GetTrackBytesProperty(
	MP4FileHandle hFile, 
	MP4TrackId trackId, 
	const char* propName,
	u_int8_t** ppValue, 
	u_int32_t* pValueSize);

bool MP4SetTrackIntegerProperty(
	MP4FileHandle hFile, 
	MP4TrackId trackId, 
	const char* propName, 
	int64_t value);

bool MP4SetTrackFloatProperty(
	MP4FileHandle hFile, 
	MP4TrackId trackId, 
	const char* propName, 
	float value);

bool MP4SetTrackStringProperty(
	MP4FileHandle hFile, 
	MP4TrackId trackId, 
	const char* propName, 
	const char* value);

bool MP4SetTrackBytesProperty(
	MP4FileHandle hFile, 
	MP4TrackId trackId, 
	const char* propName, 
	const u_int8_t* pValue, 
	u_int32_t valueSize);

/* sample operations */

bool MP4ReadSample(
	/* input parameters */
	MP4FileHandle hFile,
	MP4TrackId trackId, 
	MP4SampleId sampleId,
	/* input/output parameters */
	u_int8_t** ppBytes, 
	u_int32_t* pNumBytes, 
	/* output parameters */
	MP4Timestamp* pStartTime DEFAULT(NULL), 
	MP4Duration* pDuration DEFAULT(NULL),
	MP4Duration* pRenderingOffset DEFAULT(NULL), 
	bool* pIsSyncSample DEFAULT(NULL));

/* uses (unedited) time to specify sample instead of sample id */
bool MP4ReadSampleFromTime(
	/* input parameters */
	MP4FileHandle hFile,
	MP4TrackId trackId, 
	MP4Timestamp when,
	/* input/output parameters */
	u_int8_t** ppBytes, 
	u_int32_t* pNumBytes, 
	/* output parameters */
	MP4Timestamp* pStartTime DEFAULT(NULL), 
	MP4Duration* pDuration DEFAULT(NULL),
	MP4Duration* pRenderingOffset DEFAULT(NULL), 
	bool* pIsSyncSample DEFAULT(NULL));

bool MP4WriteSample(
	MP4FileHandle hFile,
	MP4TrackId trackId,
	const u_int8_t* pBytes, 
	u_int32_t numBytes,
	MP4Duration duration DEFAULT(MP4_INVALID_DURATION),
	MP4Duration renderingOffset DEFAULT(0), 
	bool isSyncSample DEFAULT(true));

bool MP4CopySample(
	MP4FileHandle srcFile,
	MP4TrackId srcTrackId, 
	MP4SampleId srcSampleId,
	MP4FileHandle dstFile DEFAULT(MP4_INVALID_FILE_HANDLE),
	MP4TrackId dstTrackId DEFAULT(MP4_INVALID_TRACK_ID),
	MP4Duration dstSampleDuration DEFAULT(MP4_INVALID_DURATION));

bool MP4EncAndCopySample(
	MP4FileHandle srcFile,
	MP4TrackId srcTrackId, 
	MP4SampleId srcSampleId,
        encryptFunc_t encfcnp,
        u_int32_t encfcnparam1,
	MP4FileHandle dstFile DEFAULT(MP4_INVALID_FILE_HANDLE),
	MP4TrackId dstTrackId DEFAULT(MP4_INVALID_TRACK_ID),
	MP4Duration dstSampleDuration DEFAULT(MP4_INVALID_DURATION));

/* Note this function is not yet implemented */
bool MP4ReferenceSample(
	MP4FileHandle srcFile,
	MP4TrackId srcTrackId, 
	MP4SampleId srcSampleId,
	MP4FileHandle dstFile,
	MP4TrackId dstTrackId,
	MP4Duration dstSampleDuration DEFAULT(MP4_INVALID_DURATION));

u_int32_t MP4GetSampleSize(
	MP4FileHandle hFile,
	MP4TrackId trackId, 
	MP4SampleId sampleId);

u_int32_t MP4GetTrackMaxSampleSize(
	MP4FileHandle hFile,
	MP4TrackId trackId); 

MP4SampleId MP4GetSampleIdFromTime(
	MP4FileHandle hFile,
	MP4TrackId trackId, 
	MP4Timestamp when, 
	bool wantSyncSample DEFAULT(false));

MP4Timestamp MP4GetSampleTime(
	MP4FileHandle hFile,
	MP4TrackId trackId, 
	MP4SampleId sampleId);

MP4Duration MP4GetSampleDuration(
	MP4FileHandle hFile,
	MP4TrackId trackId, 
	MP4SampleId sampleId);

MP4Duration MP4GetSampleRenderingOffset(
	MP4FileHandle hFile,
	MP4TrackId trackId, 
	MP4SampleId sampleId);

bool MP4SetSampleRenderingOffset(
	MP4FileHandle hFile,
	MP4TrackId trackId, 
	MP4SampleId sampleId,
	MP4Duration renderingOffset);

int8_t MP4GetSampleSync(
	MP4FileHandle hFile,
	MP4TrackId trackId, 
	MP4SampleId sampleId);

/* rtp hint track operations */

bool MP4GetHintTrackRtpPayload(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId,
	char** ppPayloadName DEFAULT(NULL),
	u_int8_t* pPayloadNumber DEFAULT(NULL),
	u_int16_t* pMaxPayloadSize DEFAULT(NULL),
	char **ppEncodingParams DEFAULT(NULL));

#define MP4_SET_DYNAMIC_PAYLOAD 0xff

bool MP4SetHintTrackRtpPayload(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId,
	const char* pPayloadName,
	u_int8_t* pPayloadNumber,
	u_int16_t maxPayloadSize DEFAULT(0),
	const char *encode_params DEFAULT(NULL),
	bool include_rtp_map DEFAULT(true),
	bool include_mpeg4_esid DEFAULT(true));

const char* MP4GetSessionSdp(
	MP4FileHandle hFile);

bool MP4SetSessionSdp(
	MP4FileHandle hFile,
	const char* sdpString);

bool MP4AppendSessionSdp(
	MP4FileHandle hFile,
	const char* sdpString);

const char* MP4GetHintTrackSdp(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId);

bool MP4SetHintTrackSdp(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId,
	const char* sdpString);

bool MP4AppendHintTrackSdp(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId,
	const char* sdpString);

MP4TrackId MP4GetHintTrackReferenceTrackId(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId);

bool MP4ReadRtpHint(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId,
	MP4SampleId hintSampleId,
	u_int16_t* pNumPackets DEFAULT(NULL));

u_int16_t MP4GetRtpHintNumberOfPackets(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId);

int8_t MP4GetRtpPacketBFrame(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId,
	u_int16_t packetIndex);

int32_t MP4GetRtpPacketTransmitOffset(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId,
	u_int16_t packetIndex);

bool MP4ReadRtpPacket(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId,
	u_int16_t packetIndex,
	u_int8_t** ppBytes, 
	u_int32_t* pNumBytes,
	u_int32_t ssrc DEFAULT(0),
	bool includeHeader DEFAULT(true),
	bool includePayload DEFAULT(true));

MP4Timestamp MP4GetRtpTimestampStart(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId);

bool MP4SetRtpTimestampStart(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId,
	MP4Timestamp rtpStart);

bool MP4AddRtpHint(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId);

bool MP4AddRtpVideoHint(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId,
	bool isBframe DEFAULT(false), 
	u_int32_t timestampOffset DEFAULT(0));

bool MP4AddRtpPacket(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId,
	bool setMbit DEFAULT(false),
	int32_t transmitOffset DEFAULT(0));

bool MP4AddRtpImmediateData(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId,
	const u_int8_t* pBytes,
	u_int32_t numBytes);

bool MP4AddRtpSampleData(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId,
	MP4SampleId sampleId,
	u_int32_t dataOffset,
	u_int32_t dataLength);

bool MP4AddRtpESConfigurationPacket(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId);

bool MP4WriteRtpHint(
	MP4FileHandle hFile,
	MP4TrackId hintTrackId,
	MP4Duration duration,
	bool isSyncSample DEFAULT(true));

/* 3GP specific utilities */

bool MP4Make3GPCompliant(
	const char* fileName,
	u_int32_t verbosity DEFAULT(0),
	char* majorBrand DEFAULT(0),
	u_int32_t minorVersion DEFAULT(0),
	char** supportedBrands DEFAULT(NULL),
	u_int32_t supportedBrandsCount DEFAULT(0),
	bool deleteIodsAtom DEFAULT(true));

/* ISMA specific utilities */

bool MP4MakeIsmaCompliant(const char* fileName, 
	u_int32_t verbosity DEFAULT(0),
	bool addIsmaComplianceSdp DEFAULT(true));

char* MP4MakeIsmaSdpIod(
	u_int8_t videoProfile,
	u_int32_t videoBitrate,
	u_int8_t* videoConfig,
	u_int32_t videoConfigLength,
	u_int8_t audioProfile,
	u_int32_t audioBitrate,
	u_int8_t* audioConfig,
	u_int32_t audioConfigLength,
	u_int32_t verbosity DEFAULT(0));

/* edit list */

/* NOTE this section of functionality 
 * has not yet been fully tested 
 */

MP4EditId MP4AddTrackEdit(
	MP4FileHandle hFile,
	MP4TrackId trackId,
	MP4EditId editId DEFAULT(MP4_INVALID_EDIT_ID),
	MP4Timestamp startTime DEFAULT(0),
	MP4Duration duration DEFAULT(0),
	bool dwell DEFAULT(false));

bool MP4DeleteTrackEdit(
	MP4FileHandle hFile,
	MP4TrackId trackId,
	MP4EditId editId);

u_int32_t MP4GetTrackNumberOfEdits(
	MP4FileHandle hFile,
	MP4TrackId trackId);

MP4Timestamp MP4GetTrackEditStart(
	MP4FileHandle hFile,
	MP4TrackId trackId,
	MP4EditId editId);

MP4Duration MP4GetTrackEditTotalDuration(
	MP4FileHandle hFile,
	MP4TrackId trackId,
	MP4EditId editId DEFAULT(MP4_INVALID_EDIT_ID));

MP4Timestamp MP4GetTrackEditMediaStart(
	MP4FileHandle hFile,
	MP4TrackId trackId,
	MP4EditId editId);

bool MP4SetTrackEditMediaStart(
	MP4FileHandle hFile,
	MP4TrackId trackId,
	MP4EditId editId,
	MP4Timestamp startTime);

MP4Duration MP4GetTrackEditDuration(
	MP4FileHandle hFile,
	MP4TrackId trackId,
	MP4EditId editId);

bool MP4SetTrackEditDuration(
	MP4FileHandle hFile,
	MP4TrackId trackId,
	MP4EditId editId,
	MP4Duration duration);

int8_t MP4GetTrackEditDwell(
	MP4FileHandle hFile,
	MP4TrackId trackId,
	MP4EditId editId);

bool MP4SetTrackEditDwell(
	MP4FileHandle hFile,
	MP4TrackId trackId,
	MP4EditId editId,
	bool dwell);

bool MP4ReadSampleFromEditTime(
	/* input parameters */
	MP4FileHandle hFile,
	MP4TrackId trackId, 
	MP4Timestamp when, 
	/* input/output parameters */
	u_int8_t** ppBytes, 
	u_int32_t* pNumBytes, 
	/* output parameters */
	MP4Timestamp* pStartTime DEFAULT(NULL), 
	MP4Duration* pDuration DEFAULT(NULL),
	MP4Duration* pRenderingOffset DEFAULT(NULL), 
	bool* pIsSyncSample DEFAULT(NULL));

MP4SampleId MP4GetSampleIdFromEditTime(
	MP4FileHandle hFile,
	MP4TrackId trackId, 
	MP4Timestamp when, 
	MP4Timestamp* pStartTime DEFAULT(NULL), 
	MP4Duration* pDuration DEFAULT(NULL));
/* iTunes metadata handling */
bool MP4MetadataDelete(MP4FileHandle hFile);
bool MP4GetMetadataByIndex(MP4FileHandle hFile, u_int32_t index,
			   char** ppName, // need to free memory
			   u_int8_t** ppValue,  // need to free
			   u_int32_t* pValueSize);
bool MP4SetMetadataName(MP4FileHandle hFile, const char* value);
bool MP4GetMetadataName(MP4FileHandle hFile, char** value);
bool MP4DeleteMetadataName(MP4FileHandle hFile);
bool MP4SetMetadataArtist(MP4FileHandle hFile, const char* value);
bool MP4GetMetadataArtist(MP4FileHandle hFile, char** value);
bool MP4DeleteMetadataArtist(MP4FileHandle hFile);
bool MP4SetMetadataWriter(MP4FileHandle hFile, const char* value);
bool MP4GetMetadataWriter(MP4FileHandle hFile, char** value);
bool MP4DeleteMetadataWriter(MP4FileHandle hFile);
bool MP4SetMetadataComment(MP4FileHandle hFile, const char* value);
bool MP4GetMetadataComment(MP4FileHandle hFile, char** value);
bool MP4DeleteMetadataComment(MP4FileHandle hFile);
bool MP4SetMetadataTool(MP4FileHandle hFile, const char* value);
bool MP4GetMetadataTool(MP4FileHandle hFile, char** value);
bool MP4DeleteMetadataTool(MP4FileHandle hFile);
bool MP4SetMetadataYear(MP4FileHandle hFile, const char* value);
bool MP4GetMetadataYear(MP4FileHandle hFile, char** value);
bool MP4DeleteMetadataYear(MP4FileHandle hFile);
bool MP4SetMetadataAlbum(MP4FileHandle hFile, const char* value);
bool MP4GetMetadataAlbum(MP4FileHandle hFile, char** value);
bool MP4DeleteMetadataAlbum(MP4FileHandle hFile);
bool MP4SetMetadataTrack(MP4FileHandle hFile,
			 u_int16_t track, u_int16_t totalTracks);
bool MP4GetMetadataTrack(MP4FileHandle hFile,
			 u_int16_t* track, u_int16_t* totalTracks);
bool MP4DeleteMetadataTrack(MP4FileHandle hFile);
bool MP4SetMetadataDisk(MP4FileHandle hFile,
			u_int16_t disk, u_int16_t totalDisks);
bool MP4GetMetadataDisk(MP4FileHandle hFile,
			u_int16_t* disk, u_int16_t* totalDisks);
bool MP4DeleteMetadataDisk(MP4FileHandle hFile);
bool MP4SetMetadataGenre(MP4FileHandle hFile, const char *genre);
bool MP4GetMetadataGenre(MP4FileHandle hFile, char **genre);
bool MP4DeleteMetadataGenre(MP4FileHandle hFile);
bool MP4SetMetadataGrouping(MP4FileHandle hFile, const char *grouping);
bool MP4GetMetadataGrouping(MP4FileHandle hFile, char **grouping);
bool MP4DeleteMetadataGrouping(MP4FileHandle hFile);
bool MP4SetMetadataTempo(MP4FileHandle hFile, u_int16_t tempo);
bool MP4GetMetadataTempo(MP4FileHandle hFile, u_int16_t* tempo);
bool MP4DeleteMetadataTempo(MP4FileHandle hFile);
bool MP4SetMetadataCompilation(MP4FileHandle hFile, u_int8_t cpl);
bool MP4GetMetadataCompilation(MP4FileHandle hFile, u_int8_t* cpl);
bool MP4DeleteMetadataCompilation(MP4FileHandle hFile);
bool MP4SetMetadataPartOfGaplessAlbum(MP4FileHandle hFile, uint8_t pgap);
bool MP4GetMetadataPartOfGaplessAlbum(MP4FileHandle hFile, uint8_t *pgap);
bool MP4DeleteMetadataPartOfGaplessAlbum(MP4FileHandle hFile);
bool MP4SetMetadataCoverArt(MP4FileHandle hFile,
			    u_int8_t *coverArt, u_int32_t size);
bool MP4GetMetadataCoverArt(MP4FileHandle hFile,
			    u_int8_t **coverArt, u_int32_t* size,
			    uint32_t index DEFAULT(0));
u_int32_t MP4GetMetadataCoverArtCount(MP4FileHandle hFile);
bool MP4DeleteMetadataCoverArt(MP4FileHandle hFile);
bool MP4SetMetadataAlbumArtist(MP4FileHandle hFile, const char* value);
bool MP4GetMetadataAlbumArtist(MP4FileHandle hFile,    char** value);
bool MP4DeleteMetadataAlbumArtist(MP4FileHandle hFile); 


bool MP4SetMetadataFreeForm(MP4FileHandle hFile, const char *name,
			    const u_int8_t* pValue, u_int32_t valueSize, const char *owner DEFAULT(NULL));
bool MP4GetMetadataFreeForm(MP4FileHandle hFile, const char *name,
			    u_int8_t** pValue, u_int32_t* valueSize, const char *owner DEFAULT(NULL));
bool MP4DeleteMetadataFreeForm(MP4FileHandle hFile, const char *name, const char *owner DEFAULT(NULL));

/* time conversion utilties */

/* predefined values for timeScale parameter below */
#define MP4_SECONDS_TIME_SCALE		1
#define MP4_MILLISECONDS_TIME_SCALE 1000
#define MP4_MICROSECONDS_TIME_SCALE 1000000
#define MP4_NANOSECONDS_TIME_SCALE 	1000000000

#define MP4_SECS_TIME_SCALE 	MP4_SECONDS_TIME_SCALE
#define MP4_MSECS_TIME_SCALE	MP4_MILLISECONDS_TIME_SCALE
#define MP4_USECS_TIME_SCALE	MP4_MICROSECONDS_TIME_SCALE
#define MP4_NSECS_TIME_SCALE	MP4_NANOSECONDS_TIME_SCALE

u_int64_t MP4ConvertFromMovieDuration(
	MP4FileHandle hFile,
	MP4Duration duration,
	u_int32_t timeScale);

u_int64_t MP4ConvertFromTrackTimestamp(
	MP4FileHandle hFile,
	MP4TrackId trackId, 
	MP4Timestamp timeStamp,
	u_int32_t timeScale);

MP4Timestamp MP4ConvertToTrackTimestamp(
	MP4FileHandle hFile,
	MP4TrackId trackId, 
	u_int64_t timeStamp,
	u_int32_t timeScale);

u_int64_t MP4ConvertFromTrackDuration(
	MP4FileHandle hFile,
	MP4TrackId trackId, 
	MP4Duration duration,
	u_int32_t timeScale);

MP4Duration MP4ConvertToTrackDuration(
	MP4FileHandle hFile,
	MP4TrackId trackId, 
	u_int64_t duration,
	u_int32_t timeScale);

char* MP4BinaryToBase16(
	const u_int8_t* pData, 
	u_int32_t dataSize);

char* MP4BinaryToBase64(
	const u_int8_t* pData, 
	u_int32_t dataSize);

uint8_t *Base64ToBinary(const char *pData, 
			uint32_t decodeSize, 
			uint32_t *pDataSize);
void MP4Free(void *p);
#ifdef __cplusplus
}
#endif

/* undefined our utlity macro to avoid conflicts */
#undef DEFAULT

#endif /* __MP4_INCLUDED__ */
