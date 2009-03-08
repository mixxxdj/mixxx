#ifndef MP4V2_STREAMING_H
#define MP4V2_STREAMING_H

/**************************************************************************//**
 *
 *  @defgroup mp4_hint MP4v2 Streaming
 *  @{
 *
 *****************************************************************************/

MP4V2_EXPORT
bool MP4GetHintTrackRtpPayload(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId,
    char**        ppPayloadName DEFAULT(NULL),
    uint8_t*      pPayloadNumber DEFAULT(NULL),
    uint16_t*     pMaxPayloadSize DEFAULT(NULL),
    char**        ppEncodingParams DEFAULT(NULL) );

#define MP4_SET_DYNAMIC_PAYLOAD 0xff

MP4V2_EXPORT
bool MP4SetHintTrackRtpPayload(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId,
    const char*   pPayloadName,
    uint8_t*      pPayloadNumber,
    uint16_t      maxPayloadSize DEFAULT(0),
    const char *  encode_params DEFAULT(NULL),
    bool          include_rtp_map DEFAULT(true),
    bool          include_mpeg4_esid DEFAULT(true) );

MP4V2_EXPORT
const char* MP4GetSessionSdp(
    MP4FileHandle hFile );

MP4V2_EXPORT
bool MP4SetSessionSdp(
    MP4FileHandle hFile,
    const char*   sdpString );

MP4V2_EXPORT
bool MP4AppendSessionSdp(
    MP4FileHandle hFile,
    const char*   sdpString );

MP4V2_EXPORT
const char* MP4GetHintTrackSdp(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId );

MP4V2_EXPORT
bool MP4SetHintTrackSdp(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId,
    const char*   sdpString );

MP4V2_EXPORT
bool MP4AppendHintTrackSdp(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId,
    const char*   sdpString );

MP4V2_EXPORT
MP4TrackId MP4GetHintTrackReferenceTrackId(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId );

MP4V2_EXPORT
bool MP4ReadRtpHint(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId,
    MP4SampleId   hintSampleId,
    uint16_t*     pNumPackets DEFAULT(NULL) );

MP4V2_EXPORT
uint16_t MP4GetRtpHintNumberOfPackets(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId );

MP4V2_EXPORT
int8_t MP4GetRtpPacketBFrame(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId,
    uint16_t      packetIndex );

MP4V2_EXPORT
int32_t MP4GetRtpPacketTransmitOffset(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId,
    uint16_t      packetIndex );

MP4V2_EXPORT
bool MP4ReadRtpPacket(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId,
    uint16_t      packetIndex,
    uint8_t**     ppBytes,
    uint32_t*     pNumBytes,
    uint32_t      ssrc DEFAULT(0),
    bool          includeHeader DEFAULT(true),
    bool          includePayload DEFAULT(true) );

MP4V2_EXPORT
MP4Timestamp MP4GetRtpTimestampStart(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId );

MP4V2_EXPORT
bool MP4SetRtpTimestampStart(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId,
    MP4Timestamp  rtpStart );

MP4V2_EXPORT
bool MP4AddRtpHint(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId );

MP4V2_EXPORT
bool MP4AddRtpVideoHint(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId,
    bool          isBframe DEFAULT(false),
    uint32_t      timestampOffset DEFAULT(0) );

MP4V2_EXPORT
bool MP4AddRtpPacket(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId,
    bool          setMbit DEFAULT(false),
    int32_t       transmitOffset DEFAULT(0) );

MP4V2_EXPORT
bool MP4AddRtpImmediateData(
    MP4FileHandle  hFile,
    MP4TrackId     hintTrackId,
    const uint8_t* pBytes,
    uint32_t       numBytes );

MP4V2_EXPORT
bool MP4AddRtpSampleData(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId,
    MP4SampleId   sampleId,
    uint32_t      dataOffset,
    uint32_t      dataLength );

MP4V2_EXPORT
bool MP4AddRtpESConfigurationPacket(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId );

MP4V2_EXPORT
bool MP4WriteRtpHint(
    MP4FileHandle hFile,
    MP4TrackId    hintTrackId,
    MP4Duration   duration,
    bool          isSyncSample DEFAULT(true) );

/** @} ***********************************************************************/

#endif /* MP4V2_STREAMING_H */
