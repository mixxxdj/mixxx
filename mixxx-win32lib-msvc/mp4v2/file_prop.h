#ifndef MP4V2_FILE_PROP_H
#define MP4V2_FILE_PROP_H

/**************************************************************************//**
 *
 *  @defgroup mp4_file_prop MP4v2 File Property
 *  @{
 *
 *****************************************************************************/

/* generic props */

MP4V2_EXPORT
bool MP4HaveAtom(
    MP4FileHandle hFile,
    const char*   atomName );

MP4V2_EXPORT
bool MP4GetIntegerProperty(
    MP4FileHandle hFile,
    const char*   propName,
    uint64_t*     retval );

MP4V2_EXPORT
bool MP4GetFloatProperty(
    MP4FileHandle hFile,
    const char*   propName,
    float*        retvalue );

MP4V2_EXPORT
bool MP4GetStringProperty(
    MP4FileHandle hFile,
    const char*   propName,
    const char**  retvalue );

MP4V2_EXPORT
bool MP4GetBytesProperty(
    MP4FileHandle hFile,
    const char*   propName,
    uint8_t**     ppValue,
    uint32_t*     pValueSize );

MP4V2_EXPORT
bool MP4SetIntegerProperty(
    MP4FileHandle hFile,
    const char*   propName,
    int64_t       value );

MP4V2_EXPORT
bool MP4SetFloatProperty(
    MP4FileHandle hFile,
    const char*   propName,
    float         value );

MP4V2_EXPORT
bool MP4SetStringProperty(
    MP4FileHandle hFile,
    const char*   propName,
    const char*   value );

MP4V2_EXPORT
bool MP4SetBytesProperty(
    MP4FileHandle  hFile,
    const char*    propName,
    const uint8_t* pValue,
    uint32_t       valueSize );

/* specific props */

MP4V2_EXPORT
MP4Duration MP4GetDuration( MP4FileHandle hFile );

/** Get the time scale of the movie (file).
 *
 *  MP4GetTimeScale returns the time scale in units of ticks per second for
 *  the mp4 file. Caveat: tracks may use the same time scale as the movie
 *  or may use their own time scale.
 *
 *  @param hFile handle of file for operation.
 *
 *  @return timescale (ticks per second) of the mp4 file.
 */
MP4V2_EXPORT
uint32_t MP4GetTimeScale( MP4FileHandle hFile );

/** Set the time scale of the movie (file).
 *
 *  MP4SetTimeScale sets the time scale of the mp4 file. The time scale is
 *  in the number of clock ticks per second. Caveat:  tracks may use the
 *  same time scale as the movie or may use their own time scale.
 *
 *  @param hFile handle of file for operation.
 *  @param value desired timescale for the movie.
 *
 *  @return On success, true. On failure, false.
 */
MP4V2_EXPORT
bool MP4SetTimeScale( MP4FileHandle hFile, uint32_t value );

/** Change the general timescale of file hFile.
 *
 *  This function changes the general timescale of the file <b>hFile</b>
 *  to the new timescale <b>value</b> by recalculating all values that depend
 *  on the timescale in "moov.mvhd".
 *
 *  If the timescale is already equal to value nothing is done.
 *
 *  @param hFile handle of file to change.
 *  @param value the new timescale.
 */
MP4V2_EXPORT
void MP4ChangeMovieTimeScale( MP4FileHandle hFile, uint32_t value );

MP4V2_EXPORT
uint8_t MP4GetODProfileLevel( MP4FileHandle hFile );

MP4V2_EXPORT
bool MP4SetODProfileLevel( MP4FileHandle hFile, uint8_t value );

MP4V2_EXPORT
uint8_t MP4GetSceneProfileLevel( MP4FileHandle hFile );

MP4V2_EXPORT
bool MP4SetSceneProfileLevel( MP4FileHandle hFile, uint8_t value );

MP4V2_EXPORT
uint8_t MP4GetVideoProfileLevel(
    MP4FileHandle hFile,
    MP4TrackId    trackId DEFAULT(MP4_INVALID_TRACK_ID) );

MP4V2_EXPORT
void MP4SetVideoProfileLevel( MP4FileHandle hFile, uint8_t value );

MP4V2_EXPORT
uint8_t MP4GetAudioProfileLevel( MP4FileHandle hFile );

MP4V2_EXPORT
void MP4SetAudioProfileLevel( MP4FileHandle hFile, uint8_t value );

MP4V2_EXPORT
uint8_t MP4GetGraphicsProfileLevel( MP4FileHandle hFile );

MP4V2_EXPORT
bool MP4SetGraphicsProfileLevel( MP4FileHandle hFile, uint8_t value );

/** @} ***********************************************************************/

#endif /* MP4V2_FILE_PROP_H */
