#ifndef MP4V2_CHAPTER_H
#define MP4V2_CHAPTER_H

/**************************************************************************//**
 *
 *  @defgroup mp4_chapter MP4v2 Chapter
 *  @{
 *
 *****************************************************************************/

/** The maximum length of a QuickTime chapter title (in 8-bit chars)
 */
#define MP4V2_CHAPTER_TITLE_MAX 1023

/** Chapter item.
 *  This item defines various attributes for a chapter.
 *  @ingroup mp4_chapter
 */
typedef struct MP4Chapter_s {
    MP4Duration duration; /**< duration of chapter in milliseconds */
    char title[MP4V2_CHAPTER_TITLE_MAX+1]; /**< title of chapter */
} MP4Chapter_t;

/** Known chapter types.
 *  @ingroup mp4_chapter
 */
typedef enum {
    MP4ChapterTypeNone = 0, /**< no chapters found return value */
    MP4ChapterTypeAny  = 1, /**< any or all known chapter types */
    MP4ChapterTypeQt   = 2, /**< QuickTime chapter type */
    MP4ChapterTypeNero = 4  /**< Nero chapter type */
} MP4ChapterType;

/** Add a QuickTime chapter.
 *
 *  This function adds a QuickTime chapter to file <b>hFile</b>.
 *
 *  @param hFile handle of file to add chapter.
 *  @param chapterTrackId ID of chapter track or #MP4_INVALID_TRACK_ID
 *      if unknown.
 *  @param chapterDuration duration (in the timescale of the chapter track).
 *  @param chapterTitle title text for the chapter or NULL to use default
 *      title format ("Chapter %03d", n) where n is the chapter number.
 */
MP4V2_EXPORT
void MP4AddChapter(
    MP4FileHandle hFile,
    MP4TrackId    chapterTrackId,
    MP4Duration   chapterDuration,
    const char*   chapterTitle DEFAULT(0));

/** Add a QuickTime chapter track.
 *
 *  This function adds a chapter (text) track to file <b>hFile</b>.
 *  The optional parameter <b>timescale</b> may be supplied to give the new
 *  chapter a specific timescale. Otherwise the chapter track will have
 *  the same timescale as the reference track defined in parameter refTrackId.
 *
 *  @param hFile handle of file to add chapter track.
 *  @param refTrackId ID of the track that will reference the chapter track.
 *  @param timescale the timescale of the chapter track or 0 to use the
 *      timescale of track specified by <b>refTrackId</b>.
 *
 *  @return ID of the created chapter track.
 */
MP4V2_EXPORT
MP4TrackId MP4AddChapterTextTrack(
    MP4FileHandle hFile,
    MP4TrackId    refTrackId,
    uint32_t      timescale DEFAULT(0) );

/** Add a Nero chapter.
 *
 *  This function adds a Nero chapter to file <b>hFile</b>.
 *
 *  @param hFile handle of file to add chapter.
 *  @param chapterStart the start time of the chapter in 100 nanosecond units
 *  @param chapterTitle title text for the chapter or NULL to use default
 *      title format ("Chapter %03d", n) where n is the chapter number.
 */
MP4V2_EXPORT
void MP4AddNeroChapter(
    MP4FileHandle hFile,
    MP4Timestamp  chapterStart,
    const char*   chapterTitle DEFAULT(0));

/** Convert chapters to another type.
 *
 *  This function converts existing chapters in file <b>hFile</b>
 *  from one type to another type.
 *  Conversion from Nero to QuickTime or QuickTime to Nero is supported.
 *
 *  @param hFile handle of file to convert.
 *  @param toChapterType the chapter type to convert to:
 *      @li #MP4ChapterTypeQt (convert from Nero to Qt)
 *      @li #MP4ChapterTypeNero (convert from Qt to Nero)
 *
 *  @return the chapter type before conversion or #MP4ChapterTypeNone
 *      if the source chapters do not exist
 *      or invalid <b>toChapterType</b> was specified.
 */
MP4V2_EXPORT
MP4ChapterType MP4ConvertChapters(
    MP4FileHandle  hFile,
    MP4ChapterType toChapterType DEFAULT(MP4ChapterTypeQt));

/** Delete chapters.
 *
 *  This function deletes existing chapters in file <b>hFile</b>.
 *
 *  @param hFile handle of file to delete chapters.
 *  @param chapterType the type of chapters to delete:
 *      @li #MP4ChapterTypeAny (delete all known chapter types)
 *      @li #MP4ChapterTypeQt
 *      @li #MP4ChapterTypeNero
 *  @param chapterTrackId ID of the chapter track if known,
 *      or #MP4_INVALID_TRACK_ID.
 *      Only applies when <b>chapterType</b>=#MP4ChapterTypeQt.
 *
 *  @return the type of deleted chapters
 */
MP4V2_EXPORT
MP4ChapterType MP4DeleteChapters(
    MP4FileHandle  hFile,
    MP4ChapterType chapterType DEFAULT(MP4ChapterTypeQt),
    MP4TrackId     chapterTrackId DEFAULT(MP4_INVALID_TRACK_ID) );

/** Get list of chapters.
 *
 *  This function gets a chpter list from file <b>hFile</b>.
 *
 *  @param hFile handle of file to read.
 *  @param chapterList address receiving array of chapter items.
 *      If a non-NULL is received the caller is responsible for freeing the
 *      memory with MP4Free().
 *  @param chapterCount address receiving count of items in array.
 *  @param chapterType the type of chapters to read:
 *      @li #MP4ChapterTypeAny (any chapters, searched in order of Qt, Nero)
 *      @li #MP4ChapterTypeQt
 *      @li #MP4ChapterTypeNero
 *
 *  @result the first type of chapters found.
 */
MP4V2_EXPORT
MP4ChapterType MP4GetChapters(
    MP4FileHandle  hFile,
    MP4Chapter_t** chapterList,
    uint32_t*      chapterCount,
    MP4ChapterType chapterType DEFAULT(MP4ChapterTypeQt));

/** Set list of chapters OKOK.
 *
 *  This functions sets the complete chapter list in file <b>hFile</b>.
 *  If any chapters of the same type already exist they will first
 *  be deleted.
 *
 *  @param hFile handle of file to modify.
 *  @param chapterList array of chapters items.
 *  @param chapterCount count of items in array.
 *  @param chapterType type of chapters to write:
 *      @li #MP4ChapterTypeAny (chapters of all types are written)
 *      @li #MP4ChapterTypeQt
 *      @li #MP4ChapterTypeNero
 *
 *  @return the type of chapters written.
 */
MP4V2_EXPORT
MP4ChapterType MP4SetChapters(
    MP4FileHandle hFile,
    MP4Chapter_t* chapterList,
    uint32_t       chapterCount,
    MP4ChapterType chapterType DEFAULT(MP4ChapterTypeQt));

/** @} ***********************************************************************/

#endif /* MP4V2_CHAPTER_H */
