#include "library/dao/trackschema.h"

//static
QStringList TrackSchema::GetColumnNames() {
    QStringList columns;

    // This is the canonical ordering of columns. Changing this order can break things, so always
    // add new columns to the bottom of the list.
    columns << LIBRARYTABLE_ID
            << LIBRARYTABLE_PLAYED
            << LIBRARYTABLE_TIMESPLAYED
            //has to be up here otherwise Played and TimesPlayed are not shown
            << LIBRARYTABLE_ALBUMARTIST
            << LIBRARYTABLE_ALBUM
            << LIBRARYTABLE_ARTIST
            << LIBRARYTABLE_TITLE
            << LIBRARYTABLE_YEAR
            << LIBRARYTABLE_RATING
            << LIBRARYTABLE_GENRE
            << LIBRARYTABLE_COMPOSER
            << LIBRARYTABLE_GROUPING
            << LIBRARYTABLE_TRACKNUMBER
            << LIBRARYTABLE_KEY
            << LIBRARYTABLE_KEY_ID
            << LIBRARYTABLE_BPM
            << LIBRARYTABLE_BPM_LOCK
            << LIBRARYTABLE_DURATION
            << LIBRARYTABLE_BITRATE
            << LIBRARYTABLE_REPLAYGAIN
            << LIBRARYTABLE_FILETYPE
            << LIBRARYTABLE_DATETIMEADDED
            << TRACKLOCATIONSTABLE_LOCATION
            << TRACKLOCATIONSTABLE_FSDELETED
            << LIBRARYTABLE_COMMENT
            << LIBRARYTABLE_MIXXXDELETED
            << LIBRARYTABLE_COLOR
            << LIBRARYTABLE_COVERART_SOURCE
            << LIBRARYTABLE_COVERART_TYPE
            << LIBRARYTABLE_COVERART_LOCATION
            << LIBRARYTABLE_COVERART_HASH;
    return columns;
}

// static
QString TrackSchema::TableForColumn(const QString& columnName){
    if (columnName == "location" || columnName == "fs_deleted") {
        return "track_locations";
    }
    // This doesn't detect unknown columns, but that's not really important here.
    return "library";
}