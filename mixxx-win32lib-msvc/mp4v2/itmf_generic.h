#ifndef MP4V2_ITMF_GENERIC_H
#define MP4V2_ITMF_GENERIC_H

/**************************************************************************//**
 *
 *  @defgroup mp4_itmf_generic MP4v2 iTMF (iTunes Metadata Format) Generic
 *  @{
 *
 *****************************************************************************/

/** Basic types of value data as enumerated in spec. */
typedef enum MP4ItmfBasicType_e
{
    MP4_ITMF_BT_IMPLICIT  = 0,   /**< for use with tags for which no type needs to be indicated */
    MP4_ITMF_BT_UTF8      = 1,   /**< without any count or null terminator */
    MP4_ITMF_BT_UTF16     = 2,   /**< also known as UTF-16BE */
    MP4_ITMF_BT_SJIS      = 3,   /**< deprecated unless it is needed for special Japanese characters */
    MP4_ITMF_BT_HTML      = 6,   /**< the HTML file header specifies which HTML version */
    MP4_ITMF_BT_XML       = 7,   /**< the XML header must identify the DTD or schemas */
    MP4_ITMF_BT_UUID      = 8,   /**< also known as GUID; stored as 16 bytes in binary (valid as an ID) */
    MP4_ITMF_BT_ISRC      = 9,   /**< stored as UTF-8 text (valid as an ID) */
    MP4_ITMF_BT_MI3P      = 10,  /**< stored as UTF-8 text (valid as an ID) */
    MP4_ITMF_BT_GIF       = 12,  /**< (deprecated) a GIF image */
    MP4_ITMF_BT_JPEG      = 13,  /**< a JPEG image */
    MP4_ITMF_BT_PNG       = 14,  /**< a PNG image */
    MP4_ITMF_BT_URL       = 15,  /**< absolute, in UTF-8 characters */
    MP4_ITMF_BT_DURATION  = 16,  /**< in milliseconds, 32-bit integer */
    MP4_ITMF_BT_DATETIME  = 17,  /**< in UTC, counting seconds since midnight, January 1, 1904; 32 or 64-bits */
    MP4_ITMF_BT_GENRES    = 18,  /**< a list of enumerated values */
    MP4_ITMF_BT_INTEGER   = 21,  /**< a signed big-endian integer with length one of { 1,2,3,4,8 } bytes */
    MP4_ITMF_BT_RIAA_PA   = 24,  /**< RIAA parental advisory; { -1=no, 1=yes, 0=unspecified }, 8-bit ingteger */
    MP4_ITMF_BT_UPC       = 25,  /**< Universal Product Code, in text UTF-8 format (valid as an ID) */
    MP4_ITMF_BT_BMP       = 27,  /**< Windows bitmap image */

    MP4_ITMF_BT_UNDEFINED = 255  /**< undefined */
} MP4ItmfBasicType;

/** Data structure.
 *  Models an iTMF data atom contained in an iTMF metadata item atom.
 */
typedef struct MP4ItmfData_s
{
    uint8_t          typeSetIdentifier; /**< always zero. */
    MP4ItmfBasicType typeCode;          /**< iTMF basic type. */
    uint32_t         locale;            /**< always zero. */
    uint8_t*         value;             /**< may be NULL. */
    uint32_t         valueSize;         /**< value size in bytes. */
} MP4ItmfData;

/** List of data. */
typedef struct MP4ItmfDataList_s
{
    MP4ItmfData* elements; /**< flat array. NULL when size is zero. */
    uint32_t     size;     /**< number of elements. */
} MP4ItmfDataList;

/** Item structure.
 *  Models an iTMF metadata item atom contained in an ilst atom.
 */
typedef struct MP4ItmfItem_s
{
    int32_t         index;    /**< 0-based index of item in ilst container. -1 if undefined. */
    char*           code;     /**< four-char code identifing atom type. NULL-terminated. */
    char*           mean;     /**< may be NULL. UTF-8 meaning. NULL-terminated. */
    char*           name;     /**< may be NULL. UTF-8 name. NULL-terminated. */
    MP4ItmfDataList dataList; /**< list of data. size is always >= 1. */
} MP4ItmfItem;

/** List of items. */
typedef struct MP4ItmfItemList_s
{
    MP4ItmfItem* elements; /**< flat array. NULL when size is zero. */
    uint32_t     size;     /**< number of elements. */
} MP4ItmfItemList;

/** Allocate an item on the heap.
 *  @param code four-char code identifying atom type. NULL-terminated.
 *  @param numData number of data elements to allocate. Must be >= 1.
 *  @return newly allocated item.
 */
MP4V2_EXPORT MP4ItmfItem*
MP4ItmfItemAlloc( const char* code, uint32_t numData );

/** Free an item (deep free).
 *  @param item to be free'd.
 */
MP4V2_EXPORT void
MP4ItmfItemFree( MP4ItmfItem* item );

/** Free an item list (deep free).
 *  @param itemList to be free'd.
 */
MP4V2_EXPORT void
MP4ItmfItemListFree( MP4ItmfItemList* itemList );

/** Get list of all items from file.
 *  @param hFile handle of file to operate on.
 *  @return On succes, list of items, which must be free'd. On failure, NULL.
 */
MP4V2_EXPORT MP4ItmfItemList*
MP4ItmfGetItems( MP4FileHandle hFile );

/** Get list of items by code from file.
 *  @param hFile handle of file to operate on.
 *  @param code four-char code identifying atom type. NULL-terminated.
 *  @return On succes, list of items, which must be free'd. On failure, NULL.
 */
MP4V2_EXPORT MP4ItmfItemList*
MP4ItmfGetItemsByCode( MP4FileHandle hFile, const char* code );

/** Get list of items by meaning from file.
 *  Implicitly only returns atoms of code @b{----}.
 *  @param hFile handle of file to operate on.
 *  @param meaning UTF-8 meaning. NULL-terminated.
 *  @param name may be NULL. UTF-8 name. NULL-terminated.
 *  @return On succes, list of items, which must be free'd. On failure, NULL.
 */
MP4V2_EXPORT MP4ItmfItemList*
MP4ItmfGetItemsByMeaning( MP4FileHandle hFile, const char* meaning, const char* name );

/** Add an item to file.
 *  @param hFile handle of file to operate on.
 *  @param item object to add.
 *  @return <b>true</b> on success, <b>false</b> on failure.
 */
MP4V2_EXPORT bool
MP4ItmfAddItem( MP4FileHandle hFile, const MP4ItmfItem* item );

/** Overwrite an existing item in file.
 *  @param hFile handle of file to operate on.
 *  @param item object to overwrite. Must have a valid index obtained from prior get.
 *  @return <b>true</b> on success, <b>false</b> on failure.
 */
MP4V2_EXPORT bool
MP4ItmfSetItem( MP4FileHandle hFile, const MP4ItmfItem* item );

/** Remove an existing item from file.
 *  @param hFile handle of file to operate on.
 *  @param item object to remove. Must have a valid index obtained from prior get.
 *  @return <b>true</b> on success, <b>false</b> on failure.
 */
MP4V2_EXPORT bool
MP4ItmfRemoveItem( MP4FileHandle hFile, const MP4ItmfItem* item );

/** @} ***********************************************************************/

#endif /* MP4V2_ITMF_GENERIC_H */
