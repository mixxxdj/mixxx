#ifndef MP4V2_META_H
#define MP4V2_META_H

/**************************************************************************//**
 *
 *  @defgroup mp4_meta MP4v2 Metadata (deprecated)
 *  @{
 *
 *****************************************************************************/

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the iTMF generic API.
  */
MP4V2_EXPORT
bool MP4MetadataDelete(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the iTMF generic API.
  */
MP4V2_EXPORT
bool MP4GetMetadataByIndex(
    MP4FileHandle hFile,
    uint32_t      index,
    char**        ppName,   /* need to free memory */
    uint8_t**     ppValue,  /* need to free */
    uint32_t*     pValueSize );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4SetMetadataName(
    MP4FileHandle hFile,
    const char*   value );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4GetMetadataName(
    MP4FileHandle hFile,
    char**        value );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4DeleteMetadataName(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4SetMetadataArtist(
    MP4FileHandle hFile,
    const char*   value );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4GetMetadataArtist(
    MP4FileHandle hFile,
    char**        value );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4DeleteMetadataArtist(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4SetMetadataWriter(
    MP4FileHandle hFile,
    const char*   value );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4GetMetadataWriter(
    MP4FileHandle hFile,
    char**        value );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4DeleteMetadataWriter(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4SetMetadataComment(
    MP4FileHandle hFile,
    const char*   value );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4GetMetadataComment(
    MP4FileHandle hFile,
    char**        value );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4DeleteMetadataComment(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4SetMetadataTool(
    MP4FileHandle hFile,
    const char*   value );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4GetMetadataTool(
    MP4FileHandle hFile,
    char**        value );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4DeleteMetadataTool(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4SetMetadataYear(
    MP4FileHandle hFile,
    const char*   value );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4GetMetadataYear(
    MP4FileHandle hFile,
    char**        value );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4DeleteMetadataYear(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4SetMetadataAlbum(
    MP4FileHandle hFile,
    const char*   value );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4GetMetadataAlbum(
    MP4FileHandle hFile,
    char**        value );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4DeleteMetadataAlbum(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_EXPORT
bool MP4SetMetadataTrack(
    MP4FileHandle hFile,
    uint16_t      track,
    uint16_t      totalTracks );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4GetMetadataTrack(
    MP4FileHandle hFile,
    uint16_t*     track,
    uint16_t*     totalTracks );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_EXPORT
bool MP4DeleteMetadataTrack(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_EXPORT
bool MP4SetMetadataDisk(
    MP4FileHandle hFile,
    uint16_t      disk,
    uint16_t      totalDisks );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4GetMetadataDisk(
    MP4FileHandle hFile,
    uint16_t*     disk,
    uint16_t*     totalDisks );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_EXPORT
bool MP4DeleteMetadataDisk(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4SetMetadataGenre(
    MP4FileHandle hFile,
    const char*   genre );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4GetMetadataGenre(
    MP4FileHandle hFile,
    char**        genre );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4DeleteMetadataGenre(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4SetMetadataGrouping(
    MP4FileHandle hFile,
    const char* grouping );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4GetMetadataGrouping(
    MP4FileHandle hFile,
    char**        grouping );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4DeleteMetadataGrouping(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4SetMetadataTempo(
    MP4FileHandle hFile,
    uint16_t      tempo );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4GetMetadataTempo(
    MP4FileHandle hFile,
    uint16_t*     tempo );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4DeleteMetadataTempo(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4SetMetadataCompilation(
    MP4FileHandle hFile,
    uint8_t       cpl );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4GetMetadataCompilation(
    MP4FileHandle hFile,
    uint8_t*      cpl );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4DeleteMetadataCompilation(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4SetMetadataPartOfGaplessAlbum(
    MP4FileHandle hFile,
    uint8_t       pgap );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4GetMetadataPartOfGaplessAlbum(
    MP4FileHandle hFile,
    uint8_t*      pgap );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4DeleteMetadataPartOfGaplessAlbum(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4SetMetadataCoverArt(
    MP4FileHandle hFile,
    uint8_t*      coverArt,
    uint32_t      size );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4GetMetadataCoverArt(
    MP4FileHandle hFile,
    uint8_t**     coverArt,
    uint32_t*     size,
    uint32_t      index DEFAULT(0) );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
uint32_t MP4GetMetadataCoverArtCount(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4DeleteMetadataCoverArt(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4SetMetadataAlbumArtist(
    MP4FileHandle hFile,
    const char*   value );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4GetMetadataAlbumArtist(
    MP4FileHandle hFile,
    char**        value );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the tags convenience API.
  */
MP4V2_DEPRECATED
MP4V2_EXPORT
bool MP4DeleteMetadataAlbumArtist(
    MP4FileHandle hFile );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the iTMF generic API.
  */
MP4V2_EXPORT
bool MP4SetMetadataFreeForm(
    MP4FileHandle  hFile,
    const char*    name,
    const uint8_t* pValue,
    uint32_t       valueSize,
    const char*    owner DEFAULT(NULL) );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the iTMF generic API.
  */
MP4V2_EXPORT
bool MP4GetMetadataFreeForm(
    MP4FileHandle hFile,
    const char*   name,
    uint8_t**     pValue,
    uint32_t*     valueSize,
    const char*   owner DEFAULT(NULL) );

/** 
  * @deprecated Deprecated, scheduled for removal. Please use the iTMF generic API.
  */
MP4V2_EXPORT
bool MP4DeleteMetadataFreeForm(
    MP4FileHandle hFile,
    const char*   name,
    const char*   owner DEFAULT(NULL) );

/** @} ***********************************************************************/

#endif /* MP4V2_META_H */
