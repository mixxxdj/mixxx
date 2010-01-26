#ifndef MP4V2_ITMF_TAGS_H
#define MP4V2_ITMF_TAGS_H

/**************************************************************************//**
 *
 *  @defgroup mp4_itmf_tags MP4v2 iTMF (iTunes Metadata Format) Tags
 *  @{
 *
 *****************************************************************************/

/** Enumeration of possible MP4TagArtwork::type values. */
typedef enum MP4TagArtworkType_e
{
    MP4_ART_UNDEFINED = 0,
    MP4_ART_BMP       = 1,
    MP4_ART_GIF       = 2,
    MP4_ART_JPEG      = 3,
    MP4_ART_PNG       = 4,
} MP4TagArtworkType;

/** Data object representing a single piece of artwork. */
typedef struct MP4TagArtwork_s
{
    const void*       data; /**< raw picture data */
    uint32_t          size; /**< data size in bytes */
    MP4TagArtworkType type; /**< data type */
} MP4TagArtwork;

typedef struct MP4TagTrack_s
{
    uint16_t index;
    uint16_t total;
} MP4TagTrack;

typedef struct MP4TagDisk_s
{
    uint16_t index;
    uint16_t total;
} MP4TagDisk;

/** Tags <b>convenience</b> structure.
 *
 *  This structure is used in the tags convenience API which allows for
 *  simplified retrieval and modification of the majority of known tags.
 *
 *  This is a read-only structure and each tag is present if and only if the
 *  pointer is a <b>non-NULL</b> value. The actual data is backed by a hidden
 *  data cache which is only updated when the appropriate metadata <b>set</b>
 *  function is used, or if MP4TagsFetch() is invoked. Thus, if other API
 *  is used to manipulate relevent atom structure of the MP4 file, the user
 *  is responsible for re-fetching the data in this structure.
 */
typedef struct MP4Tags_s
{
    void* __handle; /* internal use only */

    const char*        name;
    const char*        artist;
    const char*        albumArtist; 
    const char*        album;
    const char*        grouping;
    const char*        composer;
    const char*        comments;
    const char*        genre;
    const uint16_t*    genreType;
    const char*        releaseDate;
    const MP4TagTrack* track;
    const MP4TagDisk*  disk;
    const uint16_t*    tempo;
    const uint8_t*     compilation;

    const char*     tvShow;
    const char*     tvNetwork;
    const char*     tvEpisodeID;
    const uint32_t* tvSeason;
    const uint32_t* tvEpisode;

    const char* description;
    const char* longDescription;
    const char* lyrics;

    const char* sortName;
    const char* sortArtist;
    const char* sortAlbumArtist;
    const char* sortAlbum;
    const char* sortComposer;
    const char* sortTVShow;

    const MP4TagArtwork* artwork;
    uint32_t             artworkCount;

    const char* copyright;
    const char* encodingTool;
    const char* encodedBy;
    const char* purchaseDate;

    const uint8_t* podcast;
    const char*    keywords;  /* TODO: Needs testing */
    const char*    category;    

    const uint8_t* hdVideo;
    const uint8_t* mediaType;
    const uint8_t* contentRating;
    const uint8_t* gapless;

    const char*     iTunesAccount;
    const uint8_t*  iTunesAccountType;
    const uint32_t* iTunesCountry;
    const uint32_t* cnID;
    const uint32_t* atID;
    const uint64_t* plID;
    const uint32_t* geID;
} MP4Tags;

/** Allocate tags convenience structure for reading and settings tags.
 *
 *  This function allocates a new structure which represents a snapshot
 *  of all the tags therein, tracking if the tag is missing,
 *  or present and with value. It is the caller's responsibility to free
 *  the structure with MP4TagsFree().
 *
 *  @return structure with all tags missing.
 */
MP4V2_EXPORT
const MP4Tags* MP4TagsAlloc();

/** Fetch data from mp4 file and populate structure.
 *
 *  The tags structure and its hidden data-cache is updated to
 *  reflect the actual tags values found in the <b>hFile</b>.
 *
 *  @param tags structure to fetch (write) into.
 *  @param hFile handle of file to fetch data from.
 */
MP4V2_EXPORT
void MP4TagsFetch( const MP4Tags* tags, MP4FileHandle hFile );

/** Store data to mp4 file from structure.
 *
 *  The tags structure is pushed out to the mp4 file,
 *  adding tags if needed, removing tags if needed, and updating
 *  the values to modified tags.
 *
 *  @param tags structure to store (read) from.
 *  @param hFile handle of file to store data to.
 */
MP4V2_EXPORT
void MP4TagsStore( const MP4Tags* tags, MP4FileHandle hFile );

/** Free tags convenience structure.
 *
 *  This function frees memory associated with the structure.
 *
 *  @param tags structure to destroy.
 */
MP4V2_EXPORT
void MP4TagsFree( const MP4Tags* tags );

MP4V2_EXPORT void MP4TagsSetName            ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetArtist          ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetAlbumArtist     ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetAlbum           ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetGrouping        ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetComposer        ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetComments        ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetGenre           ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetGenreType       ( const MP4Tags*, const uint16_t* );
MP4V2_EXPORT void MP4TagsSetReleaseDate     ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetTrack           ( const MP4Tags*, const MP4TagTrack* );
MP4V2_EXPORT void MP4TagsSetDisk            ( const MP4Tags*, const MP4TagDisk* );
MP4V2_EXPORT void MP4TagsSetTempo           ( const MP4Tags*, const uint16_t* );
MP4V2_EXPORT void MP4TagsSetCompilation     ( const MP4Tags*, const uint8_t* );

MP4V2_EXPORT void MP4TagsSetTVShow          ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetTVNetwork       ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetTVEpisodeID     ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetTVSeason        ( const MP4Tags*, const uint32_t* );
MP4V2_EXPORT void MP4TagsSetTVEpisode       ( const MP4Tags*, const uint32_t* );

MP4V2_EXPORT void MP4TagsSetDescription     ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetLongDescription ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetLyrics          ( const MP4Tags*, const char* );

MP4V2_EXPORT void MP4TagsSetSortName        ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetSortArtist      ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetSortAlbumArtist ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetSortAlbum       ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetSortComposer    ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetSortTVShow      ( const MP4Tags*, const char* );

MP4V2_EXPORT void MP4TagsAddArtwork         ( const MP4Tags*, MP4TagArtwork* );
MP4V2_EXPORT void MP4TagsSetArtwork         ( const MP4Tags*, uint32_t, MP4TagArtwork* );
MP4V2_EXPORT void MP4TagsRemoveArtwork      ( const MP4Tags*, uint32_t );

MP4V2_EXPORT void MP4TagsSetCopyright       ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetEncodingTool    ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetEncodedBy       ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetPurchaseDate    ( const MP4Tags*, const char* );

MP4V2_EXPORT void MP4TagsSetPodcast         ( const MP4Tags*, const uint8_t* );
MP4V2_EXPORT void MP4TagsSetKeywords        ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetCategory        ( const MP4Tags*, const char* );

MP4V2_EXPORT void MP4TagsSetHDVideo         ( const MP4Tags*, const uint8_t* );
MP4V2_EXPORT void MP4TagsSetMediaType       ( const MP4Tags*, const uint8_t* );
MP4V2_EXPORT void MP4TagsSetContentRating   ( const MP4Tags*, const uint8_t* );
MP4V2_EXPORT void MP4TagsSetGapless         ( const MP4Tags*, const uint8_t* );

MP4V2_EXPORT void MP4TagsSetITunesAccount     ( const MP4Tags*, const char* );
MP4V2_EXPORT void MP4TagsSetITunesAccountType ( const MP4Tags*, const uint8_t* );
MP4V2_EXPORT void MP4TagsSetITunesCountry     ( const MP4Tags*, const uint32_t* );
MP4V2_EXPORT void MP4TagsSetCNID              ( const MP4Tags*, const uint32_t* );
MP4V2_EXPORT void MP4TagsSetATID              ( const MP4Tags*, const uint32_t* );
MP4V2_EXPORT void MP4TagsSetPLID              ( const MP4Tags*, const uint64_t* );
MP4V2_EXPORT void MP4TagsSetGEID              ( const MP4Tags*, const uint32_t* );

/** @} ***********************************************************************/

#endif /* MP4V2_ITMF_TAGS_H */
